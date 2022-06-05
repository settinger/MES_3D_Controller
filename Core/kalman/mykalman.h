/*
 * fusion.h
 * Largely based on the MPU6050 library by Konstantin Bulanov: https://github.com/leech001/MPU6050
 * That library is released under a GNU GPL v3.0 license.
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

  float KalmanEstimatedTheta;
  float KalmanEstimatedPhi;
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

#endif /* KALMAN_MYKALMAN_H_ */
