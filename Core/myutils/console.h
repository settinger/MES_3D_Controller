/*
 * console.h
 *
 *  Created on: May 3, 2022
 *      Author: Sam
 */

#ifndef SRC_CONSOLE_H_
#define SRC_CONSOLE_H_

#include <stdint.h>
#include <stdbool.h>

#include "app.h"
#include "color.h"

#define CONSOLE_PROMPT ("> ")
#define PARAMETER_SEPARATOR (' ')
#define ENDLINE "\r\n"
#define CONSOLE_COMMAND_MAX_LENGTH 256        // longest possible line

// The C library itoa is sometimes a complicated function and the library costs aren't worth it
// so this is implements the parts of the function needed for console.
#define CONSOLE_USE_BUILTIN_ITOA  1

// Console init and operate functions called from main.c
void ConsoleInit(void);
structAppState ConsoleProcess(structAppState currentStatus);

// Enum of instruction response, called from consoleCommands.c
typedef enum {
  COMMAND_SUCCESS = 0U,
  COMMAND_PARAMETER_ERROR = 2U,
  COMMAND_PARAMETER_END = 3U,
  COMMAND_ERROR = 0XFFU
} commandResult;

// Enum of commands for the web client
typedef enum {
  MOVE_CURSOR   = 'm',
  DRAW_SPOT     = 'd',
  MOVE_AND_DRAW = 'x',
  COLOR_CHANGE  = 'c',
  RESIZE_CURSOR = 'r'
} clientCommand;

// Commands for receiving/sending strings
// Must be null terminated!
commandResult ConsoleSendString(const char *buffer);
commandResult ConsoleSendLine(const char *buffer);
commandResult ConsoleMoveEuler(clientCommand command, float theta, float phi);
commandResult ConsoleResizeCursor(uint16_t size);
commandResult ConsoleChangeColor(clientColor color);

#endif /* SRC_CONSOLE_H_ */
