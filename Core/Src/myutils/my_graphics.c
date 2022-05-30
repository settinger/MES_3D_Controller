/*
 * my_graphics.c
 *
 *  Created on: May 29, 2022
 *      Author: Sam
 */

#include "my_graphics.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "stdio.h"

void clearScreen() {
  BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
  BSP_LCD_FillRect(0, 0, 240, 300);
}
void prepareDisplay(void) {
  BSP_LCD_DisplayOn();
  BSP_LCD_LayerDefaultInit(0, LCD_FRAME_BUFFER_LAYER0);
  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(LCD_COLOR_LIGHTGRAY);
  BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
  BSP_LCD_SetFont(&Font16);

  // TODO: take these out
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_FillCircle(100, 200, 10);
  BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
  BSP_LCD_FillCircle(140, 160, 15);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillCircle(180, 120, 20);
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
}
