#ifndef DELAYMS_
#define DELAYMS_

typedef unsigned int size_t;

void DelayMS(size_t ms) {
    size_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
};

#endif