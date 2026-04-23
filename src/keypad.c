#include "hardware/irq.h"
#include "hardware/structs/timer.h"
#include "hardware/structs/sio.h"
#include "hardware/gpio.h"
#include "hardware/regs/intctrl.h"
#include "queue.h"
#include "pico/stdlib.h"



// Global column variable
int col = -1;

// Global key state
static bool state[16]; // Are keys pressed/released

// Keymap for the keypad
const char keymap[17] = "DCBA#9630852*741";

uint32_t previously_high = 0;
uint8_t rows;

// Defined here to avoid circular dependency issues with autotest
// You can see the struct definition in queue.h
KeyEvents kev = { 
    .head = 0,
    .tail = 0 
};


bool key_available(void) {
    return kev.head != kev.tail;
}


uint8_t keypad_read_rows() {
    //return:
    //the mask of gpio pins shifted to the right by two bits
    //AND with 1111
    //so will be a 32 bit mask w/ lsb being gp2 - gp5 showing if they are high or low
    rows = (sio_hw->gpio_in >> 2) & 0x0Fu;
}

void keypad_drive_column() {

    //set 1 to the intr register to clear alarm0 interrupt
    timer_hw->intr = 1u << 0;

    //increment through the columns
    col = (col + 1) & 3;

    //create a mask that will be shifted to the right by 6 plus col
    uint32_t currently_pressed = 1u << (6 + col);

    //if one of the currently high or one of them just become high then toggle that gpio pin
    sio_hw->gpio_togl = previously_high ^ currently_pressed;
    previously_high = currently_pressed;

    //in alarm0 set the amount of time for the timer to go off to 2500 ms
    timer_hw->alarm[0] = timer_hw->timelr + 25000u;
}

void keypad_isr() {
    
    //set intr register to be 10 to clear alarm1 interupt
    timer_hw->intr = 1u << 1;

    //read which of the row gpio pins are high
    
    keypad_read_rows();

    //loop through the 4 rows
    for (int r = 0; r < 4; r++) {

        int idx;

        idx = col * 4 + r;
        
        //if the current gpio for the row we are looking at is true
        //set a bool value to true or false telling us if a button in that row is being pressed
        bool pressed_now = (rows >> r) & 1;

        //if this row is being pressed and we havent already pressed this row
        if (pressed_now && !state[idx]) {

            //set the appropriate index in state to be true
            state[idx] = true;

            //9 bit register with msb as the key has been pressed
            //other 8 lsb's are the ascii value of the button pressed in binary
            uint16_t event = (1u << 8) | (uint8_t)keymap[idx];
            key_push(event);
        } 
        //else if there is a button not being pressed in this row and we have read that row to have an event on it previously
        else if (!pressed_now && state[idx]) {

            //set the index of state in question to 0
            state[idx] = false;

            //make the 9th bit zero but keep the ascii value
            uint16_t event = (0u << 8) | (uint8_t)keymap[idx];
            key_push(event);
        }
    }

    //set alarm 1 to go off in 25ms
    timer_hw->alarm[1] = timer_hw->timelr + 25000u;
}

/********************************************************* */
// Implement the functions below.

void keypad_init_pins() {
    //loop through the gpio pins needed
    for (int pin = 6; pin <= 9; pin++) {
        //init the pin
        gpio_init(pin);
        //set as output
        gpio_set_dir(pin, GPIO_OUT);
        //initially set pin to zero
        gpio_put(pin, 0);
    }

    //loop through the rows for inputs
    for (int pin = 2; pin <= 5; pin++) {
        //initialize the pin
        gpio_init(pin);
        //set as input
        gpio_set_dir(pin, GPIO_IN);
    }

    // Track "no column currently high"
    previously_high = 0;
    col = -1;
}

void keypad_init_timer() {

    
    timer_hw->alarm[0] = timer_hw->timelr + 1000000u;
    timer_hw->alarm[1] = timer_hw->timelr + 1010000u;

    // Clear any pending alarm flags (WC: write 1 to clear)
    timer_hw->intr = (1u << 0) | (1u << 1);

    // Enable ALARM0 + ALARM1
    timer_hw->inte |= (1u << 0) | (1u << 1);

    // Install *exclusive* handlers (lab hard requirement)
    irq_set_exclusive_handler(TIMER0_IRQ_0, keypad_drive_column);
    irq_set_exclusive_handler(TIMER0_IRQ_1, keypad_isr);

    irq_set_enabled(TIMER0_IRQ_0, true);
    irq_set_enabled(TIMER0_IRQ_1, true);
}
