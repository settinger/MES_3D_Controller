// Tools for creating and updating U-V map for the three.js object when the U-V map is an SVG

let textureSize = 2048;

let mySVG = newSVG("svg", {
  width: textureSize,
  height: textureSize,
  viewBox: `0 0 ${textureSize} ${textureSize}`,
});

const fillBackground = mySVG.appendSVG("rect", {
  x: 0,
  y: 0,
  width: textureSize,
  height: textureSize,
  fill: "%23FFFFFF",
});

/*
const rect2 = mySVG.appendSVG("rect", {
  x: 0,
  y: 0,
  width: 1600,
  height: 824,
  fill: "%23007BB855",
});
*/

/*
UV MAPPING
Don't forget, θ and φ are in degrees at this stage!
Confusingly, I chose on the STM32 to restrict θ to [-90, 90]
and restrict φ to [-180, 180]. I usually do the other way round,
but this was cribbing heavily from an NXP application note with
slightly different axes than I'm used to.
First, rotation φ is applied around the x-axis.
Next, rotation θ is applied around the body z-axis.
If θ=90 or θ=-90, any value of φ yields the same point intersecting
the inertial frame's X-axis. θ=-90 corresponds to 0 on the y-axis
of the texture map; θ=90 corresponds to 2048 on the y-axis.
I think φ=-180 is 0 on x axis of texture; φ=180 is max texture size.
So: we have to apply a transform to convert from euler angles to UV-map.
 */

const θ2y = (θ) => ((θ + 90) / 180) * textureSize;
const y2θ = (y) => (y * 180) / textureSize - 90;
const φ2x = (φ) => ((φ + 180) / 360) * textureSize;
const x2φ = (x) => (x * 360) / textureSize - 180;

// Draw a splotch of paint where the cursor currently is.
// Due to the U-V mapping it won't be a perfect circle or square or anything
const drawSplotch = (θ, φ) => {
  // The center of the splotch is here, but due to mapping it should not just be a circle
  // That's an issue for another time, probably
  let cx = φ2x(φ);
  let cy = θ2y(θ);

  mySVG.appendSVG("circle", { cx, cy, r: 20, fill: "%23007BB855" });

  updateTexture();
};
