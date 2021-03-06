const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(
  75,
  window.innerWidth / window.innerHeight,
  0.1,
  1000
);
// Axes in Frame N (inertial reference frame)
const Nx = new THREE.Vector3(1, 0, 0);
const Ny = new THREE.Vector3(0, 1, 0);
const Nz = new THREE.Vector3(0, 0, 1);
const N0 = new THREE.Vector3(0, 0, 0);
const origPosition = new THREE.Vector3(0, 2, 0);
const origRotation = new THREE.Euler(0, 0, 0);

const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

const geometry = new THREE.BoxGeometry(1, 1, 1);
const material = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
const cube = new THREE.Mesh(geometry, material);
cube.position.set(...origPosition);
cube.rotation.set(...origRotation);

scene.add(cube);
const NFrame = new THREE.AxesHelper(5);
scene.add(NFrame);
const cubeFrame = new THREE.AxesHelper(2);
cube.add(cubeFrame);

camera.position.z = 5;
camera.position.y = 2;
camera.position.x = 2;
camera.lookAt(N0);

renderer.render(scene, camera);

// I can't believe "rotateOnWorldAxis" just doesn't work and people accept that
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

  // Set object rotation to new values
  const eul = new THREE.Euler(phi, 0, theta, "XYZ");
  object.setRotationFromEuler(eul);

  // Set object position to new values
  object.position.sub(point);
  object.position.applyEuler(eul);
  object.position.add(point);
};

const orient = (θ, φ) => {
  setAngleInertial(cube, (θ * Math.PI) / 180, (φ * Math.PI) / 180);
  renderer.render(scene, camera);
};
