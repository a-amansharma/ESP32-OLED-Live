#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================
// ESP32 OLED LIVE STUDIO
// 0.96" SSD1306 OLED - 128x64
// ============================================================

// ---------------- OLED ----------------

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ---------------- WEB SERVER ----------------

WebServer server(80);

// ---------------- WIFI ----------------

const char* AP_SSID = "ESP32-OLED";
const char* AP_PASSWORD = "12345678";

// ---------------- TEXT ----------------

int currentTextSize = 1;

// ---------------- DRAWING ----------------

bool drawingBuffer[64][128];

// ============================================================
// CLEAR DRAWING BUFFER
// ============================================================

void clearDrawingBuffer() {

  for (int y = 0; y < 64; y++) {

    for (int x = 0; x < 128; x++) {

      drawingBuffer[y][x] = false;

    }

  }

}

// ============================================================
// DISPLAY TEXT
// ============================================================

void displayText(String text) {

  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(
    currentTextSize
  );


  int charWidth =
    6 * currentTextSize;

  int lineHeight =
    8 * currentTextSize;


  int maxChars =
    SCREEN_WIDTH / charWidth;


  if (maxChars < 1) {
    maxChars = 1;
  }


  String line = "";

  int y = 0;


  for (
    int i = 0;
    i < text.length();
    i++
  ) {

    char c = text[i];


    // --------------------------------------------------------
    // MANUAL LINE BREAK
    // --------------------------------------------------------

    if (c == '\n') {

      if (
        y + lineHeight <=
        SCREEN_HEIGHT
      ) {

        display.setCursor(
          0,
          y
        );

        display.print(
          line
        );

      }


      line = "";

      y += lineHeight;


      if (
        y >= SCREEN_HEIGHT
      ) {

        break;

      }


      continue;

    }


    // --------------------------------------------------------
    // ADD CHARACTER
    // --------------------------------------------------------

    line += c;


    // --------------------------------------------------------
    // AUTOMATIC WRAP
    // --------------------------------------------------------

    if (
      line.length() >=
      maxChars
    ) {

      if (
        y + lineHeight <=
        SCREEN_HEIGHT
      ) {

        display.setCursor(
          0,
          y
        );

        display.print(
          line
        );

      }


      line = "";

      y += lineHeight;


      if (
        y >= SCREEN_HEIGHT
      ) {

        break;

      }

    }

  }


  // ----------------------------------------------------------
  // LAST LINE
  // ----------------------------------------------------------

  if (
    line.length() > 0 &&
    y + lineHeight <=
    SCREEN_HEIGHT
  ) {

    display.setCursor(
      0,
      y
    );

    display.print(
      line
    );

  }


  display.display();

}

// ============================================================
// DISPLAY DRAWING
// ============================================================

void displayDrawing() {

  display.clearDisplay();


  for (
    int y = 0;
    y < 64;
    y++
  ) {

    for (
      int x = 0;
      x < 128;
      x++
    ) {

      if (
        drawingBuffer[y][x]
      ) {

        display.drawPixel(
          x,
          y,
          SSD1306_WHITE
        );

      }

    }

  }


  display.display();

}

// ============================================================
// BASE64 VALUE
// ============================================================

int base64Value(
  char c
) {

  if (
    c >= 'A' &&
    c <= 'Z'
  ) {

    return c - 'A';

  }


  if (
    c >= 'a' &&
    c <= 'z'
  ) {

    return c - 'a' + 26;

  }


  if (
    c >= '0' &&
    c <= '9'
  ) {

    return c - '0' + 52;

  }


  if (c == '+') {

    return 62;

  }


  if (c == '/') {

    return 63;

  }


  return -1;

}

// ============================================================
// DECODE DRAWING
// ============================================================

bool decodeDrawing(
  String encoded
) {

  const int expectedBytes =
    1024;


  uint8_t decoded[
    expectedBytes
  ];


  int decodedLength = 0;

  int value = 0;

  int bits = -8;


  for (
    int i = 0;
    i < encoded.length();
    i++
  ) {

    char c =
      encoded[i];


    if (c == '=') {

      break;

    }


    int v =
      base64Value(c);


    if (v < 0) {

      continue;

    }


    value =
      (value << 6) | v;


    bits += 6;


    if (bits >= 0) {

      if (
        decodedLength <
        expectedBytes
      ) {

        decoded[
          decodedLength
        ] =
          (value >> bits) &
          0xFF;

        decodedLength++;

      }


      bits -= 8;

    }

  }


  if (
    decodedLength <
    expectedBytes
  ) {

    return false;

  }


  clearDrawingBuffer();


  int byteIndex = 0;


  for (
    int y = 0;
    y < 64;
    y++
  ) {

    for (
      int x = 0;
      x < 128;
      x++
    ) {

      int bit =
        7 - (x % 8);


      drawingBuffer[y][x] =
        (
          decoded[byteIndex]
          >> bit
        ) & 1;


      if (
        x % 8 == 7
      ) {

        byteIndex++;

      }

    }

  }


  return true;

}

// ============================================================
// HTML PAGE
// ============================================================

const char MAIN_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta charset="UTF-8">

<meta
  name="viewport"
  content="width=device-width,
           initial-scale=1,
           maximum-scale=1,
           user-scalable=no"
>

<title>ESP32 OLED Studio</title>

<style>

/* ==========================================================
   RESET
   ========================================================== */

* {

  box-sizing: border-box;

  -webkit-tap-highlight-color:
    transparent;

}


html,
body {

  margin: 0;

  padding: 0;

  width: 100%;

  min-height: 100%;

  background: #08090d;

  color: #ffffff;

  font-family:
    -apple-system,
    BlinkMacSystemFont,
    "Segoe UI",
    sans-serif;

}


body {

  display: flex;

  justify-content: center;

}


/* ==========================================================
   APP
   ========================================================== */

.app {

  width: 100%;

  max-width: 850px;

  min-height: 100vh;

  padding: 22px;

}


/* ==========================================================
   HEADER
   ========================================================== */

.header {

  display: flex;

  align-items: center;

  justify-content:
    space-between;

  margin-bottom: 22px;

}


.brand {

  display: flex;

  align-items: center;

  gap: 12px;

}


.logo {

  width: 42px;

  height: 42px;

  border-radius: 12px;

  background: #ffffff;

  color: #08090d;

  display: flex;

  align-items: center;

  justify-content: center;

}


.logo svg {

  width: 24px;

  height: 24px;

}


.title {

  font-size: 18px;

  font-weight: 700;

  letter-spacing:
    -0.3px;

}


.subtitle {

  font-size: 12px;

  color: #777b86;

  margin-top: 3px;

}


.status {

  display: flex;

  align-items: center;

  gap: 7px;

  font-size: 12px;

  color: #9a9da7;

}


.status-dot {

  width: 8px;

  height: 8px;

  border-radius: 50%;

  background: #36d399;

  box-shadow:
    0 0 10px
    rgba(54,211,153,.7);

}


/* ==========================================================
   MODE SWITCH
   ========================================================== */

.mode-switch {

  width: 100%;

  display: grid;

  grid-template-columns:
    1fr 1fr;

  background: #12141a;

  border:
    1px solid #232630;

  padding: 5px;

  border-radius: 15px;

  margin-bottom: 18px;

}


.mode-btn {

  height: 48px;

  border: 0;

  border-radius: 11px;

  background: transparent;

  color: #777b86;

  font-size: 14px;

  font-weight: 600;

  cursor: pointer;

  display: flex;

  align-items: center;

  justify-content: center;

  gap: 9px;

  transition:
    background .15s ease,
    color .15s ease;

}


.mode-btn svg {

  width: 19px;

  height: 19px;

  stroke-width: 1.9;

}


.mode-btn.active {

  background: #ffffff;

  color: #08090d;

}


/* ==========================================================
   CARD
   ========================================================== */

.card {

  background: #111319;

  border:
    1px solid #242731;

  border-radius: 20px;

  padding: 18px;

}


/* ==========================================================
   PANELS
   ========================================================== */

.panel {

  display: none;

}


.panel.active {

  display: block;

}


/* ==========================================================
   LABEL
   ========================================================== */

.label-row {

  display: flex;

  justify-content:
    space-between;

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


/* ==========================================================
   TEXT BOX
   ========================================================== */

textarea {

  width: 100%;

  min-height: 190px;

  resize: vertical;

  background: #08090d;

  border:
    1px solid #292c36;

  border-radius: 14px;

  color: #ffffff;

  padding: 16px;

  font-family:
    -apple-system,
    BlinkMacSystemFont,
    "Segoe UI",
    sans-serif;

  font-size: 17px;

  line-height: 1.5;

  outline: none;

}


textarea:focus {

  border-color: #666b78;

}


/* ==========================================================
   FONT CONTROL
   ========================================================== */

.font-control {

  margin-top: 16px;

  padding: 13px 14px;

  background: #08090d;

  border:
    1px solid #292c36;

  border-radius: 13px;

}


.font-control-top {

  display: flex;

  justify-content:
    space-between;

  align-items: center;

  margin-bottom: 12px;

}


.font-label {

  font-size: 12px;

  font-weight: 600;

  color: #c7c9d1;

}


.font-value {

  min-width: 32px;

  text-align: center;

  font-size: 11px;

  font-weight: 700;

  color: #ffffff;

  background: #20232b;

  padding: 4px 7px;

  border-radius: 6px;

}


.font-slider-row {

  display: flex;

  align-items: center;

  gap: 10px;

}


.font-small {

  font-size: 12px;

  color: #777b86;

}


.font-large {

  font-size: 20px;

  color: #ffffff;

}


#fontSlider {

  flex: 1;

  height: 4px;

  cursor: pointer;

  accent-color: #ffffff;

}


/* ==========================================================
   LIVE INFO
   ========================================================== */

.live-info {

  display: flex;

  align-items: center;

  gap: 7px;

  margin-top: 12px;

  color: #777b86;

  font-size: 12px;

}


.live-dot {

  width: 7px;

  height: 7px;

  border-radius: 50%;

  background: #36d399;

  box-shadow:
    0 0 7px
    rgba(54,211,153,.6);

}


/* ==========================================================
   DRAWING
   ========================================================== */

.canvas-wrapper {

  width: 100%;

  background: #08090d;

  border:
    1px solid #292c36;

  border-radius: 15px;

  padding: 12px;

  display: flex;

  justify-content: center;

}


.canvas-inner {

  width: 100%;

  max-width: 640px;

  aspect-ratio: 2 / 1;

  background: #000000;

  border-radius: 4px;

  overflow: hidden;

}


#drawCanvas {

  width: 100%;

  height: 100%;

  display: block;

  background: #000000;

  cursor: crosshair;

  touch-action: none;

}


.canvas-info {

  display: flex;

  justify-content:
    space-between;

  align-items: center;

  margin-top: 11px;

  color: #686c77;

  font-size: 11px;

}


/* ==========================================================
   DRAWING BUTTONS
   ========================================================== */

.tool-row {

  display: grid;

  grid-template-columns:
    1fr 1fr;

  gap: 10px;

  margin-top: 13px;

}


.secondary-btn {

  height: 45px;

  border:
    1px solid #2b2e38;

  border-radius: 12px;

  background: #17191f;

  color: #d9dbe1;

  font-size: 13px;

  font-weight: 600;

  cursor: pointer;

  display: flex;

  align-items: center;

  justify-content: center;

  gap: 8px;

}


.secondary-btn:active {

  transform: scale(.98);

}


.secondary-btn svg {

  width: 17px;

  height: 17px;

}


/* ==========================================================
   PREVIEW
   ========================================================== */

.preview-title {

  margin-top: 20px;

  margin-bottom: 10px;

  font-size: 12px;

  color: #727681;

}


.oled-preview {

  width: 100%;

  max-width: 256px;

  aspect-ratio: 2 / 1;

  background: #000000;

  border:
    1px solid #282b34;

  border-radius: 7px;

  margin: auto;

  overflow: hidden;

}


#previewCanvas {

  width: 100%;

  height: 100%;

  image-rendering: pixelated;

}


/* ==========================================================
   MOBILE
   ========================================================== */

@media(max-width:600px) {

  .app {

    padding: 15px;

  }


  .card {

    padding: 13px;

    border-radius: 17px;

  }


  .title {

    font-size: 16px;

  }


  textarea {

    min-height: 180px;

  }

}

</style>

</head>


<body>


<div class="app">


<!-- ========================================================
     HEADER
     ======================================================== -->

<div class="header">


  <div class="brand">


    <div class="logo">

      <!-- OLED ICON -->

      <svg
        viewBox="0 0 24 24"
        fill="none"
        stroke="currentColor"
        stroke-linecap="round"
        stroke-linejoin="round"
      >

        <rect
          x="3"
          y="5"
          width="18"
          height="14"
          rx="2"
        />

        <path d="M7 9h2"/>

        <path d="M7 12h10"/>

        <path d="M7 15h6"/>

      </svg>

    </div>


    <div>

      <div class="title">
        ESP32 OLED Studio
      </div>

      <div class="subtitle">
        Live OLED controller
      </div>

    </div>


  </div>


  <div class="status">

    <span class="status-dot"></span>

    Live

  </div>


</div>


<!-- ========================================================
     MODE SWITCH
     ======================================================== -->

<div class="mode-switch">


  <!-- TYPE BUTTON -->

  <button
    id="typeModeBtn"
    class="mode-btn active"
    onclick="switchMode('type')"
  >

    <!-- TYPE ICON -->

    <svg
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      stroke-linecap="round"
      stroke-linejoin="round"
    >

      <path d="M5 5h14"/>

      <path d="M12 5v14"/>

      <path d="M8 19h8"/>

    </svg>

    Type

  </button>


  <!-- DRAW BUTTON -->

  <button
    id="drawModeBtn"
    class="mode-btn"
    onclick="switchMode('draw')"
  >

    <!-- PEN ICON -->

    <svg
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      stroke-linecap="round"
      stroke-linejoin="round"
    >

      <path
        d="M4 20l4.5-1
           10.7-10.7
           a2.1 2.1 0 0 0-3-3
           L5.5 16z"
      />

      <path d="M14.5 6.5l3 3"/>

      <path d="M4 20l1-4.5"/>

    </svg>

    Draw

  </button>


</div>


<!-- ========================================================
     MAIN CARD
     ======================================================== -->

<div class="card">


<!-- ========================================================
     TYPE PANEL
     ======================================================== -->

<div
  id="typePanel"
  class="panel active"
>


  <div class="label-row">


    <div class="label">
      Type on the OLED
    </div>


    <div
      id="charCounter"
      class="counter"
    >
      0 characters
    </div>


  </div>


  <textarea
    id="textInput"
    maxlength="250"
    placeholder="Start typing..."
  ></textarea>


  <!-- FONT SIZE -->

  <div class="font-control">


    <div class="font-control-top">


      <span class="font-label">
        Font size
      </span>


      <span
        id="fontSizeValue"
        class="font-value"
      >
        1×
      </span>


    </div>


    <div class="font-slider-row">


      <span class="font-small">
        A
      </span>


      <input
        id="fontSlider"
        type="range"
        min="1"
        max="4"
        step="1"
        value="1"
      >


      <span class="font-large">
        A
      </span>


    </div>


  </div>


  <div class="live-info">


    <span class="live-dot"></span>


    Live — typing and size update OLED instantly


  </div>


</div>


<!-- ========================================================
     DRAW PANEL
     ======================================================== -->

<div
  id="drawPanel"
  class="panel"
>


  <div class="label-row">


    <div class="label">
      Draw on the OLED
    </div>


    <div class="counter">
      128 × 64
    </div>


  </div>


  <div class="canvas-wrapper">


    <div class="canvas-inner">


      <canvas
        id="drawCanvas"
        width="128"
        height="64"
      ></canvas>


    </div>


  </div>


  <div class="canvas-info">


    <span>
      Draw with mouse or finger
    </span>


    <span>
      LIVE
    </span>


  </div>


  <div class="tool-row">


    <!-- UNDO -->

    <button
      class="secondary-btn"
      onclick="undoStroke()"
    >


      <svg
        viewBox="0 0 24 24"
        fill="none"
        stroke="currentColor"
        stroke-linecap="round"
        stroke-linejoin="round"
      >

        <path
          d="M9 7L4 12l5 5"
        />

        <path
          d="M4 12h10
             a6 6 0 0 1
             0 12"
        />

      </svg>


      Undo


    </button>


    <!-- CLEAR -->

    <button
      class="secondary-btn"
      onclick="clearCanvas()"
    >


      <svg
        viewBox="0 0 24 24"
        fill="none"
        stroke="currentColor"
        stroke-linecap="round"
        stroke-linejoin="round"
      >

        <path d="M4 7h16"/>

        <path d="M10 11v6"/>

        <path d="M14 11v6"/>

        <path
          d="M6 7l1 14h10l1-14"
        />

        <path
          d="M9 7V4h6v3"
        />

      </svg>


      Clear


    </button>


  </div>


</div>


</div>


<!-- ========================================================
     OLED PREVIEW
     ======================================================== -->

<div class="preview-title">
  OLED preview
</div>


<div class="oled-preview">


  <canvas
    id="previewCanvas"
    width="128"
    height="64"
  ></canvas>


</div>


</div>


<script>

// ============================================================
// MODE SWITCH
// ============================================================

function switchMode(
  mode
) {

  const typePanel =
    document.getElementById(
      "typePanel"
    );


  const drawPanel =
    document.getElementById(
      "drawPanel"
    );


  const typeButton =
    document.getElementById(
      "typeModeBtn"
    );


  const drawButton =
    document.getElementById(
      "drawModeBtn"
    );


  if (
    mode === "type"
  ) {


    typePanel.classList.add(
      "active"
    );


    drawPanel.classList.remove(
      "active"
    );


    typeButton.classList.add(
      "active"
    );


    drawButton.classList.remove(
      "active"
    );


  } else {


    typePanel.classList.remove(
      "active"
    );


    drawPanel.classList.add(
      "active"
    );


    typeButton.classList.remove(
      "active"
    );


    drawButton.classList.add(
      "active"
    );


  }

}


// ============================================================
// TEXT CONTROLS
// ============================================================

const textInput =
  document.getElementById(
    "textInput"
  );


const charCounter =
  document.getElementById(
    "charCounter"
  );


const fontSlider =
  document.getElementById(
    "fontSlider"
  );


const fontSizeValue =
  document.getElementById(
    "fontSizeValue"
  );


// ============================================================
// TEXT INPUT LIVE
// ============================================================

textInput.addEventListener(
  "input",
  function() {

    charCounter.textContent =
      this.value.length +
      " characters";


    sendLiveText();

  }
);


// ============================================================
// FONT SLIDER LIVE
// ============================================================

fontSlider.addEventListener(
  "input",
  function() {

    const size =
      parseInt(
        this.value
      );


    fontSizeValue.textContent =
      size + "×";


    sendLiveText();

  }
);


// ============================================================
// SEND LIVE TEXT
// ============================================================

function sendLiveText() {

  const text =
    textInput.value;


  const size =
    parseInt(
      fontSlider.value
    );


  fetch(
    "/text",
    {

      method: "POST",

      headers: {

        "Content-Type":
          "application/x-www-form-urlencoded"

      },

      body:
        "text=" +
        encodeURIComponent(
          text
        ) +
        "&size=" +
        size

    }
  );


  updatePreviewText(
    text,
    size
  );

}


// ============================================================
// DRAWING
// ============================================================

const canvas =
  document.getElementById(
    "drawCanvas"
  );


const ctx =
  canvas.getContext(
    "2d"
  );


ctx.fillStyle =
  "#000000";


ctx.fillRect(
  0,
  0,
  128,
  64
);


ctx.strokeStyle =
  "#ffffff";


ctx.lineWidth =
  1.2;


ctx.lineCap =
  "round";


ctx.lineJoin =
  "round";


let drawing = false;


let strokes = [];


let currentStroke = [];


// ============================================================
// LIVE DRAW THROTTLE
// ============================================================

let liveDrawTimer = null;


let liveDrawPending = false;


function liveSendDrawing() {

  liveDrawPending =
    true;


  if (
    liveDrawTimer !== null
  ) {

    return;

  }


  liveDrawTimer =
    setTimeout(
      sendDrawingNow,
      35
    );

}


function sendDrawingNow() {

  liveDrawTimer =
    null;


  if (
    !liveDrawPending
  ) {

    return;

  }


  liveDrawPending =
    false;


  const base64 =
    canvasToBase64();


  fetch(
    "/draw",
    {

      method: "POST",

      headers: {

        "Content-Type":
          "application/x-www-form-urlencoded"

      },

      body:
        "data=" +
        encodeURIComponent(
          base64
        )

    }
  );


  if (
    liveDrawPending
  ) {

    liveSendDrawing();

  }

}


// ============================================================
// GET CANVAS POINT
// ============================================================

function getCanvasPoint(
  event
) {

  const rect =
    canvas.getBoundingClientRect();


  let clientX;

  let clientY;


  if (
    event.touches &&
    event.touches.length > 0
  ) {

    clientX =
      event.touches[0].clientX;


    clientY =
      event.touches[0].clientY;


  } else {


    clientX =
      event.clientX;


    clientY =
      event.clientY;

  }


  let x =
    (
      clientX -
      rect.left
    )
    * 128
    / rect.width;


  let y =
    (
      clientY -
      rect.top
    )
    * 64
    / rect.height;


  x =
    Math.max(
      0,
      Math.min(
        127,
        Math.round(x)
      )
    );


  y =
    Math.max(
      0,
      Math.min(
        63,
        Math.round(y)
      )
    );


  return {
    x: x,
    y: y
  };

}


// ============================================================
// START DRAWING
// ============================================================

function startDrawing(
  event
) {

  event.preventDefault();


  drawing = true;


  const point =
    getCanvasPoint(
      event
    );


  currentStroke = [
    point
  ];


  ctx.fillStyle =
    "#ffffff";


  ctx.fillRect(
    point.x,
    point.y,
    1,
    1
  );


  updatePreviewCanvas();


  liveSendDrawing();

}


// ============================================================
// DRAW
// ============================================================

function draw(
  event
) {

  if (!drawing) {

    return;

  }


  event.preventDefault();


  const point =
    getCanvasPoint(
      event
    );


  const previous =
    currentStroke[
      currentStroke.length - 1
    ];


  currentStroke.push(
    point
  );


  ctx.strokeStyle =
    "#ffffff";


  ctx.lineWidth =
    1.2;


  ctx.lineCap =
    "round";


  ctx.beginPath();


  ctx.moveTo(
    previous.x,
    previous.y
  );


  ctx.lineTo(
    point.x,
    point.y
  );


  ctx.stroke();


  updatePreviewCanvas();


  liveSendDrawing();

}


// ============================================================
// STOP DRAWING
// ============================================================

function stopDrawing(
  event
) {

  if (!drawing) {

    return;

  }


  if (event) {

    event.preventDefault();

  }


  drawing = false;


  if (
    currentStroke.length > 0
  ) {

    strokes.push(
      currentStroke
    );

  }


  currentStroke = [];


  updatePreviewCanvas();


  liveSendDrawing();

}


// ============================================================
// MOUSE EVENTS
// ============================================================

canvas.addEventListener(
  "mousedown",
  startDrawing
);


canvas.addEventListener(
  "mousemove",
  draw
);


canvas.addEventListener(
  "mouseup",
  stopDrawing
);


canvas.addEventListener(
  "mouseleave",
  stopDrawing
);


// ============================================================
// TOUCH EVENTS
// ============================================================

canvas.addEventListener(
  "touchstart",
  startDrawing,
  {
    passive: false
  }
);


canvas.addEventListener(
  "touchmove",
  draw,
  {
    passive: false
  }
);


canvas.addEventListener(
  "touchend",
  stopDrawing,
  {
    passive: false
  }
);


// ============================================================
// REDRAW CANVAS
// ============================================================

function redrawCanvas() {

  ctx.fillStyle =
    "#000000";


  ctx.fillRect(
    0,
    0,
    128,
    64
  );


  ctx.strokeStyle =
    "#ffffff";


  ctx.lineWidth =
    1.2;


  ctx.lineCap =
    "round";


  ctx.lineJoin =
    "round";


  for (
    let s = 0;
    s < strokes.length;
    s++
  ) {

    const stroke =
      strokes[s];


    if (
      stroke.length === 0
    ) {

      continue;

    }


    if (
      stroke.length === 1
    ) {

      ctx.fillStyle =
        "#ffffff";


      ctx.fillRect(
        stroke[0].x,
        stroke[0].y,
        1,
        1
      );


      continue;

    }


    ctx.beginPath();


    ctx.moveTo(
      stroke[0].x,
      stroke[0].y
    );


    for (
      let i = 1;
      i < stroke.length;
      i++
    ) {

      ctx.lineTo(
        stroke[i].x,
        stroke[i].y
      );

    }


    ctx.stroke();

  }


  updatePreviewCanvas();


  liveSendDrawing();

}


// ============================================================
// UNDO
// ============================================================

function undoStroke() {

  if (
    strokes.length === 0
  ) {

    return;

  }


  strokes.pop();


  redrawCanvas();

}


// ============================================================
// CLEAR
// ============================================================

function clearCanvas() {

  strokes = [];


  currentStroke = [];


  ctx.fillStyle =
    "#000000";


  ctx.fillRect(
    0,
    0,
    128,
    64
  );


  updatePreviewCanvas();


  liveSendDrawing();

}


// ============================================================
// CANVAS TO BASE64
// ============================================================

function canvasToBase64() {

  const image =
    ctx.getImageData(
      0,
      0,
      128,
      64
    );


  const bytes = [];


  let currentByte = 0;


  let bitCount = 0;


  for (
    let y = 0;
    y < 64;
    y++
  ) {

    for (
      let x = 0;
      x < 128;
      x++
    ) {

      const index =
        (
          y * 128 +
          x
        ) * 4;


      const brightness =
        image.data[index] +
        image.data[index + 1] +
        image.data[index + 2];


      const pixel =
        brightness > 200;


      currentByte =
        currentByte << 1;


      if (pixel) {

        currentByte |= 1;

      }


      bitCount++;


      if (
        bitCount === 8
      ) {

        bytes.push(
          currentByte
        );


        currentByte = 0;


        bitCount = 0;

      }

    }

  }


  let binary = "";


  for (
    let i = 0;
    i < bytes.length;
    i++
  ) {

    binary +=
      String.fromCharCode(
        bytes[i]
      );

  }


  return btoa(
    binary
  );

}


// ============================================================
// PREVIEW
// ============================================================

const preview =
  document.getElementById(
    "previewCanvas"
  );


const previewCtx =
  preview.getContext(
    "2d"
  );


function updatePreviewCanvas() {

  previewCtx.clearRect(
    0,
    0,
    128,
    64
  );


  previewCtx.drawImage(
    canvas,
    0,
    0
  );

}


// ============================================================
// TEXT PREVIEW
// ============================================================

function updatePreviewText(
  text,
  size
) {

  previewCtx.fillStyle =
    "#000000";


  previewCtx.fillRect(
    0,
    0,
    128,
    64
  );


  previewCtx.fillStyle =
    "#ffffff";


  const charWidth =
    6 * size;


  const lineHeight =
    8 * size;


  const maxChars =
    Math.floor(
      128 /
      charWidth
    );


  previewCtx.font =
    (
      8 * size
    ) +
    "px monospace";


  previewCtx.textBaseline =
    "top";


  let line = "";


  let y = 0;


  for (
    let i = 0;
    i < text.length;
    i++
  ) {

    const c =
      text[i];


    if (
      c === "\n"
    ) {

      if (
        y + lineHeight <= 64
      ) {

        previewCtx.fillText(
          line,
          0,
          y
        );

      }


      line = "";


      y += lineHeight;


      if (
        y >= 64
      ) {

        break;

      }


    } else {


      line += c;


      if (
        line.length >=
        maxChars
      ) {

        if (
          y + lineHeight <= 64
        ) {

          previewCtx.fillText(
            line,
            0,
            y
          );

        }


        line = "";


        y += lineHeight;


        if (
          y >= 64
        ) {

          break;

        }

      }

    }

  }


  if (
    line.length > 0 &&
    y + lineHeight <= 64
  ) {

    previewCtx.fillText(
      line,
      0,
      y
    );

  }

}


// ============================================================
// INITIAL PREVIEW
// ============================================================

updatePreviewCanvas();

</script>

</body>

</html>

)rawliteral";


// ============================================================
// ROOT
// ============================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    MAIN_PAGE
  );

}


// ============================================================
// TEXT HANDLER
// ============================================================

void handleText() {

  if (
    !server.hasArg("text")
  ) {

    server.send(
      400,
      "text/plain",
      "Missing text"
    );

    return;

  }


  String text =
    server.arg(
      "text"
    );


  if (
    server.hasArg("size")
  ) {

    currentTextSize =
      server.arg(
        "size"
      ).toInt();

  }


  if (
    currentTextSize < 1
  ) {

    currentTextSize = 1;

  }


  if (
    currentTextSize > 4
  ) {

    currentTextSize = 4;

  }


  Serial.print(
    "LIVE TEXT: "
  );


  Serial.println(
    text
  );


  Serial.print(
    "FONT SIZE: "
  );


  Serial.println(
    currentTextSize
  );


  displayText(
    text
  );


  server.send(
    200,
    "text/plain",
    "OK"
  );

}


// ============================================================
// DRAW HANDLER
// ============================================================

void handleDrawing() {

  if (
    !server.hasArg("data")
  ) {

    server.send(
      400,
      "text/plain",
      "Missing data"
    );

    return;

  }


  String data =
    server.arg(
      "data"
    );


  if (
    !decodeDrawing(
      data
    )
  ) {

    server.send(
      400,
      "text/plain",
      "Invalid drawing"
    );

    return;

  }


  displayDrawing();


  server.send(
    200,
    "text/plain",
    "OK"
  );

}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(
    115200
  );


  delay(500);


  // ----------------------------------------------------------
  // OLED
  // ----------------------------------------------------------

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );


  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDR
    )
  ) {

    Serial.println(
      "OLED NOT FOUND"
    );


    while (true) {

      delay(1000);

    }

  }


  display.clearDisplay();


  display.setTextColor(
    SSD1306_WHITE
  );


  display.setTextSize(1);


  display.setCursor(
    0,
    0
  );


  display.println(
    "ESP32 OLED"
  );


  display.println(
    "Live Studio"
  );


  display.display();


  clearDrawingBuffer();


  // ----------------------------------------------------------
  // WIFI ACCESS POINT
  // ----------------------------------------------------------

  WiFi.mode(
    WIFI_AP
  );


  WiFi.softAP(
    AP_SSID,
    AP_PASSWORD
  );


  IPAddress ip =
    WiFi.softAPIP();


  Serial.println();


  Serial.println(
    "================================"
  );


  Serial.println(
    "ESP32 OLED LIVE STUDIO"
  );


  Serial.println(
    "================================"
  );


  Serial.print(
    "WiFi Name: "
  );


  Serial.println(
    AP_SSID
  );


  Serial.print(
    "Password: "
  );


  Serial.println(
    AP_PASSWORD
  );


  Serial.print(
    "Open: http://"
  );


  Serial.println(
    ip
  );


  // ----------------------------------------------------------
  // WEB SERVER ROUTES
  // ----------------------------------------------------------

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );


  server.on(
    "/text",
    HTTP_POST,
    handleText
  );


  server.on(
    "/draw",
    HTTP_POST,
    handleDrawing
  );


  server.begin();


  Serial.println(
    "Web server started."
  );


  // ----------------------------------------------------------
  // READY SCREEN
  // ----------------------------------------------------------

  display.clearDisplay();


  display.setTextSize(1);


  display.setCursor(
    0,
    0
  );


  display.println(
    "WiFi Ready"
  );


  display.println();


  display.println(
    AP_SSID
  );


  display.println();


  display.println(
    "192.168.4.1"
  );


  display.display();

}

// ============================================================
// LOOP
// ============================================================

void loop() {

  server.handleClient();

}