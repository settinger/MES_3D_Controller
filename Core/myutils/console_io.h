/*
 * console_io.h
 * A wrapper between raw inputs and console operations
 * Expects console inputs to be receved on STM32F429I-DISC1 USART5
 * Ported from Elecia White's code here:
 * https://wokwi.com/projects/324879108372693587
 *
 *  Created on: May 3, 2022
 *      Author: Sam
 */

#ifndef SRC_CONSOLE_IO_H_
#define SRC_CONSOLE_IO_H_

#include <stdint.h>
#include "stdlib.h"
#include "usart.h"

typedef enum {
  CONSOLE_SUCCESS = 0u, CONSOLE_ERROR = 1u
} consoleError;

consoleError ConsoleIoInit(void);
consoleError ConsoleIoReceive(uint8_t *buffer, const uint32_t bufferLength,
    uint32_t *readLength);
consoleError ConsoleIoSend(const char *buffer); // Must be null-terminated string

#endif /* SRC_CONSOLE_IO_H_ */
