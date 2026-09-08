#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void Display_Init(void);
void Display_SetNumber(uint16_t number);
void Display_Refresh(void);

#endif