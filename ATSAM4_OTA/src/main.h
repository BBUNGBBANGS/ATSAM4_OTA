#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "asf.h"

typedef struct 
{
    uint8_t Reprogram_Flag;
    uint8_t dummy1;
    uint8_t dummy2;
    uint8_t dummy3;
}Reprogram_Info_t;

#endif
