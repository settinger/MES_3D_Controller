const scene = new THREE.Scene();

// Axes in Frame N (inertial reference frame)
const Nx = new THREE.Vector3(1, 0, 0);
const Ny = new THREE.Vector3(0, 1, 0);
const Nz = new THREE.Vector3(0, 0, 1);
const N0 = new THREE.Vector3(0, 0, 0);
const origPosition = new THREE.Vector3(0, 0, 0);
const origRotation = new THREE.Euler(0, 0, 0);

// Define camera and lighting
const camera = new THREE.PerspectiveCamera(
  75,
  window.innerWidth / window.innerHeight,
  0.1,
  1000
);
camera.position.set(2, 2, 5);
camera.lookAt(N0);
const light = new THREE.DirectionalLight(0xffffff, 1);
light.position.set(4, 2, 5);
scene.add(light);

// Load textures, either bitmap or SVG
const texCallback = (texture) => {
  const material = new THREE.MeshLambertMaterial({ map: texture });
  surface.material = material;
};

// Either load a bitmap texture OR an SVG texture
const loader = new THREE.TextureLoader();
if (true) {
  const texture = loader.load(canvas.toDataURL("image/jpg"), texCallback);
} else {
  const svgData = new XMLSerializer().serializeToString(mySVG);
  const svgURI = `data:image/svg+xml,${svgData}`;
  const texture = loader.load(svgURI, texCallback);
}

// Define renderer and where it lives on the webpage
const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight); // TODO: add an onResize listener
document.body.appendChild(renderer.domElement);

// Define the geometries and materials
const geometry = new THREE.BoxGeometry(0.4, 0.4, 0.4);
const cursorGeo = new THREE.SphereGeometry(0.2); // The cursor that indicates where paint will appear
const surfaceGeo = new THREE.SphereGeometry(2); // The surface on which the paint will appear
const material = new THREE.MeshLambertMaterial({
  color: 0x00ff00,
});
const material2 = new THREE.MeshLambertMaterial({
  color: 0xffffff,
  opacity: 0.5,
  transparent: true,
});

// Create objects
const cursor = new THREE.Mesh(cursorGeo, material); // The cursor that indicates where paint will appear
const surface = new THREE.Mesh(surfaceGeo, material2); // The surface on which the paint will appear
const axesN = new THREE.AxesHelper(5); // Axes to help orient us in the inertial reference frame
const axesB = new THREE.AxesHelper(1); // Axes to show the cursor's reference frame
scene.add(surface);
surface.add(axesN);
//scene.add(cursor);
//cursor.add(axesB);
//cursor.position.x = 2;

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

// Here is where the idle animation occurs
let theta = 0;
let phi = 0;

function animate() {
  requestAnimationFrame(animate);
  phi += 0.009;
  setAngleInertial(surface, theta, phi);
  renderer.render(scene, camera);
}
animate();
