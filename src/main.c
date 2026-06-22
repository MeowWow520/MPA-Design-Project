//
// Created by MeowWow520 on 2026/6/22.
//

#include <stdio.h>
#include "REG52.H"
typedef unsigned char uint8;
typedef unsigned int size_t;
#define OPEN  1
#define CLOSE 0

sbit KEY_LEFT   = P2^0;
sbit KEY_RIGHT  = P2^1;
sbit KEY_START  = P2^2;
sbit KEY_ROTATE = P2^3;

size_t key_state = 4;

uint8 display_buffer[16] = {
    0x00000000, //  up  line 1
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line 8
    0x00000000, // down line 1
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line 8
};
void DelayMS(uint32 ms) {
    int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
};
void HandleEvents() {
    if(KEY_LEFT == 0) {
        DelayMS(10);
        if(KEY_LEFT == 0) {
            while(KEY_LEFT == 0);
            key_state = 0;
        }
    }
    if(KEY_RIGHT == 0) {
        DelayMS(10);
        if(KEY_RIGHT == 0) {
            while(KEY_RIGHT == 0);
            key_state = 1;
        }
    }
    if(KEY_START == 0) {
        DelayMS(10);
        if(KEY_START == 0) {
            while(KEY_START == 0);
            key_state = 2;
        }
    }
    if(KEY_ROTATE == 0) {
        DelayMS(10);
        if(KEY_ROTATE == 0) {
            while(KEY_ROTATE == 0);
            key_state = 3;
        }
    }
};

void main(void) {
    // init sth.
    while (1) {
        HandleEvents();
        if (key_state == 4)

    }
}