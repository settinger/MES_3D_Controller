// Tools for creating and updating UV-map for the three.js object

let size = 2048;
canvas = document.createElement("canvas");
ctx = canvas.getContext("2d");
canvas.id = "textureMap";
canvas.width = size;
canvas.height = size;
canvas.style.display = "none";
document.body.append(canvas);

ctx.fillStyle = "white";
ctx.fillRect(0, 0, size, size);
ctx.fillStyle = "green";
ctx.fillRect(0, 1024, size / 2, 104);

const createTexture = (size = 2048) => {};

let mySVG = document.createElementNS("http://www.w3.org/2000/svg", "svg");
mySVG.setAttribute("width", 1024);
mySVG.setAttribute("height", 1024);
mySVG.setAttribute("viewBox", "0 0 1024 1024");
//mySVG.style.display = "none";
let blah = document.createElementNS("http://www.w3.org/2000/svg", "rect");
mySVG.append(blah);
blah.setAttribute("x", 0);
blah.setAttribute("y", 0);
blah.setAttribute("width", size);
blah.setAttribute("height", size);
blah.setAttribute("fill", "white");
let blah2 = document.createElementNS("http://www.w3.org/2000/svg", "rect");
mySVG.append(blah2);
blah2.setAttribute("x", 0);
blah2.setAttribute("y", 0);
blah2.setAttribute("width", 512);
blah2.setAttribute("height", 400);
blah2.setAttribute("fill", "blue");

//document.body.append(mySVG);

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
