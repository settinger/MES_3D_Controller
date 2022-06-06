/*
 * mykalman.h
 * Partly based on the MPU6050 library by Konstantin Bulanov: https://github.com/leech001/MPU6050
 * That library is released under a GNU GPL v3 license.
 * And that library is based on the C++ Kalman Filter library by Kristian Lauszus: https://github.com/TKJElectronics/KalmanFilter
 * That library is released under a GNU GPL v2 license.
 *
 * Also using this application note from NXP: https://www.nxp.com/files-static/sensors/doc/app_note/AN3461.pdf
 *
 *  Created on: Jun 3, 2022
 *      Author: Sam
 */

#ifndef KALMAN_MYKALMAN_H_
#define KALMAN_MYKALMAN_H_

#include <stdint.h>

// Sensor struct
typedef struct {
  float Ax;
  float Ay;
  float Az;

  float Gx;
  float Gy;
  float Gz;

  float KalmanStateTheta;
  float KalmanStatePhi;
} sensors_t;

// Kalman struct
typedef struct {
  float Q_angle;
  float Q_bias;
  float R_measure;
  float angle;
  float bias;
  float P[2][2];
} Kalman_t;

void getReadings(sensors_t *sensorStruct, uint32_t t);
float kalmanUpdate(Kalman_t *Kalman, float newAngle, float newRate, float dt);
float closestWrap(float angle);

#endif /* KALMAN_MYKALMAN_H_ */
