/*
 * app.c
 *
 *  Created on: May 29, 2022
 *      Author: Sam
 */

#include "app.h"
#include "my_graphics.h"

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

}

structAppState sizePickerTouchHandler(int16_t x, int16_t y) {
}
