/*
 * console.c
 * CLI code ported from Elecia White's:
 * https://wokwi.com/projects/324879108372693587
 *
 *  Created on: May 3, 2022
 *      Author: Sam
 */

#include <string.h>  // for NULL
#include <stdlib.h>  // for atoi and itoa (though this code implement a version of that)
#include <stdbool.h>
#include "console.h"
#include "console_io.h"
#include "console_commands.h"
#include "app.h"

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif

#define NOT_FOUND -1
#define INT16_MAX_STRLEN 8 // six characters plus two NULL
#define INT32_MAX_STRLEN 16
#define NULL_CHAR '\0'
#define CR_CHAR '\r'
#define LF_CHAR '\n'

// global variables
char receiveBuffer[CONSOLE_COMMAND_MAX_LENGTH];
uint32_t receivedSoFar;
bool receiveBufferNeedsChecking = false;

// local functions
static uint32_t ConsoleResetBuffer(char receiveBuffer[],
    const uint32_t filledLength, uint32_t usedSoFar);

/*
 * ConsoleResetBuffer
 * In an ideal world, this would just zero out the buffer.
 * However, there are times when the buffer may have data
 * beyond what was used in the last command. We don't want
 * to lose that data so we move it to the start of the
 * command buffer and then zero the rest.
 */

static uint32_t ConsoleResetBuffer(char receiveBuffer[],
    const uint32_t filledLength, uint32_t usedSoFar) {
  uint32_t remaining = (filledLength - usedSoFar);
  uint32_t i = 0;

  while (usedSoFar < filledLength) {
    receiveBuffer[i] = receiveBuffer[usedSoFar]; // Move the end to the start
    i++;
    usedSoFar++;
  }
  // Fill the rest of the buffer with NULL characters
  for ( /* nothing */; i < CONSOLE_COMMAND_MAX_LENGTH; i++) {
    receiveBuffer[i] = NULL_CHAR;
  }
  return remaining;
}

/*
 * ConsoleInit
 * Initialize console interface and what it depends on.
 */
void ConsoleInit(void) {
  uint32_t i;
  ConsoleIoInit();
  ConsoleIoSend("\r\n######################\r\n\nDevice has turned on.\r\n");
  ConsoleIoSend("Keyboard interface online.");
  ConsoleIoSend(ENDLINE);
  receivedSoFar = 0u;

  for (i = 0u; i < CONSOLE_COMMAND_MAX_LENGTH; i++) {
    receiveBuffer[i] = NULL_CHAR;
  }
}

/*
 * ConsoleProcess
 * Look for new inputs that have arrived; if valid command, run right away.
 * No checking for endlines for this particular application.
 * Call this from a loop to handle commands as they become available.
 */
structAppState ConsoleProcess(structAppState currentStatus) {
  uint32_t received; // Number of inputs received

  ConsoleIoReceive((uint8_t*) &(receiveBuffer[receivedSoFar]),
  CONSOLE_COMMAND_MAX_LENGTH - receivedSoFar, &received);

  if ((received > 0) || receiveBufferNeedsChecking) {
    receivedSoFar += received;
    // Any input needs to be processed, not just inputs after endline
//    // Check if received inputs are valid instructions
//    if (GAME_LEVEL_SELECT == currentStatus) {
//      process = levelSelectProcessInput(receiveBuffer[0]);
//    } else {
//      process = gameProcessInput(receiveBuffer[0]);
//    }
// TODO: replace above with project-relevant process
    // Reset buffer by moving any leftovers and nulling the rest.
    // This clears up to and including the front endline character.
    receivedSoFar = ConsoleResetBuffer(receiveBuffer, receivedSoFar, 1);
    receiveBufferNeedsChecking = (receivedSoFar > 0); // (receivedSoFar > 0 ? true : false);
  }

  return APP_NORMAL; // TODO: replace
}

/*
 * The C library itoa is sometimes a complicated function and the library costs
 * aren't worth it, so this implements the necessary parts for console.
 */
#if CONSOLE_USE_BUILTIN_ITOA
#define itoa smallItoa
static void smallItoa(int in, char *outBuffer, int radix) {
  bool isNegative = false;
  int tmpIn;
  int stringLen = 1u;  // String will be at least as long as the NULL character

  if (in < 0) {
    isNegative = true;
    in = -in;
    stringLen++;
  }

  // Get length of string to be composed
  tmpIn = in;
  while ((int) tmpIn / radix != 0) {
    tmpIn = (int) tmpIn / radix;
    stringLen++;
  }

  // Starting with NULL character, fill in string backwards
  *(outBuffer + stringLen) = NULL_CHAR;
  stringLen--;

  tmpIn = in;
  do {
    *(outBuffer + stringLen) = (tmpIn % radix) + '0';
    tmpIn = (int) (tmpIn / radix);
  } while (stringLen--);

  if (isNegative) {
    *(outBuffer) = '-';
  }
}
#endif

/* ConsoleSendString
 * Send a null terminated string to the console.
 * This is a light wrapper around ConsoleIoSend. It uses the same
 * API convention as the rest of the top level ConsoleSendX APIs
 * while exposing this functionality at the top level so that the
 * lower level console_io module doesn't need to be a dependency.
 */
commandResult ConsoleSendString(const char *buffer) {
  ConsoleIoSend(buffer);
  return COMMAND_SUCCESS;
}

commandResult ConsoleSendLine(const char *buffer) {
  ConsoleIoSend(buffer);
  ConsoleIoSend(ENDLINE);
  return COMMAND_SUCCESS;
}
