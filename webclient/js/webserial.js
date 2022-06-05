let port;
let keepReading = true;
let reader;
let closedPromise;
const bConn = document.getElementById("bConn");
const bDisconn = document.getElementById("bDisconn");

const streamReader = new TextDecoder();
let stream = "";

const idle = async (time) => {
  await new Promise((p) => setTimeout(p, time));
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
        // value is a Uint8Array.
        //console.log(value);
        //console.log(blah.decode(value));
        let newInput = streamReader.decode(value);
        for (let glyph of newInput) {
          stream += glyph;
          if (glyph == "\n" || glyph == "\0") {
            console.log(stream);
            if (stream.startsWith("^")) {
              stream.substring(1);
              orient(...stream.substring(1).split(","));
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
