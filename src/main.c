#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "variables.h"
#include "hardware/spi.h"
#include <math.h>   
#include "lcd.h"


void keypad_init_pins(void);
void keypad_init_timer(void);
void display_init_pins(void);
void display_init_timer(void);
void draw_menu(void);
void process_key(char key);
void display_print(const uint16_t message[]);
uint32_t key_pop(void);

#define PIN_SDI    15
#define PIN_CS     13
#define PIN_SCK    14
#define PIN_DC     17
#define PIN_nRESET 16
//////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////
void init_spi_lcd() {
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_DC, GPIO_FUNC_SIO);
    gpio_set_function(PIN_nRESET, GPIO_FUNC_SIO);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_nRESET, GPIO_OUT);

    gpio_put(PIN_CS, 1); // CS high
    gpio_put(PIN_DC, 0); // DC low
    gpio_put(PIN_nRESET, 1); // nRESET high

    // initialize SPI1 with 48 MHz clock
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SDI, GPIO_FUNC_SPI);
    spi_init(spi1, 12 * 1000 * 1000);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

int main(void)
{
    stdio_init_all();

    init_spi_lcd();
    
    LCD_Setup();
    LCD_Clear(0x0000);

    keypad_init_pins();
    keypad_init_timer();

    draw_menu();

    for (;;) {
        uint32_t keyevent = key_pop();

        uint8_t pressed = (keyevent >> 8) & 1u;
        char key = keyevent & 0xFF;

        if (pressed) {
            process_key(key);
        }
    }

    return 0;
}