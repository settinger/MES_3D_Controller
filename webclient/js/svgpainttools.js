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
First, rotation θ is applied around the z-axis.
Next, rotation φ is applied around the x-axis.
If θ=90 or θ=-90, any value of φ yields the same point intersecting
the inertial frame's X-axis. θ=-90 corresponds to 0 on the V-axis
of the texture map; θ=90 corresponds to 2048 on the V-axis.
I think φ=-180 is 0 on U-axis; φ=180 is max texture size.
So: we have to apply a transform to convert from euler angles to UV-map.
 */

const θ2v = (θ) => ((θ + 90) / 180) * textureSize;
const v2θ = (v) => (v * 180) / textureSize - 90;
const φ2u = (φ) => ((φ + 180) / 360) * textureSize;
const u2φ = (u) => (u * 360) / textureSize - 180;

// Draw a splotch of paint where the cursor currently is.
// Due to the U-V mapping it won't be a perfect circle or square or anything
const drawSplotch = (θ, φ, update = true) => {
  // The center of the splotch is here, but due to mapping it should not just be a circle
  // That's an issue for another time, probably
  let cx = φ2u(φ);
  let cy = θ2v(θ);

  mySVG.appendSVG("circle", { cx, cy, r: 20, fill: "%23007BB855" });

  if (update) {
    updateTexture();
  }
};

/*
For the type of spherical symmetry I want, there are 48 points drawn per stroke
I think the easiest way to do this is to convert (θ, φ) to cartesian (x, y, z)
and convert *that* to (u, v). But that involves writing more transforms.

Assume radius is 1. x*x+y*y+z*z = 1. Keep things easy.
*/
const θφ2xyz = (θ, φ) => {
  let d2r = Math.PI / 180; // convert degrees to radians
  let x = Math.sin(-θ * d2r);
  let y = Math.cos(θ * d2r) * Math.cos(φ * d2r);
  let z = Math.cos(θ * d2r) * Math.sin(φ * d2r);
  return [x, y, z];
};

const xyz2uv = (x, y, z) => {
  let r2d = 180 / Math.PI; // Convert radians to degrees
  let θ = -Math.asin(x) * r2d;
  let φ = Math.acos(y / Math.sqrt(1 - x * x)) * r2d * Math.sign(z);
  return [φ2u(φ), θ2v(θ)];
};

const xyzSplotch = (x, y, z) => {
  let r2d = 180 / Math.PI; // Convert radians to degrees
  let θ = -Math.asin(x) * r2d;
  let φ = Math.acos(y / Math.sqrt(1 - x * x)) * r2d * Math.sign(z);
  let cx = φ2u(φ);
  let cy = θ2v(θ);
  mySVG.appendSVG("circle", { cx, cy, r: 20, fill: "%23007BB855" });
};

// For now: a hack to decimate the number of times this runs
let dec = 0;
const symmetricSplotch = (θ, φ) => {
  dec++;
  if (dec < 4) return;
  dec = 0;
  let [x0, y0, z0] = θφ2xyz(θ, φ);
  for (let x of [x0, -x0]) {
    for (let y of [y0, -y0]) {
      for (let z of [z0, -z0]) {
        xyzSplotch(x, y, z);
        xyzSplotch(x, z, y);
        xyzSplotch(y, x, z);
        xyzSplotch(y, z, x);
        xyzSplotch(z, x, y);
        xyzSplotch(z, y, x);
      }
    }
  }
  updateTexture();
};
