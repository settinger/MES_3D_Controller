/*
 * console_commands.h
 * The command interface used by console.c
 * Ported from Elecia White's code here:
 * https://wokwi.com/projects/324879108372693587
 *
 *  Created on: May 3, 2022
 *      Author: Sam
 */

#ifndef SRC_CONSOLE_COMMANDS_H_
#define SRC_CONSOLE_COMMANDS_H_

#include <stdint.h>
#include "console.h"

#define CONSOLE_COMMAND_MAX_COMMAND_LENGTH 10 // Length of max console command
#define CONSOLE_COMMAND_MAX_LENGTH 256        // whole command with arguments
#define CONSOLE_COMMAND_MAX_HELP_LENGTH 64    // Shorten to reduce RAM use

#if CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
#define HELP(x)  (x)
#else
#define HELP(x)  0
#endif

typedef commandResult(*consoleCommand)(const char buffer[]);

typedef struct consoleCommandStruct {
  const char* name;
  consoleCommand execute;
#if CONSOLE_COMMAND_MAX_HELP_LENGTH > 0
  char help[CONSOLE_COMMAND_MAX_HELP_LENGTH];
#else
  uint8_t doNotCare;
#endif
} consoleCommandTable;

#define CONSOLE_COMMAND_TABLE_END {NULL, NULL, HELP("")}

const consoleCommandTable * ConsoleCommandsGetTable(void);
//commandResult commandProcess(char oneChar);

#endif /* SRC_CONSOLE_COMMANDS_H_ */
