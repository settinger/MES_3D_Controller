const scene = new THREE.Scene();

// Axes in Frame N (inertial reference frame)
const Nx = new THREE.Vector3(1, 0, 0);
const Ny = new THREE.Vector3(0, 1, 0);
const Nz = new THREE.Vector3(0, 0, 1);
const N0 = new THREE.Vector3(0, 0, 0);
const origPosition = new THREE.Vector3(0, 1, 0); // Resting location of cursor
const origRotation = new THREE.Euler(0, 0, 0);

// Globals for cursor color and size
let cursorColor = "007BB8"; // ASCII code for a nice azulejo blue
let cursorSize = 20; // Starting radius of cursor (in U-V map pixel units)

// Define camera and lighting
const camera = new THREE.PerspectiveCamera(
  75,
  window.innerWidth / window.innerHeight,
  0.1,
  1000
);
const origCameraSpot = new THREE.Vector3(1, 1.2, 2.5);
camera.position.set(...origCameraSpot);
//camera.position.set(5, 1, 1);
camera.lookAt(N0);

const light = new THREE.DirectionalLight(0xffffff, 1);
light.position.set(2, 1, 2.5);
scene.add(light);
const light2 = new THREE.DirectionalLight(0xffffff, 1);
light2.position.set(-2, 1, -2.5);
scene.add(light2);

const turnCam = (x) => {
  const rot = new THREE.Matrix3().set(
    Math.cos(x),
    0,
    Math.sin(x),
    0,
    1,
    0,
    -Math.sin(x),
    0,
    Math.cos(x)
  );
  camera.position.applyMatrix3(rot);
  camera.lookAt(N0);

  refresh();
};

// Add camera-rotate button controls
const lCam = document.getElementById("lCam");
const rCam = document.getElementById("rCam");
let lTimer, rTimer;
lCam.addEventListener("mousedown", () => {
  turnCam(-0.1);
});
lCam.addEventListener("mousedown", async () => {
  clearInterval(lTimer);
  lTimer = setInterval(() => {
    turnCam(-0.1); // TODO: rotate camera -y
  }, 200);
});
lCam.addEventListener("mouseup", () => {
  clearInterval(lTimer);
});
lCam.addEventListener("mouseleave", () => {
  clearInterval(lTimer);
});
rCam.addEventListener("mousedown", () => {
  turnCam(0.1);
});
rCam.addEventListener("mousedown", async () => {
  clearInterval(rTimer);
  rTimer = setInterval(() => {
    turnCam(0.1); // TODO: rotate camera -y
  }, 200);
});
rCam.addEventListener("mouseup", () => {
  clearInterval(rTimer);
});
rCam.addEventListener("mouseleave", () => {
  clearInterval(rTimer);
});

// Either load a bitmap texture OR an SVG texture
// I only implemented SVG thus far
const texCallback = (texture) => {
  const material = new THREE.MeshLambertMaterial({ map: texture });
  surface.material = material;
};

const loader = new THREE.TextureLoader();
const updateTexture = () => {
  if (true) {
    const texture = loader.load(canvas.toDataURL("image/jpg"), texCallback);
  } else {
    const svgData = new XMLSerializer().serializeToString(mySVG);
    const svgURI = `data:image/svg+xml,${svgData}`;
    const texture = loader.load(svgURI, texCallback);
  }
};

// Frame refresh
const refresh = () => {
  renderer.render(scene, camera);
};

// Define renderer and where it lives on the webpage
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight); // TODO: add an onResize listener
//document.body.appendChild(renderer.domElement);
document.body.insertBefore(renderer.domElement, document.body.firstChild);

// Define the geometries and materials
const cursorGeo = new THREE.SphereGeometry(cursorSize / 300); // The cursor that indicates where paint will appear
const surfaceGeo = new THREE.SphereGeometry(1); // The surface on which the paint will appear
const cursorMaterial = new THREE.MeshLambertMaterial({
  color: parseInt(`0x${cursorColor}`),
});
const surfaceMaterial = new THREE.MeshLambertMaterial({
  color: 0xffffff,
  opacity: 0.5,
  transparent: true,
});

// Create objects
const cursor = new THREE.Mesh(cursorGeo, cursorMaterial); // The cursor that indicates where paint will appear
const surface = new THREE.Mesh(surfaceGeo, surfaceMaterial); // The surface on which the paint will appear
const axesN = new THREE.AxesHelper(5); // Axes to help orient us in the inertial reference frame
const axesB = new THREE.AxesHelper(0.5); // Axes to show the cursor's reference frame
scene.add(surface);
scene.add(axesN);
scene.add(cursor);
cursor.add(axesB);
cursor.position.set(...origPosition);
// These rotations align the texture map so (0, 0) on the texture is mapped to the smallest φ and smallest θ:
surface.rotateOnAxis(Nz, -Math.PI / 2);
surface.rotateOnAxis(Ny, -Math.PI);

// I can't believe "rotateOnWorldAxis" just doesn't work and people accept that
// Anyway here are my kludges, one to apply a rotation and one to set theta/phi
const rotateInertial = (object, axis, angle, point = N0) => {
  let q = new THREE.Quaternion();
  q.setFromAxisAngle(axis, angle);
  object.applyQuaternion(q);

  object.position.sub(point);
  object.position.applyQuaternion(q);
  object.position.add(point);
};
const setAngleInertial = (object, theta, phi, point = N0) => {
  // Reset position in space
  object.position.set(...origPosition);
  object.rotation.set(...origRotation);

  // Set cube rotation to new values
  const eul = new THREE.Euler(phi, 0, theta, "XYZ");
  object.setRotationFromEuler(eul);

  // Set cube position to new values
  object.position.sub(point);
  object.position.applyEuler(eul);
  object.position.add(point);
};

// Perform this when serial monitor receives new position data
const orient = (θ, φ) => {
  setAngleInertial(cursor, (θ * Math.PI) / 180, (φ * Math.PI) / 180);
  renderer.render(scene, camera);
};

renderer.render(scene, camera);
updateTexture();
setTimeout(refresh, 10);
