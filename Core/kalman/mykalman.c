/*
 * mykalman.c
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

#include "mykalman.h"
#include "accel.h"
#include "stm32f429i_discovery_gyroscope.h"
#include <math.h>

#define PI 3.141592f
#define HALFPI 1.570796f
#define RAD_TO_DEG 180/PI

uint32_t timer;
float acceleration_mg[3];
float gyro_mdps[3];

Kalman_t KalmanTheta = { .Q_angle = 0.001f, .Q_bias = 0.003f,
    .R_measure = 0.03f, .angle = 0.0f };

Kalman_t KalmanPhi = { .Q_angle = 0.001f, .Q_bias = 0.003f, .R_measure = 0.03f,
    .angle = 0.0 };

void getReadings(sensors_t *sensorStruct, uint32_t t) {
  accel_read(acceleration_mg);
  BSP_GYRO_GetXYZ(gyro_mdps);

  /*
   * This is obnoxious but I may need to compensate for the fact that
   * three.js camera only supports +Z out of screen, +Y up, +X right.
   * This should make the axes match when the board is being held with
   * the USB-mini cable toward the ceiling.
   *
   * Phi (roll) is defined as rotation along the X axis
   * Theta (pitch) is defined as rotation along the Z axis
   * Psi (yaw) is defined as rotation along the Y axis (initially aligned with gravity)
   */

  sensorStruct->Ax = acceleration_mg[0] / 1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Ay = acceleration_mg[1] / 1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Az = acceleration_mg[2] / 1000; // Unit: g (~9.8 m/s^2)
  sensorStruct->Gx = gyro_mdps[0] / 1000; // Unit: degrees per second
  sensorStruct->Gy = gyro_mdps[1] / 1000; // Unit: degrees per second
  sensorStruct->Gz = gyro_mdps[2] / 1000; // Unit: degrees per second

  float dt = (float) (t) / 1000; // Unit: seconds

  /*
   * Begin Kalman filter
   * State estimation: use accelerometer data to get naive
   * values for theta and phi
   */

  /*
   * Compute phi from accelerometers, return result in degrees
   * When holding the board upright with USB-mini pointed toward the
   * ceiling and screen toward the user, phi is measured as the angle
   * between the board's Y axis and gravity.
   * This assumes the force of gravity is much greater than the force
   * being applied by the user to the controller.
   */
  float phi = 0.0;
  float force_magnitude = sqrtf(
      sensorStruct->Ax * sensorStruct->Ax + sensorStruct->Ay * sensorStruct->Ay
          + sensorStruct->Az * sensorStruct->Az);
  if (force_magnitude != 0.0) {
    phi = atan2f(-1 * sensorStruct->Az, -1 * sensorStruct->Ay) * RAD_TO_DEG;
  }

  /*
   * Compute theta from accelerometers, return result in degrees
   * Theta is measured as the angle made by the X-axis acceleration
   * component and the norm of the other two acceleration components.
   */
  float norm_YZ = sqrtf(
      sensorStruct->Ay * sensorStruct->Ay
          + sensorStruct->Az * sensorStruct->Az);
  float theta = atan2f(-1 * sensorStruct->Ax, norm_YZ) * RAD_TO_DEG;

  /*
   * The above values are limited to [-180, 180] but existing state might
   * extend beyond those; in such circumstances we forgo Kalman updates
   * and handle the discontinuities another way.
   */

  if ((phi < -90 && sensorStruct->KalmanStatePhi > 90)
      || (phi > 90 && sensorStruct->KalmanStatePhi < 90)) {
    KalmanPhi.angle = phi;
    sensorStruct->KalmanStatePhi = phi;
  } else {
    sensorStruct->KalmanStatePhi = kalmanUpdate(&KalmanPhi, phi,
        -sensorStruct->Gx, dt);
  }

  // If phi goes too high or low, set the observed rate back to keep it within limits
  // This feels hacky and fishy to me and I don't get why the original authors did this
  if (fabsf(sensorStruct->KalmanStatePhi) > 90) {
    sensorStruct->Gz = -1 * sensorStruct->Gz;
  }

  sensorStruct->KalmanStateTheta = kalmanUpdate(&KalmanTheta, theta,
      sensorStruct->Gz, dt);
}

/*
 * The update steps of the Kalman filter process
 */
float kalmanUpdate(Kalman_t *Kalman, float newAngle, float newRate, float dt) {
  // Predict next state using existing state and observed gyro data
  float rate = newRate - Kalman->bias;
  Kalman->angle += dt * rate;

  // Predict the estimate covariance P
  Kalman->P[0][0] += dt
      * (dt * Kalman->P[1][1] - Kalman->P[0][1] - Kalman->P[1][0]
          + Kalman->Q_angle);
  Kalman->P[0][1] -= dt * Kalman->P[1][1];
  Kalman->P[1][0] -= dt * Kalman->P[1][1];
  Kalman->P[1][1] += Kalman->Q_bias * dt;

  // Innovation covariance
  float S = Kalman->P[0][0] + Kalman->R_measure;

  // Optimal Kalman gain
  float K[2];
  K[0] = Kalman->P[0][0] / S;
  K[1] = Kalman->P[1][0] / S;

  // Measurement post-fit residual and state
  float y = newAngle - Kalman->angle;
  Kalman->angle += K[0] * y;
  Kalman->bias += K[1] * y;

  // Updated estimate covariance
  float P00_temp = Kalman->P[0][0];
  float P01_temp = Kalman->P[0][1];
  Kalman->P[0][0] -= K[0] * P00_temp;
  Kalman->P[0][1] -= K[0] * P01_temp;
  Kalman->P[1][0] -= K[1] * P00_temp;
  Kalman->P[1][1] -= K[1] * P01_temp;

  return Kalman->angle;
}

/*
 * Closest Wrap - a hacky way to do away with discontinuties due to trig functions being limited to [-pi, pi]
 * If the angle is in [0, 360], the closest wrap-around angle is 180 degrees; if the angle is [360, 720],
 * closest is 540, and so on. Same with negative values: [-360, 0] yields -180; [-720, -360] yields -540.
 */

float closestWrap(float angle) {
  int steps = 0;
  while (angle < 0) {
    steps--;
    angle += 360;
  }
  while (angle > 360) {
    steps++;
    angle -= 360;
  }
  return (float) (180 + steps * 360);
}
