/*
 * my_graphics.h
 *
 *  Created on: May 29, 2022
 *      Author: Sam
 */

#ifndef SRC_MYUTILS_MY_GRAPHICS_H_
#define SRC_MYUTILS_MY_GRAPHICS_H_

#include "stdio.h"
#include <stdbool.h>

#define LCD_FRAME_BUFFER_LAYER0 (LCD_FRAME_BUFFER + 0x130000)
#define LCD_FRAME_BUFFER_LAYER1 LCD_FRAME_BUFFER

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define SCREEN_HALFWIDTH (SCREEN_WIDTH >> 1)
#define SCREEN_HALFHEIGHT (SCREEN_HEIGHT >> 1)

void prepareDisplay(void);
void clearScreen(void);

#endif /* SRC_MYUTILS_MY_GRAPHICS_H_ */
