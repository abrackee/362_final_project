#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "variables.h"

//////////////////////////////////////////////////////////////////////////////

const char* username = "abrackee";

//////////////////////////////////////////////////////////////////////////////

void keypad_init_pins(void);
void keypad_init_timer(void);
void display_init_pins(void);
void display_init_timer(void);
void display_print(const uint16_t message[]);
uint32_t key_pop(void);

extern char font[];
time_zone = 0;
temp_setting = 0;


//////////////////////////////////////////////////////////////////////////////
// MENU STATE
//////////////////////////////////////////////////////////////////////////////

typedef enum {
    MENU_MAIN,
    MENU_TIME,
    MENU_TEMP,
    MENU_SELECT_TIME_ZONE,
    MENU_VIEW_CURRENT_TIME,
    MENU_SET_WATER_TEMP
} menu_state_t;

static menu_state_t current_menu = MENU_MAIN;

//////////////////////////////////////////////////////////////////////////////
// FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

void display_menu_main(void);
void display_menu_time(void);
void display_menu_temp(void);
void display_menu_select_time_zone(void);
void draw_menu(void);
void process_key(char key);

// actions
void set_time_zone(char key);
void view_current_time(void);
void set_water_temperature(void);

//////////////////////////////////////////////////////////////////////////////
// MENU DISPLAY FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

void display_menu_main(void) {
    printf("\nMENU:\n");
    printf("A. Time\n");
    printf("B. Temperature\n");
}

void display_menu_time(void) {
    printf("\nTime Menu:\n");
    printf("A. Set Time Zone\n");
    printf("B. View Current Time\n");
    printf("C. Exit\n");
}

void display_menu_temp(void) {
    printf("\nTemperature Menu:\n");
    printf("A. Set Water Temperature\n");
    printf("B. Exit\n");
}

void display_menu_view_current_time(void) {
    printf("\nView Current Time:\n");
    printf("A. Enter\n");
    printf("B. Exit\n");
}

void display_menu_set_water_temp(void) {
    printf("\nSet Water Temperature:\n");
}

void display_menu_select_time_zone(void) {
    printf("\nSelect Time Zone:\n");
    printf("A. Eastern\n");
    printf("B. Central\n");
    printf("C. Mountain\n");
}

///////////////////////////////////////////

void draw_menu(void) {
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

        case MENU_VIEW_CURRENT_TIME:
            display_menu_view_current_time();
            break;

        case MENU_SET_WATER_TEMP:
            display_menu_set_water_temp();
            break;
        case MENU_SELECT_TIME_ZONE:
            display_menu_select_time_zone();
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

void set_time_zone(char key) {
    if (key == 'A') {
        time_zone = 0;
        printf("\nEastern selected\n");
    }
    else if (key == 'B') {
        time_zone = 1;
        printf("\nCentral selected\n");
    }
    else if (key == 'C') {
        time_zone = 2;
        printf("\nMountain selected\n");
    }
}


void view_current_time(void) {

    printf("CURRENT TIME");
}

void set_water_temperature(void) {
    char digits[3];
    int count = 0;

    printf("\nEnter 2 digits:\n");

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

    printf("\n%d\n", temp_setting);
}

//////////////////////////////////////////////////////////////////////////////
// MENU LOGIC
//////////////////////////////////////////////////////////////////////////////

void process_key(char key) {
    switch (current_menu) {

        case MENU_MAIN:
            if (key == 'A') {
                current_menu = MENU_TIME;
                draw_menu();
            }
            else if (key == 'B') {
                current_menu = MENU_TEMP;
                draw_menu();
            }
            break;

        case MENU_TIME:
            if (key == 'A') {
                current_menu = MENU_SELECT_TIME_ZONE;
                draw_menu();
            }
            else if (key == 'B') {
                current_menu = MENU_VIEW_CURRENT_TIME;
                draw_menu();            
            }
            else if (key == 'C') {
                current_menu = MENU_MAIN;
                draw_menu();
            }
            break;

        case MENU_TEMP:
            if (key == 'A') {
                current_menu = MENU_SET_WATER_TEMP;
                draw_menu();
                set_water_temperature();
                current_menu = MENU_MAIN;
                draw_menu();
            }
            else if (key == 'B') {
                current_menu = MENU_MAIN;
                draw_menu();
            }
            break;

        case MENU_VIEW_CURRENT_TIME:
            if (key == 'A') {
                printf("\nCurrent Time\n");
                current_menu = MENU_MAIN;
                draw_menu();
            }
            else if (key == 'B') {
                current_menu = MENU_MAIN;
                draw_menu();
            }
            break;

        default:
            current_menu = MENU_MAIN;
            draw_menu();
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////

int main(void)
{
    stdio_init_all();

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