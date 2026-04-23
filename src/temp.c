#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "variables.h"

#define DS18B20_PIN 32
#define TEMP_CONTROL_PIN 30

float temp_c = 0.0f;

static volatile bool sample_requested = false;
static struct repeating_timer temp_timer;

static inline void ow_drive_low(void) {
    gpio_set_dir(DS18B20_PIN, GPIO_OUT);
    gpio_put(DS18B20_PIN, 0);
}

static inline void ow_release_line(void) {
    gpio_set_dir(DS18B20_PIN, GPIO_IN);
}

static inline bool ow_read_line(void) {
    return gpio_get(DS18B20_PIN);
}

static bool ow_reset_pulse(void) {
    bool presence;

    ow_drive_low();
    sleep_us(480);

    ow_release_line();
    sleep_us(70);

    presence = !ow_read_line();

    sleep_us(410);

    return presence;
}

static void ow_write_bit(int bit) {
    ow_drive_low();

    if (bit) {
        sleep_us(6);
        ow_release_line();
        sleep_us(64);
    } else {
        sleep_us(60);
        ow_release_line();
        sleep_us(10);
    }
}

static int ow_read_bit(void) {
    int bit;

    ow_drive_low();
    sleep_us(6);

    ow_release_line();
    sleep_us(9);

    bit = ow_read_line();

    sleep_us(55);

    return bit;
}

static void ow_write_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        ow_write_bit(byte & 1);
        byte >>= 1;
    }
}

static uint8_t ow_read_byte(void) {
    uint8_t byte = 0;

    for (int i = 0; i < 8; i++) {
        if (ow_read_bit()) {
            byte |= (1 << i);
        }
    }

    return byte;
}

static float ds18b20_read_temp_c(void) {
    uint8_t temp_lsb;
    uint8_t temp_msb;
    int16_t raw_temp;

    if (!ow_reset_pulse()) {
        return temp_c;
    }

    ow_write_byte(0xCC);
    ow_write_byte(0x44);

    sleep_ms(750);

    if (!ow_reset_pulse()) {
        return temp_c;
    }

    ow_write_byte(0xCC);
    ow_write_byte(0xBE);

    temp_lsb = ow_read_byte();
    temp_msb = ow_read_byte();

    raw_temp = (int16_t)((temp_msb << 8) | temp_lsb);

    return raw_temp / 16.0f;
}

static bool temp_timer_callback(struct repeating_timer *t) {
    sample_requested = true;
    return true;
}

static void update_temp_control_pin(void) {
    float low_limit = (float)temp_setting - 2.0f;

    if (temp_setting <= 0) {
        gpio_put(TEMP_CONTROL_PIN, 0);
        return;
    }

    if (temp_c < low_limit) {
        gpio_put(TEMP_CONTROL_PIN, 1);
    } else {
        gpio_put(TEMP_CONTROL_PIN, 0);
    }
}

void temp_init(void) {
    gpio_init(DS18B20_PIN);
    gpio_pull_up(DS18B20_PIN);
    ow_release_line();

    gpio_init(TEMP_CONTROL_PIN);
    gpio_set_dir(TEMP_CONTROL_PIN, GPIO_OUT);
    gpio_put(TEMP_CONTROL_PIN, 0);

    add_repeating_timer_ms(5000, temp_timer_callback, NULL, &temp_timer);
}

bool temp_task(void) {
    if (!sample_requested) {
        return false;
    }

    sample_requested = false;

    temp_c = ds18b20_read_temp_c();

    update_temp_control_pin();

    printf("Temperature: %.2f C\n", temp_c);
    printf("Set Temp: %d C\n", temp_setting);
    fflush(stdout);

    return true;
}