#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "variables.h"
#include "temp.h"

extern char font[];


//////////////////////////////////////////////////////////////////////////////
// MENU STATE
//////////////////////////////////////////////////////////////////////////////

typedef enum {
    MENU_MAIN,
    MENU_TIME,
    MENU_TEMP,
    MENU_PUMP,
    MENU_LIGHT,
    MENU_FEEDING,
    MENU_LIGHT_ON,
    MENU_LIGHT_OFF
} menu_state_t;

static menu_state_t current_menu = MENU_MAIN;

uint16_t fc   = 0xFFFF;   // white text
uint16_t bg   = 0x0000;   // black background
uint8_t size  = 16;       // font size
uint8_t mode  = 0;        // usually normal draw mode

int current_time;
int temp_setting;
//////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

void display_menu_main(void);
void display_menu_time(void);
void display_menu_temp(void);
void display_menu_pump(void);
void draw_menu(void);
int LCD_Clear(int);
void process_key(char key);
void LCD_DrawString(uint16_t x, uint16_t y,
                    uint16_t fc, uint16_t bg,
                    const char *p,
                    uint8_t size, uint8_t mode);
// actions
void set_time_zone(char key);
void view_current_time(void);
void set_water_temperature(void);
void update_temperature_display(void);


//////////////////////////////////////////////////////////////////////////////
// MENU DISPLAY FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

void display_menu_main(void) {
    
    LCD_DrawString(10, 10, fc, bg, "MENU:",          size, mode);
    LCD_DrawString(10, 30, fc, bg, "1. Set Time",        size, mode);
    LCD_DrawString(10, 50, fc, bg, "2. Set Temperature", size, mode);
    LCD_DrawString(10, 70, fc, bg, "3. Set Pump Interval", size, mode);
    LCD_DrawString(10, 90, fc, bg, "4. Light Menu", size, mode);
    LCD_DrawString(10, 110, fc, bg, "5. Feeding Frequency", size, mode);
    
    char time_str[32];
    sprintf(time_str, "Current Time:  %04d", current_time);
    LCD_DrawString(10, 130, fc, bg, time_str, size, mode);

    char temp_str[32];
    sprintf(temp_str, "Temp: %.2f C", temp_c);
    LCD_DrawString(10, 150, fc, bg, temp_str, size, mode);
}

void display_menu_time(void) {
    LCD_DrawString(10, 10, fc, bg, "Enter Time In HHMM:", size, mode);
}

void display_menu_temp(void) {
    LCD_DrawString(10, 10, fc, bg, "Enter Temperature:", size, mode);
}

void display_menu_pump(void){
    LCD_DrawString(10, 10, fc, bg, "Enter Pump Interval", size, mode);
}

void display_menu_light(void){
    LCD_DrawString(10, 10, fc, bg, "1. Set Time On", size, mode);
    LCD_DrawString(10, 30, fc, bg, "2. Set Time Off", size, mode);
}

void display_menu_light_on(void){
    LCD_DrawString(10, 10, fc, bg, "Enter Time On: ", size, mode);
}

void display_menu_light_off(void){
    LCD_DrawString(10, 10, fc, bg, "Enter Time Off: ", size, mode);
}

void display_menu_feeding(void){
    LCD_DrawString(10, 10, fc, bg, "Enter Feeding Frequency:", size, mode);
}

///////////////////////////////////////////

void draw_menu(void) {
    LCD_Clear(bg);
    
    switch (current_menu) {
        case MENU_MAIN:
            display_menu_main();
            break;

        case MENU_TIME:
            display_menu_time();
            break;

        case MENU_TEMP:
            display_menu_temp();
            break;

        case MENU_PUMP:
            display_menu_pump();
            break;

        case MENU_LIGHT:
            display_menu_light();
            break;
        case MENU_LIGHT_ON:
            display_menu_light_on();
            break;
        case MENU_LIGHT_OFF:
            display_menu_light_off();
            break;
        case MENU_FEEDING:
            display_menu_feeding();
            break;
        default:
            current_menu = MENU_MAIN;
            display_menu_main();
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
// ACTION FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
void update_temperature_display(void) {
    if (current_menu != MENU_MAIN) {
        return;
    }

    char temp_str[32];

    LCD_DrawString(10, 150, fc, bg, "                    ", size, mode);

    sprintf(temp_str, "Temp: %.2f C", temp_c);
    LCD_DrawString(10, 150, fc, bg, temp_str, size, mode);
}

void set_time(void) {
    char digits[5];
    int count = 0;

    while (count < 4) {
        uint32_t keyevent = key_pop();

        uint8_t pressed = (keyevent >> 8) & 1u;
        char key = (char)(keyevent & 0xFF);

        if (!pressed) {
            continue;
        }

        if (key >= '0' && key <= '9') {
            digits[count] = key;
            count++;
        }
    }

    digits[4] = '\0';

    current_time = (digits[0] - '0') * 1000 +
                   (digits[1] - '0') * 100  +
                   (digits[2] - '0') * 10   +
                   (digits[3] - '0');
}


void set_water_temperature(void) {
    char digits[3];
    int count = 0;

    while (count < 2) {
        uint32_t keyevent = key_pop();

        uint8_t pressed = (keyevent >> 8) & 1u;
        char key = keyevent & 0xFF;

        if (!pressed) {
            continue;
        }

        if (key >= '0' && key <= '9') {
            digits[count] = key;
            count++;
            printf("%c", key);
        }
    }

    digits[2] = '\0';
    temp_setting = (digits[0] - '0') * 10 + (digits[1] - '0');
}

//////////////////////////////////////////////////////////////////////////////
// MENU LOGIC
//////////////////////////////////////////////////////////////////////////////

void process_key(char key) {
    switch (current_menu) {

        case MENU_MAIN:
            if (key == '1') {
                current_menu = MENU_TIME;
                draw_menu();
                set_time();
                current_menu = MENU_MAIN;
                draw_menu();

            }
            else if (key == '2') {
                current_menu = MENU_TEMP;
                draw_menu();
                set_water_temperature();
                current_menu = MENU_MAIN;
                draw_menu();
            }
            else if (key == '3') {
                current_menu = MENU_PUMP;
                draw_menu();
            }
            else if (key == '4') {
                current_menu = MENU_LIGHT;
                draw_menu();
            }
            else if (key == '5') {
                current_menu = MENU_FEEDING;
                draw_menu();
            }
            break;

        case MENU_LIGHT:
            if (key == '1') {
                current_menu = MENU_LIGHT_ON;
                draw_menu();
            }
            else if (key == '2') {
                current_menu = MENU_LIGHT_OFF;
                draw_menu();
            }
            break;

        default:
            current_menu = MENU_MAIN;
            draw_menu();
            break;
    }
}
