// Tools for creating and updating U-V map for the three.js object when the U-V map is an SVG

let size = 2048;

let mySVG = newSVG("svg", {
  width: size,
  height: size,
  viewBox: `0 0 ${size} ${size}`,
});

const rect1 = mySVG.appendSVG("rect", {
  x: 0,
  y: 0,
  width: size,
  height: size,
  fill: "white",
});

const rect2 = mySVG.appendSVG("rect", {
  x: 0,
  y: 0,
  width: 512,
  height: 400,
  fill: "blue",
});

/*
UV MAPPING
x axis of canvas corresponds to θ (y-axis) on sphere
x = 0 => θ = 0; x = size => θ = 2π
y axis of canvas corresponds to φ (x-axis) on sphere
y = 0 => φ = π; y = size => φ = -π
 */
const θ2x = (θ) => (θ / (2 * Math.PI)) * size;
const x2θ = (x) => (x / size) * 2 * Math.PI;
const φ2y = (φ) => ((Math.PI - φ) * size) / (2 * Math.PI);
const y2φ = (y) => Math.PI - (y / size) * 2 * Math.PI;

// Draw a splotch of paint where the cursor currently is.
// Due to the U-V mapping it won't be a perfect circle or square or anything
const drawSplotch = (θ, φ) => {
  // The center of the splotch is here, but due to mapping it should not just be a circle
  let x = θ2x(θ);
  let y = φ2y(φ);
};
