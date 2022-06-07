let port;
let keepReading = true;
let reader;
let closedPromise;
const bConn = document.getElementById("bConn");
const bDisconn = document.getElementById("bDisconn");

const streamReader = new TextDecoder();
let stream = "";
let counter = 0;

const idle = async (time) => {
  await new Promise((p) => setTimeout(p, time));
};

// This is the function that executes a screen update
const commandProcess = (stream) => {
  let args = stream.split(","); // Don't forget all arguments are strings at this point!
  if (args[0] == "m") {
    // A move-cursor command was received
    if (args.length != 3) return;
    let θ = parseFloat(args[1]);
    let φ = parseFloat(args[2]);
    if (θ < -90 || θ > 90 || φ < -180 || φ > 180) return;
    orient(θ, φ);
    counter = 0;
  } else if (args[0] == "d") {
    // A draw-spot command was received
    if (args.length != 3) return;
    let θ = parseFloat(args[1]);
    let φ = parseFloat(args[2]);
    if (θ < -90 || θ > 90 || φ < -180 || φ > 180) return;
    symmetricSplotch(θ, φ);
    counter = 0;
  } else if (args[0] == "x") {
    // A move-and-draw command was received
    if (args.length != 3) return;
    let θ = parseFloat(args[1]);
    let φ = parseFloat(args[2]);
    if (θ < -90 || θ > 90 || φ < -180 || φ > 180) return;
    symmetricSplotch(θ, φ);
    orient(θ, φ);
  } else if (args[0] == "c") {
    // A change-color command was received
    if (args.length != 2) return;
    cursorColor = args[1].trim();
    cursor.material.color.setHex(`0x${cursorColor}`);
  } else if (args[0] == "r") {
    // A resize-cursor command was received
    if (args.length != 2) return;
    let size = parseInt(args[1]);
    if (!size || size < 0 || size > 120) return;
    // Unfortunately the only way I know to change the cursor radius is with scale()
    cursorGeo.scale(size / cursorSize, size / cursorSize, size / cursorSize);
    cursorSize = size;
  }
};

async function readUntilClosed() {
  while (port.readable && keepReading) {
    reader = port.readable.getReader();
    try {
      while (true) {
        const { value, done } = await reader.read();
        if (done) {
          // reader.cancel() has been called.
          break;
        }
        let newInput = streamReader.decode(value);
        for (let glyph of newInput) {
          stream += glyph;
          if (glyph == "\n" || glyph == "\0") {
            console.log(stream);
            commandProcess(stream);
            stream = "";
          }
        }
      }
    } catch (error) {
      // Handle error...
    } finally {
      // Allow the serial port to be closed later.
      reader.releaseLock();
    }
  }
  await port.close();
}

bConn.addEventListener("click", async () => {
  port = await navigator.serial.requestPort();
  await port.open({ baudRate: 115200 });
  closedPromise = readUntilClosed();
  bDisconn.style.display = "block";
  bConn.style.display = "none";
});

bDisconn.addEventListener("click", async () => {
  keepReading = false;
  reader.cancel();
  await closedPromise;
  bDisconn.style.display = "none";
  bConn.style.display = "block";
  bConn.disabled = true;
});
