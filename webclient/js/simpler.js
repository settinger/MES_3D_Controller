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

const material = new THREE.LineBasicMaterial({ color: 0x0000ff });

const renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
document.body.appendChild(renderer.domElement);

const NFrame = new THREE.AxesHelper(5);
scene.add(NFrame);

camera.position.z = 5;
camera.position.y = 2;
camera.position.x = 2;
camera.lookAt(N0);

renderer.render(scene, camera);

const orient = (θ, φ) => {
  const points = [];
  points.push(new THREE.Vector3(0, 0, 0));
  points.push(new THREE.Vector3());

  setAngleInertial2(cube, (θ * Math.PI) / 180, (φ * Math.PI) / 180);
  renderer.render(scene, camera);
};
