
# ESP32 OLED Live Studio

A web-based live controller for a 0.96-inch 128×64 SSD1306 OLED display using an ESP32.

The project provides two modes:

- **Type Mode** — type text in a browser and see it appear on the OLED immediately.
- **Draw Mode** — draw with a mouse or touchscreen and see the drawing reflected live on the OLED.
- **Font Size Control** — change text size from 1× to 4× with a live slider.
- **Undo / Clear** — manage drawings directly from the browser.
- **OLED Preview** — see a browser preview of the 128×64 OLED.

---

## Features

### Live Typing

Text is sent to the ESP32 as soon as it is typed.

There is no Send button.

The OLED updates automatically whenever the text changes.

### Live Font Size

The Type panel includes a font-size slider:

- 1× — smallest
- 2× — medium
- 3× — large
- 4× — largest

Changing the slider updates the OLED immediately.

Because the OLED is only 128×64 pixels, larger fonts naturally fit fewer characters and lines.

### Live Drawing

The Draw panel provides an exact 128×64 drawing canvas.

You can draw using:

- Mouse
- Trackpad
- Touchscreen

The drawing is converted into a 1-bit monochrome bitmap and sent to the ESP32.

### Drawing Controls

- **Undo** removes the most recent stroke.
- **Clear** removes the complete drawing.
- Changes are reflected on the OLED live.

### Correct OLED-Size Canvas

The drawing canvas internally uses exactly:

```text
128 × 64 pixels
```

This matches the physical OLED resolution.

---

# Hardware

## Required Components

- ESP32 development board, 38-pin version
- 0.96-inch 4-pin I2C OLED
- USB cable
- Computer or phone with Wi-Fi

The OLED should normally be an SSD1306 128×64 I2C display.

---

# OLED Wiring

| OLED Pin | ESP32 Pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | GPIO 22 |
| SDA | GPIO 21 |

The sketch uses:

```cpp
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C
```

If your OLED uses another I2C address, such as `0x3D`, change:

```cpp
#define OLED_ADDR 0x3C
```

to the correct address.

---

# Arduino Libraries

Install these libraries through the Arduino IDE Library Manager:

### Adafruit GFX Library

Used for OLED graphics and text rendering.

### Adafruit SSD1306

Used to control the SSD1306 OLED.

The following ESP32 libraries are normally included with the ESP32 Arduino core:

```cpp
WiFi.h
WebServer.h
Wire.h
```

---

# Arduino IDE Setup

Select your ESP32 board in:

```text
Tools
→ Board
→ ESP32 Arduino
```

For a typical ESP32 DevKit board, use the appropriate ESP32 Dev Module option.

Select the correct USB serial port.

Example on macOS:

```text
/dev/cu.usbserial-0001
```

Then click:

```text
Upload
```

---

# First Upload

After uploading, open:

```text
Tools
→ Serial Monitor
```

Set the baud rate to:

```text
115200
```

You should see something similar to:

```text
================================
ESP32 OLED LIVE STUDIO
================================
WiFi Name: ESP32-OLED
Password: 12345678
Open: http://192.168.4.1
Web server started.
```

---

# Connecting to the ESP32

The ESP32 creates its own Wi-Fi network.

Connect your phone, laptop, or computer to:

```text
Wi-Fi: ESP32-OLED
Password: 12345678
```

Then open a browser and go to:

```text
http://192.168.4.1
```

The control interface will open.

No internet connection is required.

---

# Using Type Mode

1. Connect to `ESP32-OLED`.
2. Open `http://192.168.4.1`.
3. Select **Type**.
4. Click the text box.
5. Start typing.
6. The OLED updates automatically.

Example:

```text
Hello
ESP32
```

The text is automatically wrapped according to the selected font size.

---

# Using the Font Slider

The font slider is located underneath the text box.

Move it between:

```text
1× ───────── 2× ───────── 3× ───────── 4×
```

The current font size is shown beside the slider.

For example:

```text
Font size                    2×
A ─────────────●──────────── A
```

The OLED changes immediately.

---

# Using Draw Mode

Select:

```text
Draw
```

A 128×64 drawing area appears.

Draw directly inside the canvas.

The ESP32 receives the drawing continuously while you draw.

This makes it suitable for:

- Signatures
- Logos
- Simple sketches
- Symbols
- Handwritten designs
- Custom OLED graphics

---

# Undo

Press:

```text
Undo
```

The last completed drawing stroke is removed.

The OLED is updated automatically.

---

# Clear

Press:

```text
Clear
```

This clears:

- Browser drawing canvas
- OLED display

---

# How the Drawing Works

The browser canvas internally uses:

```text
128 × 64 pixels
```

Each pixel is converted to a single binary value:

```text
0 = OFF
1 = ON
```

The resulting 1024-byte bitmap is Base64 encoded and sent to the ESP32.

The ESP32 decodes it and stores it in:

```cpp
bool drawingBuffer[64][128];
```

The OLED is then refreshed from that bitmap.

---

# How Live Typing Works

When the user types:

```text
Hello
```

the browser sends a POST request to:

```text
/text
```

The request contains:

```text
text=Hello
size=1
```

The ESP32 receives the values and immediately calls:

```cpp
displayText(text);
```

The OLED is therefore updated without requiring a page refresh.

---

# How Live Drawing Works

During drawing, the browser repeatedly sends the current 128×64 bitmap to:

```text
/draw
```

The ESP32:

1. Receives the Base64 bitmap.
2. Decodes it.
3. Converts it to the internal drawing buffer.
4. Updates the OLED.

The drawing update is throttled slightly to reduce unnecessary network traffic while keeping the display visually live.

---

# Project Structure

The complete project can be kept as a single Arduino sketch:

```text
ESP32_OLED_Live_Studio/
└── ESP32_OLED_Live_Studio.ino
```

The HTML, CSS, JavaScript, OLED controller, Wi-Fi server, typing system, and drawing system are all contained in the `.ino` file.

---

# Important Configuration

At the top of the sketch:

```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22

#define OLED_ADDR 0x3C
```

Wi-Fi credentials are:

```cpp
const char* AP_SSID = "ESP32-OLED";
const char* AP_PASSWORD = "12345678";
```

You can change them if required.

For example:

```cpp
const char* AP_SSID = "My-OLED";
const char* AP_PASSWORD = "87654321";
```

---

# Troubleshooting

## Upload stops with serial error

If Arduino reports:

```text
Serial data stream stopped
```

or:

```text
No more data to read from the serial port
```

try:

1. Disconnect and reconnect the USB cable.
2. Select the correct serial port.
3. Close Serial Monitor before uploading.
4. Press and hold the ESP32 **BOOT** button while upload starts.
5. Release BOOT when writing begins.
6. Try another USB cable.
7. Avoid USB hubs if possible.

If the sketch previously uploaded successfully, the error is usually related to the serial connection or ESP32 bootloader communication rather than the sketch size.

---

## OLED stays blank

Check:

```text
VCC → 3.3V
GND → GND
SDA → GPIO 21
SCL → GPIO 22
```

Also verify:

```cpp
#define OLED_ADDR 0x3C
```

Some OLED modules use:

```cpp
0x3D
```

instead.

---

## Browser cannot open the page

Make sure the device is connected to:

```text
ESP32-OLED
```

Then open:

```text
http://192.168.4.1
```

Do not use Google or another search engine for the address.

The ESP32 is acting as the Wi-Fi access point.

---

## Drawing is offset

The application is designed around an exact:

```text
128 × 64
```

canvas.

Make sure you are drawing inside the black canvas area rather than outside the canvas container.

---

## Text is too large

Use the font-size slider and select:

```text
1×
```

Large font sizes intentionally display fewer characters because the OLED resolution is only 128×64.

---

# Current Text Limitation

The current text renderer uses the standard Adafruit GFX font.

It is suitable for:

- English
- Numbers
- Basic ASCII characters
- Common symbols supported by the selected font

It does **not** natively render complex Unicode scripts such as Hindi/Devanagari.

For Hindi lyrics or other custom Unicode text, a custom bitmap font system should be integrated into the text renderer.

---

# Performance Notes

The ESP32 is running both:

- Wi-Fi access point
- HTTP web server
- OLED display controller

The drawing system intentionally sends updates at a controlled rate rather than making an HTTP request for every single mouse event.

This keeps the interface responsive and reduces unnecessary network traffic.

---

# Pin Summary

```text
ESP32
│
├── GPIO 21 ───── OLED SDA
├── GPIO 22 ───── OLED SCL
├── 3.3V ──────── OLED VCC
└── GND ───────── OLED GND
```

---

# Quick Start

```text
1. Wire OLED to ESP32
2. Install Adafruit GFX
3. Install Adafruit SSD1306
4. Open the .ino file
5. Select ESP32 board
6. Select USB port
7. Upload
8. Open Serial Monitor at 115200
9. Connect to Wi-Fi: ESP32-OLED
10. Open: http://192.168.4.1
11. Type or Draw
12. Watch the OLED update live
```

---

# License

This project is provided for personal, educational, and prototype use.

You are free to modify the HTML, CSS, JavaScript, ESP32 code, icons, drawing system, and OLED rendering logic for your own projects.
