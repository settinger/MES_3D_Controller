/*
 * lis2dh.c
 * A library for interfacing with an LIS2DH Accelerometer connected over SPI
 * Based on https://github.com/owainm713/LIS3DH-Python-Module/blob/master/LIS3DH.py
 *
 *  Created on: May 29, 2022
 *      Author: Sam
 */

#include "lis2dh.h"
#include "stdio.h"

static void singleRegisterRead(uint8_t reg) {
  // set read/write bit, clear multi-read bit
  uint8_t xfer = (0b10<<6) + reg;
}
