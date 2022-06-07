/*
 * app.c
 *
 *  Created on: May 29, 2022
 *      Author: Sam
 */

#include "app.h"
#include "my_graphics.h"
#include "color.h"
#include "console.h"

extern clientColor currentColor;
extern uint16_t currentSize;

void appInit(void) {
  prepareDisplay();
}

structAppState mainScreenTouchHandler(int16_t x, int16_t y) {
  // If the touch registers as hitting a button, redraw and change state
  structAppState state = APP_NORMAL;
  if ((60 < x) && (x < 180)) {
    if ((20 < y) && (y < 140)) {
      drawColorPicker();
      state = APP_COLORPICKER;
    } else if ((180 < y) && (y < 200)) {
      drawSizePicker();
      state = APP_SIZEPICKER;
    }
  }
  return state;
}

structAppState colorPickerTouchHandler(int16_t x, int16_t y) {
  // There are eight sections of the screen
  // When touched, update color data on STM32, update color data on web client, update color data in EEPROM
  if (y < 80) {
    currentColor = (x < 120) ? COLOR_VIOLET : COLOR_LIGHT_BLUE;
  } else if (y < 160) {
    currentColor = (x < 120) ? COLOR_DARK_BLUE : COLOR_GREEN;
  } else if (y < 240) {
    currentColor = (x < 120) ? COLOR_GOLD : COLOR_ORANGE;
  } else if (y < 320) {
    currentColor = (x < 120) ? COLOR_RED : COLOR_PINK;
  } else {
    currentColor = COLOR_DEFAULT;
  }
  ConsoleChangeColor(currentColor);
  drawMainScreen(currentColor, currentSize);
  return APP_NORMAL;
}

structAppState sizePickerTouchHandler(int16_t x, int16_t y) {
  // Six dots on screen: 5, 10, 20, 30, 40, 50
}
