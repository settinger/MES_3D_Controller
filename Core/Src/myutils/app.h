/*
 * app.h
 *
 *  Created on: May 29, 2022
 *      Author: Sam
 */

#ifndef SRC_MYUTILS_APP_H_
#define SRC_MYUTILS_APP_H_

#include <stdio.h>
#include <stdbool.h>

// Structure of app states
typedef enum structAppState {
  SYSTEM_BOOT = 0U,
  APP_INIT = 1U,
  APP_NORMAL = 2U,
  APP_ERROR = 0xFFU
} structAppState;

void appInit(void);


#endif /* SRC_MYUTILS_APP_H_ */
