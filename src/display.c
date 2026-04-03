#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/structs/sio.h"
#include "hardware/structs/io_bank0.h"
#include "hardware/structs/pads_bank0.h"
#include "hardware/structs/timer.h"


// 7-segment display message buffer
// Declared as static to limit scope to this file only.
static char msg[8] = {
    0x3F, // seven-segment value of 0
    0x06, // seven-segment value of 1
    0x5B, // seven-segment value of 2
    0x4F, // seven-segment value of 3
    0x66, // seven-segment value of 4
    0x6D, // seven-segment value of 5
    0x7D, // seven-segment value of 6
    0x07, // seven-segment value of 7
};

extern char font[]; // Font mapping for 7-segment display
static int index = 0; // Current index in the message buffer

// We provide you with this function for directly displaying characters.
// However, it can't use the decimal point, which display_print does.
void display_char_print(const char message[]) {
    for (int i = 0; i < 8; i++) {
        msg[i] = font[message[i] & 0xFF];
    }
}

/********************************************************* */
// Implement the functions below.


void display_init_pins() {
    //enable output on gp10-20
    sio_hw->gpio_oe_set = (0x7FF << 10);

    for (int i = 10; i < 21; i++){
        //set as sio
        io_bank0_hw->io[i].ctrl = 5;
        pads_bank0_hw->io[i] &= ~PADS_BANK0_GPIO0_OD_BITS;
        pads_bank0_hw->io[i] &= ~PADS_BANK0_GPIO0_ISO_BITS;
    }

}

void display_isr() {

    //acknowledge timer 1 alarm 0 by clearing the intr register
    timer1_hw->intr = 1u;

    //set a new variable to be the full mask for the output of gp10 - gp 20
    //need the ascii value from msg
    //also need the sel from the current index (since we are "looping" through the displays)
    uint32_t output = ((uint32_t)msg[index] << 10) | ((uint32_t)(index & 0x7) << 18);

    //mask used to clear gp10 - gp20
    uint32_t mask = (0xFFu << 10) | (0x7u << 18);

    //clear the pins from last loop
    sio_hw->gpio_clr = mask;

    //set the pins to be the current output
    sio_hw->gpio_set = output;

    //increment index so that it goes 0 to 7 then resets
    index = (index + 1) & 7;

    //have alarm 0 go off every 3ms
    timer1_hw->alarm[0] = timer1_hw->timelr + 3000u;
}

void display_init_timer() {
    timer1_hw->intr = 1u;
    //have to or so it doesnt rewrite the other alarms
    timer1_hw->inte |= 1u;

    irq_set_exclusive_handler(TIMER1_IRQ_0, display_isr);
    irq_set_enabled(TIMER1_IRQ_0, true);

    timer1_hw->alarm[0] = timer1_hw->timelr + 3000u;
}


void display_print(const uint16_t message[]) {
    //loop through the 8 lsb of message that represent the ascii value of the button pressed
    for (int i = 0; i < 8; i++) {

        //message is the mask of key events
        //the 9 bit thing that tells us the ascii and if it is pressed or released

        //mask that represents the current ascii value of message
        uint8_t ascii   = message[i] & 0xFF;

        //mask that represents if the button is pressed or not
        uint8_t pressed = (message[i] >> 8) & 1u;

        //set msg to the binary for the 7 seg display by converting the ascii value with font
        msg[i] = font[ascii];

        //if pressed then have the decimal point on
        //else have the decimal point off
        if (pressed)
            msg[i] |= 0x80;
        else
            msg[i] &= 0x7F;
    }
}