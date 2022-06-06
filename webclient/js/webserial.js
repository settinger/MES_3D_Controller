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
  } else if (args[0] == "d") {
    // A draw-spot command was received
    if (args.length != 3) return;
    let θ = parseFloat(args[1]);
    let φ = parseFloat(args[2]);
    if (θ < -90 || θ > 90 || φ < -180 || φ > 180) return;
    symmetricSplotch(θ, φ);
  } else if (args[0] == "x") {
    // A move-and-draw command was received
    if (args.length != 3) return;
    let θ = parseFloat(args[1]);
    let φ = parseFloat(args[2]);
    if (θ < -90 || θ > 90 || φ < -180 || φ > 180) return;
    orient(θ, φ);
    symmetricSplotch(θ, φ);
  } else if (args[0] == "c") {
    // A change-color command was received
    if (args.length != 2 || args[1].length != 6) return;
    // TODO: SET COLOR OF CURSOR AND ART HERE
  } else if (args[0] == "r") {
    // A resize-cursor command was received
    if (args.length != 2) return;
    let size = parseInt(args[1]);
    if (!!size || size < 0 || size > 200) return;
    // TODO: SET CURSOR SIZE HERE
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
            //console.log(stream);
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
});

bDisconn.addEventListener("click", async () => {
  keepReading = false;
  reader.cancel();
  await closedPromise;
});
