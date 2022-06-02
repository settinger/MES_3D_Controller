/*
 * Frame.h
 *
 *  Created on: Jun 1, 2022
 *      Author: Sam
 */

#ifndef KINEMATICS_FRAME_H_
#define KINEMATICS_FRAME_H_

/*
 * Properties of a frame I need to care about:
 * Origin relative to frame N, float[3]
 * Orientation relative to frame N? Quaternion?
 * First and second derivatives relative to frame N
 */

/*
 * Properties of a vector I need to care about:
 * What frame it's represented in [Frame]
 * Convert from Frame A to Frame B
 */

class Frame {
public:
  Frame();
  virtual ~Frame();
};

#endif /* KINEMATICS_FRAME_H_ */
