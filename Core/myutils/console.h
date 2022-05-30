/*
 * console.h
 * CLI code ported from Elecia White's:
 * https://wokwi.com/projects/324879108372693587
 *
 *  Created on: May 3, 2022
 *      Author: Sam
 */

#ifndef SRC_CONSOLE_H_
#define SRC_CONSOLE_H_

#include <stdint.h>
#include <stdbool.h>

#include "app.h"

#define CONSOLE_PROMPT ("> ")
#define PARAMETER_SEPARATOR (' ')
#define ENDLINE "\r\n"

// The C library itoa is sometimes a complicated function and the library costs aren't worth it
// so this is implements the parts of the function needed for console.
#define CONSOLE_USE_BUILTIN_ITOA  1

// Console init and operate functions called from main.c
void ConsoleInit(void);
structAppState ConsoleProcess(structAppState currentStatus);

// Structure of instruction response, called from consoleCommands.c
typedef enum {
  COMMAND_SUCCESS = 0U,
  COMMAND_PARAMETER_ERROR = 2U,
  COMMAND_PARAMETER_END = 3U,
  COMMAND_ERROR = 0XFFU
} commandResult;

// Commands for receiving and sending ints (using C stdlib atoi/itoa)
commandResult ConsoleReceive_int16(const char * buffer, const uint8_t parameterNumber, int16_t* parameterInt16);
commandResult ConsoleSend_int16(int16_t parameterInt);
commandResult ConsoleSend_int32(int32_t parameterInt);

// Commands for receiving/sending hexuint16 (implement their own versions of atoi/itoa)
commandResult ConsoleReceiveHex_uint16(const char * buffer, const uint8_t parameterNumber, uint16_t* parameterUint16);
commandResult ConsoleSendHex_uint16(uint16_t parameterUint16);
commandResult ConsoleSendHex_uint8(uint8_t parameterUint8);

// Commands for receiving/sending strings
// Must be null terminated!
commandResult ConsoleSendString(const char *buffer);
commandResult ConsoleSendLine(const char *buffer);

#endif /* SRC_CONSOLE_H_ */
