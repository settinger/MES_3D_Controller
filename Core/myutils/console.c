/*
 * console.c
 * CLI code ported from Elecia White's:
 * https://wokwi.com/projects/324879108372693587
 *
 *  Created on: May 3, 2022
 *      Author: Sam
 */

#include "console.h"

#include <string.h>  // for NULL
#include <stdlib.h>  // for atoi and itoa (though this code implement a version of that)
#include <stdbool.h>

#include "app.h"
#include "console_io.h"
#include "color.h"

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

/*
 * ConsoleCursorEuler
 * Instructions of the form "m,[theta],[phi]" tell the web client
 * to set the cursor to that Euler angle
 */
commandResult ConsoleCursorEuler(float theta, float phi) {
  char command[100];
  sprintf(command, "m,%4.2f,%4.2f", theta, phi);
  ConsoleSendLine(command);
  return COMMAND_SUCCESS;
}

/*
 * ConsoleDrawEuler
 * Instructions of the form "d,[theta],[phi]" tell the web client
 * to draw a texture at that Euler angle without moving the cursor
 */
commandResult ConsoleDrawEuler(float theta, float phi) {
  char command[100];
  sprintf(command, "d,%4.2f,%4.2f", theta, phi);
  ConsoleSendLine(command);
  return COMMAND_SUCCESS;
}

/*
 * ConsoleDrawCursorEuler
 * Instructions of the form "x,[theta],[phi]" tell the web client
 * to set the cursor to that Euler angle AND draw a texture there
 */
commandResult ConsoleDrawCursorEuler(float theta, float phi) {
  char command[100];
  sprintf(command, "x,%4.2f,%4.2f", theta, phi);
  ConsoleSendLine(command);
  return COMMAND_SUCCESS;
}

/*
 * ConsoleMoveEuler
 * Use the input euler angle to draw a new spot on the texture
 * and/or update the location of the cursor.
 */
commandResult ConsoleMoveEuler(clientCommand command, float theta, float phi) {
  char toSend[100];
  sprintf(toSend, "%c,%4.2f,%4.2f", command, theta, phi);
  ConsoleSendLine(toSend);
  return COMMAND_SUCCESS;
}

/*
 * ConsoleResizeCursor
 * Use the input to resize the cursor drawing on the texture
 */
commandResult ConsoleResizeCursor(uint16_t size) {
  char toSend[100];
  sprintf(toSend, "r,%u", size);
  ConsoleSendLine(toSend);
  return COMMAND_SUCCESS;
}

/*
 * ConsoleChangeColor
 * Use the input to change the color on the texture
 */
commandResult ConsoleChangeColor(clientColor color) {
  char toSend[100];
  sprintf(toSend, "c,%06X", color);
  ConsoleSendLine(toSend);
  return COMMAND_SUCCESS;
}
