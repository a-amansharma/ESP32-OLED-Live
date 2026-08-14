#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define W 128
#define H 64
#define SDA_PIN 21
#define SCL_PIN 22
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(W, H, &Wire, -1);
WebServer server(80);

const char* SSID = "ESP32-OLED";
const char* PASS = "12345678";

int textSize = 1;
bool pixels[H][W];

void clearPixels() {
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      pixels[y][x] = 0;
    }
  }
}

void showText(String s) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(textSize);

  int charWidth = 6 * textSize;
  int lineHeight = 8 * textSize;
  int maxChars = max(1, W / charWidth);
  int y = 0;

  String line = "";

  for (int i = 0; i < s.length(); i++) {
    char c = s[i];

    if (c == '\n') {
      if (y + lineHeight <= H) {
        display.setCursor(0, y);
        display.print(line);
      }

      line = "";
      y += lineHeight;

      if (y >= H) {
        break;
      }

      continue;
    }

    line += c;

    if (line.length() >= maxChars) {
      if (y + lineHeight <= H) {
        display.setCursor(0, y);
        display.print(line);
      }

      line = "";
      y += lineHeight;

      if (y >= H) {
        break;
      }
    }
  }

  if (line.length() && y + lineHeight <= H) {
    display.setCursor(0, y);
    display.print(line);
  }

  display.display();
}

void showDraw() {
  display.clearDisplay();

  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      if (pixels[y][x]) {
        display.drawPixel(x, y, SSD1306_WHITE);
      }
    }
  }

  display.display();
}

int b64(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '+') return 62;
  if (c == '/') return 63;

  return -1;
}

bool decode(String s) {
  uint8_t data[1024];
  int count = 0;
  int value = 0;
  int bits = -8;

  for (char c : s) {
    if (c == '=') {
      break;
    }

    int q = b64(c);

    if (q < 0) {
      continue;
    }

    value = (value << 6) | q;
    bits += 6;

    if (bits >= 0) {
      if (count < 1024) {
        data[count++] = (value >> bits) & 255;
      }

      bits -= 8;
    }
  }

  if (count < 1024) {
    return false;
  }

  clearPixels();

  int index = 0;

  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      pixels[y][x] =
        (data[index] >> (7 - (x % 8))) & 1;

      if (x % 8 == 7) {
        index++;
      }
    }
  }

  return true;
}

const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
<meta name="viewport"
      content="width=device-width,initial-scale=1,user-scalable=no">
<title>ESP32 OLED Studio</title>
<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  background: #08090d;
  color: #fff;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
}

.app {
  max-width: 850px;
  margin: auto;
  padding: 22px;
}

.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.brand {
  display: flex;
  gap: 11px;
  align-items: center;
}

.logo {
  width: 42px;
  height: 42px;
  border-radius: 12px;
  background: #fff;
  color: #08090d;
  display: grid;
  place-items: center;
}

.logo svg {
  width: 24px;
}

.title {
  font-size: 18px;
  font-weight: 700;
}

.sub {
  font-size: 12px;
  color: #777b86;
  margin-top: 3px;
}

.status {
  font-size: 12px;
  color: #9a9da7;
}

.dot {
  display: inline-block;
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: #36d399;
  box-shadow: 0 0 9px #36d399;
  margin-right: 6px;
}

.switch {
  display: grid;
  grid-template-columns: 1fr 1fr;
  background: #12141a;
  border: 1px solid #232630;
  padding: 5px;
  border-radius: 15px;
  margin-bottom: 18px;
}

.mode {
  height: 48px;
  border: 0;
  border-radius: 11px;
  background: 0;
  color: #777b86;
  font-size: 14px;
  font-weight: 600;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
}

.mode svg {
  width: 19px;
}

.mode.active {
  background: #fff;
  color: #08090d;
}

.card {
  background: #111319;
  border: 1px solid #242731;
  border-radius: 20px;
  padding: 18px;
}

.panel {
  display: none;
}

.panel.active {
  display: block;
}

.row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
}

.label {
  font-size: 13px;
  font-weight: 600;
  color: #c7c9d1;
}

.counter {
  font-size: 11px;
  color: #686c77;
}

textarea {
  width: 100%;
  min-height: 190px;
  resize: vertical;
  background: #08090d;
  border: 1px solid #292c36;
  border-radius: 14px;
  color: #fff;
  padding: 16px;
  font-size: 17px;
  outline: 0;
}

textarea:focus {
  border-color: #666b78;
}

.font {
  margin-top: 16px;
  padding: 13px 14px;
  background: #08090d;
  border: 1px solid #292c36;
  border-radius: 13px;
}

.fonttop {
  display: flex;
  justify-content: space-between;
  margin-bottom: 12px;
}

.fv {
  background: #20232b;
  padding: 4px 7px;
  border-radius: 6px;
  font-size: 11px;
}

.slider {
  display: flex;
  align-items: center;
  gap: 10px;
}

.slider input {
  flex: 1;
  accent-color: #fff;
}

.small {
  font-size: 12px;
  color: #777b86;
}

.large {
  font-size: 20px;
}

.live {
  margin-top: 12px;
  color: #777b86;
  font-size: 12px;
}

.drawpreview {
  margin-bottom: 15px;
}

.previewtitle {
  font-size: 12px;
  color: #727681;
  margin: 0 0 10px;
}

.preview {
  width: 100%;
  max-width: 256px;
  aspect-ratio: 2 / 1;
  background: #000;
  border: 1px solid #282b34;
  border-radius: 7px;
  margin: auto;
  overflow: hidden;
}

.preview canvas {
  width: 100%;
  height: 100%;
  image-rendering: pixelated;
}

.canvaswrap {
  background: #08090d;
  border: 1px solid #292c36;
  border-radius: 15px;
  padding: 12px;
}

.canvasinner {
  width: 100%;
  aspect-ratio: 2 / 1;
  background: #000;
}

.canvasinner canvas {
  width: 100%;
  height: 100%;
  display: block;
  touch-action: none;
  cursor: crosshair;
}

.info {
  display: flex;
  justify-content: space-between;
  margin-top: 10px;
  color: #686c77;
  font-size: 11px;
}

.tools {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
  margin-top: 13px;
}

.btn {
  height: 45px;
  border: 1px solid #2b2e38;
  border-radius: 12px;
  background: #17191f;
  color: #d9dbe1;
  font-weight: 600;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
}

.btn svg {
  width: 17px;
}

@media (max-width: 600px) {
  .app {
    padding: 15px;
  }

  .card {
    padding: 13px;
  }
}
</style>
</head>
<body>
<div class="app">
  <div class="header">
    <div class="brand">
      <div class="logo">
        <svg viewBox="0 0 24 24"
             fill="none"
             stroke="currentColor"
             stroke-linecap="round"
             stroke-linejoin="round">
          <rect x="3" y="5" width="18" height="14" rx="2"/>
          <path d="M7 9h2M7 12h10M7 15h6"/>
        </svg>
      </div>
      <div>
        <div class="title">ESP32 OLED Studio</div>
        <div class="sub">Live OLED controller</div>
      </div>
    </div>
    <div class="status">
      <span class="dot"></span>Live
    </div>
  </div>

  <div class="switch">
    <button id="tb"
            class="mode active"
            onclick="mode('t')">
      <svg viewBox="0 0 24 24"
           fill="none"
           stroke="currentColor"
           stroke-linecap="round">
        <path d="M5 5h14M12 5v14M8 19h8"/>
      </svg>
      Type
    </button>

    <button id="db"
            class="mode"
            onclick="mode('d')">
      <svg viewBox="0 0 24 24"
           fill="none"
           stroke="currentColor"
           stroke-linecap="round"
           stroke-linejoin="round">
        <path d="M4 20l4.5-1 10.7-10.7a2.1 2.1 0 0 0-3-3L5.5 16z"/>
        <path d="M14.5 6.5l3 3"/>
      </svg>
      Draw
    </button>
  </div>

  <div class="card">
    <div id="tp" class="panel active">
      <div class="row">
        <span class="label">Type on the OLED</span>
        <span id="count" class="counter">0 characters</span>
      </div>

      <textarea id="text"
                maxlength="250"
                placeholder="Start typing..."></textarea>

      <div class="font">
        <div class="fonttop">
          <span class="label">Font size</span>
          <span id="fv" class="fv">1×</span>
        </div>

        <div class="slider">
          <span class="small">A</span>
          <input id="fs"
                 type="range"
                 min="1"
                 max="4"
                 value="1">
          <span class="large">A</span>
        </div>
      </div>

      <div class="live">
        <span class="dot"></span>
        Live — typing and size update OLED instantly
      </div>
    </div>

    <div id="dp" class="panel">
      <div class="drawpreview">
        <div class="previewtitle">OLED preview</div>

        <div class="preview">
          <canvas id="pv"
                  width="128"
                  height="64"></canvas>
        </div>
      </div>

      <div class="row">
        <span class="label">Draw on the OLED</span>
        <span class="counter">128 × 64</span>
      </div>

      <div class="canvaswrap">
        <div class="canvasinner">
          <canvas id="cv"
                  width="128"
                  height="64"></canvas>
        </div>
      </div>

      <div class="info">
        <span>Draw with mouse or finger</span>
        <span>LIVE</span>
      </div>

      <div class="tools">
        <button class="btn" onclick="undo()">
          <svg viewBox="0 0 24 24"
               fill="none"
               stroke="currentColor">
            <path d="M9 7 4 12l5 5M4 12h10a6 6 0 0 1 0 12"/>
          </svg>
          Undo
        </button>

        <button class="btn" onclick="clearCanvas()">
          <svg viewBox="0 0 24 24"
               fill="none"
               stroke="currentColor">
            <path d="M4 7h16M10 11v6M14 11v6M6 7l1 14h10l1-14M9 7V4h6v3"/>
          </svg>
          Clear
        </button>
      </div>
    </div>
  </div>
</div>

<script>
const textInput = document.getElementById("text");
const fontSlider = document.getElementById("fs");
const fontValue = document.getElementById("fv");
const counter = document.getElementById("count");

const canvas = document.getElementById("cv");
const ctx = canvas.getContext("2d");

const preview = document.getElementById("pv");
const previewCtx = preview.getContext("2d");

let drawing = false;
let strokes = [];
let currentStroke = [];
let drawTimer = null;
let drawPending = false;

ctx.fillStyle = "#000";
ctx.fillRect(0, 0, 128, 64);
ctx.strokeStyle = "#fff";
ctx.lineWidth = 1.2;
ctx.lineCap = "round";
ctx.lineJoin = "round";

function mode(value) {
  const typePanel = document.getElementById("tp");
  const drawPanel = document.getElementById("dp");
  const typeButton = document.getElementById("tb");
  const drawButton = document.getElementById("db");

  typePanel.classList.toggle("active", value === "t");
  drawPanel.classList.toggle("active", value === "d");

  typeButton.classList.toggle("active", value === "t");
  drawButton.classList.toggle("active", value === "d");

  if (value === "d") {
    previewDraw();
  }
}

textInput.addEventListener("input", () => {
  counter.textContent = `${textInput.value.length} characters`;
  sendText();
});

fontSlider.addEventListener("input", () => {
  fontValue.textContent = `${fontSlider.value}×`;
  sendText();
});

function sendText() {
  const value = textInput.value;
  const size = Number(fontSlider.value);

  fetch("/text", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded"
    },
    body:
      `text=${encodeURIComponent(value)}&size=${size}`
  });

  previewText(value, size);
}

function getPoint(event) {
  const rect = canvas.getBoundingClientRect();
  const source = event.touches?.[0] || event;

  return {
    x: Math.max(
      0,
      Math.min(
        127,
        Math.round(
          (source.clientX - rect.left) * 128 / rect.width
        )
      )
    ),
    y: Math.max(
      0,
      Math.min(
        63,
        Math.round(
          (source.clientY - rect.top) * 64 / rect.height
        )
      )
    )
  };
}

function startDrawing(event) {
  event.preventDefault();
  drawing = true;

  const point = getPoint(event);

  currentStroke = [point];

  ctx.fillStyle = "#fff";
  ctx.fillRect(point.x, point.y, 1, 1);

  previewDraw();
  sendDraw();
}

function moveDrawing(event) {
  if (!drawing) {
    return;
  }

  event.preventDefault();

  const point = getPoint(event);
  const lastPoint =
    currentStroke[currentStroke.length - 1];

  currentStroke.push(point);

  ctx.beginPath();
  ctx.moveTo(lastPoint.x, lastPoint.y);
  ctx.lineTo(point.x, point.y);
  ctx.stroke();

  previewDraw();
  sendDraw();
}

function stopDrawing(event) {
  if (!drawing) {
    return;
  }

  event?.preventDefault();
  drawing = false;

  if (currentStroke.length) {
    strokes.push(currentStroke);
  }

  currentStroke = [];

  previewDraw();
  sendDraw();
}

canvas.addEventListener("mousedown", startDrawing);
canvas.addEventListener("mousemove", moveDrawing);
canvas.addEventListener("mouseup", stopDrawing);
canvas.addEventListener("mouseleave", stopDrawing);

canvas.addEventListener(
  "touchstart",
  startDrawing,
  { passive: false }
);

canvas.addEventListener(
  "touchmove",
  moveDrawing,
  { passive: false }
);

canvas.addEventListener(
  "touchend",
  stopDrawing,
  { passive: false }
);

function redraw() {
  ctx.fillStyle = "#000";
  ctx.fillRect(0, 0, 128, 64);

  ctx.strokeStyle = "#fff";
  ctx.lineWidth = 1.2;
  ctx.lineCap = "round";
  ctx.lineJoin = "round";

  for (const stroke of strokes) {
    if (!stroke.length) {
      continue;
    }

    if (stroke.length === 1) {
      ctx.fillStyle = "#fff";
      ctx.fillRect(
        stroke[0].x,
        stroke[0].y,
        1,
        1
      );
      continue;
    }

    ctx.beginPath();
    ctx.moveTo(stroke[0].x, stroke[0].y);

    for (let i = 1; i < stroke.length; i++) {
      ctx.lineTo(
        stroke[i].x,
        stroke[i].y
      );
    }

    ctx.stroke();
  }

  previewDraw();
  sendDraw();
}

function undo() {
  if (!strokes.length) {
    return;
  }

  strokes.pop();
  redraw();
}

function clearCanvas() {
  strokes = [];
  currentStroke = [];

  ctx.fillStyle = "#000";
  ctx.fillRect(0, 0, 128, 64);

  previewDraw();
  sendDraw();
}

function canvasData() {
  const image =
    ctx.getImageData(0, 0, 128, 64).data;

  const bytes = [];
  let value = 0;
  let bits = 0;

  for (let y = 0; y < 64; y++) {
    for (let x = 0; x < 128; x++) {
      const index =
        (y * 128 + x) * 4;

      const brightness =
        image[index] +
        image[index + 1] +
        image[index + 2];

      value =
        (value << 1) |
        (brightness > 200 ? 1 : 0);

      bits++;

      if (bits === 8) {
        bytes.push(value);
        value = 0;
        bits = 0;
      }
    }
  }

  let binary = "";

  for (const byte of bytes) {
    binary += String.fromCharCode(byte);
  }

  return btoa(binary);
}

function sendDraw() {
  drawPending = true;

  if (drawTimer) {
    return;
  }

  drawTimer = setTimeout(() => {
    drawTimer = null;

    if (!drawPending) {
      return;
    }

    drawPending = false;

    fetch("/draw", {
      method: "POST",
      headers: {
        "Content-Type":
          "application/x-www-form-urlencoded"
      },
      body:
        `data=${encodeURIComponent(canvasData())}`
    });
  }, 35);
}

function previewDraw() {
  previewCtx.clearRect(0, 0, 128, 64);
  previewCtx.drawImage(canvas, 0, 0);
}

function previewText(value, size) {
  previewCtx.fillStyle = "#000";
  previewCtx.fillRect(0, 0, 128, 64);

  previewCtx.fillStyle = "#fff";

  const charWidth = 6 * size;
  const lineHeight = 8 * size;
  const maxChars =
    Math.max(
      1,
      Math.floor(128 / charWidth)
    );

  let line = "";
  let y = 0;

  previewCtx.font =
    `${8 * size}px monospace`;

  previewCtx.textBaseline = "top";

  for (let i = 0; i < value.length; i++) {
    const character = value[i];

    if (character === "\n") {
      if (y + lineHeight <= 64) {
        previewCtx.fillText(
          line,
          0,
          y
        );
      }

      line = "";
      y += lineHeight;

      if (y >= 64) {
        break;
      }

      continue;
    }

    line += character;

    if (line.length >= maxChars) {
      if (y + lineHeight <= 64) {
        previewCtx.fillText(
          line,
          0,
          y
        );
      }

      line = "";
      y += lineHeight;

      if (y >= 64) {
        break;
      }
    }
  }

  if (line && y + lineHeight <= 64) {
    previewCtx.fillText(
      line,
      0,
      y
    );
  }
}

previewDraw();
</script>
</body>
</html>
)HTML";

void root() {
  server.send_P(200, "text/html", PAGE);
}

void textHandler() {
  if (!server.hasArg("text")) {
    server.send(
      400,
      "text/plain",
      "Missing text"
    );
    return;
  }

  if (server.hasArg("size")) {
    textSize = constrain(
      server.arg("size").toInt(),
      1,
      4
    );
  }

  showText(server.arg("text"));

  server.send(
    200,
    "text/plain",
    "OK"
  );
}

void drawHandler() {
  if (!server.hasArg("data")) {
    server.send(
      400,
      "text/plain",
      "Missing data"
    );
    return;
  }

  if (!decode(server.arg("data"))) {
    server.send(
      400,
      "text/plain",
      "Invalid drawing"
    );
    return;
  }

  showDraw();

  server.send(
    200,
    "text/plain",
    "OK"
  );
}

void setup() {
  Serial.begin(115200);

  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDR
      )) {
    while (1) {
      delay(1000);
    }
  }

  clearPixels();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(
    SSID,
    PASS
  );

  server.on(
    "/",
    HTTP_GET,
    root
  );

  server.on(
    "/text",
    HTTP_POST,
    textHandler
  );

  server.on(
    "/draw",
    HTTP_POST,
    drawHandler
  );

  server.begin();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("WiFi Ready");
  display.println(SSID);
  display.println("192.168.4.1");
  display.display();

  Serial.println(
    WiFi.softAPIP()
  );
}
void loop() {
  server.handleClient();
}