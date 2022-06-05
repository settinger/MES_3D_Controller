// Tools for creating and updating U-V map for the three.js object when the U-V map is a bitmap (canvas)

let size = 2048;
canvas = document.createElement("canvas");
ctx = canvas.getContext("2d");
canvas.id = "textureMap";
canvas.width = size;
canvas.height = size;
canvas.style.display = "none"; // Are these two lines necessary?
document.body.append(canvas); // Are these two lines necessary?

ctx.fillStyle = "white";
ctx.fillRect(0, 0, size, size);
ctx.fillStyle = "green";
ctx.fillRect(0, 1024, size / 2, 104);

const createTexture = (size = 2048) => {};

/*
UV MAPPING
x axis of canvas corresponds to θ (y-axis) on sphere
x = 0 => θ = 0; x = size => θ = 2π
y axis of canvas corresponds to φ (x-axis) on sphere
y = 0 => φ = π; y = size => φ = -π
*/
const thetaToX = (θ) => (θ / (2 * Math.PI)) * size;
const xToTheta = (x) => (x / size) * 2 * Math.PI;
const phiToY = (φ) => ((Math.PI - φ) * size) / (2 * Math.PI);
const yToPhi = (y) => Math.PI - (y / size) * 2 * Math.PI;

const drawBlop = (θ, φ) => {};
