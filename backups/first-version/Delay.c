//
// Created by MeowWow520 on 2026/6/22.
//
#include "Delay.h"

void DelayMS(size_t ms) {
    size_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}