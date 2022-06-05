const scene = new THREE.Scene();

// Axes in Frame N (inertial reference frame)
const Nx = new THREE.Vector3(1, 0, 0);
const Ny = new THREE.Vector3(0, 1, 0);
const Nz = new THREE.Vector3(0, 0, 1);
const N0 = new THREE.Vector3(0, 0, 0);
const origPosition = new THREE.Vector3(0, 2, 0);
const origRotation = new THREE.Euler(0, 0, 0);

const camera = new THREE.PerspectiveCamera(
  75,
  window.innerWidth / window.innerHeight,
  0.1,
  1000
);

const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

const geometry = new THREE.BoxGeometry(1, 1, 1);
const material = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
const material2 = new THREE.MeshBasicMaterial({ color: 0xff0000 });
const cube = new THREE.Mesh(geometry, material);
scene.add(cube);
const axesHelper = new THREE.AxesHelper(5);
scene.add(axesN);

camera.position.z = 5;
camera.position.y = 2;
camera.position.x = 2;
camera.lookAt(N0);
cube.position.x = 2;

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

  // Set cube rotation to new values
  const eul = new THREE.Euler(phi, 0, theta, "XYZ");
  object.setRotationFromEuler(eul);

  // Set cube position to new values
  object.position.sub(point);
  object.position.applyEuler(eul);
  object.position.add(point);
};

let theta = 0;
let phi = 0;

function animate() {
  requestAnimationFrame(animate);
  theta += 0.009;
  phi += 0.01;
  setAngleInertial(cube, theta, phi);
  renderer.render(scene, camera);
}
animate();
