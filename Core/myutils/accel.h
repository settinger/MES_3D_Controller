/*
 * accel.h
 *
 *  Created on: May 30, 2022
 *      Author: Sam
 */

#ifndef MYUTILS_ACCEL_H_
#define MYUTILS_ACCEL_H_

#include "stdio.h"
#include "lis2dh_reg.h"

void accel_init(void);
void accel_getValues(void);

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
    uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
    uint16_t len);
static void tx_com(uint8_t *tx_buffer, uint16_t len);
static void platform_delay(uint32_t ms);
static void platform_init(void);

#endif /* MYUTILS_ACCEL_H_ */
