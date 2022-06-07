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
#include "color.h"

extern clientColor currentColor;
extern uint16_t currentSize;

void clearScreen() {
  BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
  BSP_LCD_FillRect(0, 0, 240, 320);
}
void prepareDisplay(void) {
  BSP_LCD_DisplayOn();
  BSP_LCD_LayerDefaultInit(0, LCD_FRAME_BUFFER_LAYER0);
  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(LCD_COLOR_LIGHTGRAY);
  BSP_LCD_SetBackColor(LCD_COLOR_LIGHTGRAY);
  BSP_LCD_SetFont(&Font16);

  //drawColorPicker();
  drawMainScreen(COLOR_DEFAULT, 20);
}

void drawMainScreen(clientColor color, uint16_t size) {
  // Draw an "Ink color" button and a "Cursor size" button
  clearScreen();
  BSP_LCD_SetTextColor(0xFF000000 | color);
  BSP_LCD_FillRect(40, 20, 160, 120);
  BSP_LCD_SetTextColor(LCD_COLOR_DARKGRAY);
  BSP_LCD_FillCircle(120, 240, 50);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
//  BSP_LCD_DisplayStringAt(0, 72, "Ink color", CENTER_MODE);
//  BSP_LCD_DisplayStringAt(0, 232, "Cursor size", CENTER_MODE);
}

void drawColorPicker(void) {
  // Fill in some nice rectangles of color
  clearScreen();
  BSP_LCD_SetTextColor(0xFF000000 | COLOR_VIOLET);
  BSP_LCD_FillRect(0, 0, 120, 80);
  BSP_LCD_SetTextColor(0xFF000000 | COLOR_LIGHT_BLUE);
  BSP_LCD_FillRect(120, 0, 120, 80);
  BSP_LCD_SetTextColor(0xFF000000 | COLOR_DARK_BLUE); // Dark blue
  BSP_LCD_FillRect(0, 80, 120, 80);
  BSP_LCD_SetTextColor(0xFF000000 | COLOR_GREEN); // Green
  BSP_LCD_FillRect(120, 80, 120, 80);
  BSP_LCD_SetTextColor(0xFF000000 | COLOR_GOLD); // Gold
  BSP_LCD_FillRect(0, 160, 120, 80);
  BSP_LCD_SetTextColor(0xFF000000 | COLOR_ORANGE); // Orange
  BSP_LCD_FillRect(120, 160, 120, 80);
  BSP_LCD_SetTextColor(0xFF000000 | COLOR_RED); // Deep red
  BSP_LCD_FillRect(0, 240, 120, 80);
  BSP_LCD_SetTextColor(0xFF000000 | COLOR_PINK); // Pink
  BSP_LCD_FillRect(120, 240, 120, 80);
}

void drawSizePicker(void) {

}
