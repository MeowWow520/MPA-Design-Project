//
// Created by MeowWow520 on 2026/6/22.
//

#ifndef MPA_DESIGN_PROJECT_DELAY_H
#define MPA_DESIGN_PROJECT_DELAY_H
typedef unsigned int size_t;

void DelayMS(size_t ms) {
    size_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
};


#endif //MPA_DESIGN_PROJECT_DELAY_H
