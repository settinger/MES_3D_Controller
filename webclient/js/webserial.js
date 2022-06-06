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

const isValidCommand = (stream) => {
  if (!stream.startsWith("^")) {
    return false;
  }
  let args = stream.substring(1).split(",");
  if (args.length != 2) {
    return false;
  }
  let θ = parseFloat(args[0]);
  let φ = parseFloat(args[1]);

  if (θ < -90 || θ > 90 || φ < -180 || φ > 180) {
    return false;
  }
  return true;
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
            if (isValidCommand(stream)) {
              let args = stream.substring(1).split(",");
              let θ = parseFloat(args[0]);
              let φ = parseFloat(args[1]);
              orient(θ, φ);
              counter++;
              if (counter >= 4) {
                symmetricSplotch(θ, φ);
                counter = 0;
              }
            }
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
