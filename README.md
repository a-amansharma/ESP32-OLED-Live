# ESP32 OLED Live Studio

A web-based live controller for a **0.96-inch 128×64 SSD1306 OLED display** using an ESP32.

Control the OLED directly from your phone or computer through a web interface hosted by the ESP32.

## ✨ Features

* ⌨️ **Live Type Mode** — type text and see it appear instantly on the OLED.
* ✏️ **Live Draw Mode** — draw using a mouse, trackpad, or touchscreen.
* 🔠 **Font Size Control** — 1× to 4×.
* ↩️ **Undo** — remove the last drawing stroke.
* 🗑️ **Clear** — clear the drawing and OLED.
* 🖥️ **OLED Preview** — browser preview of the 128×64 display.
* 📱 **Mobile Friendly** — works on phones, tablets, and computers.
* 📡 **No Internet Required** — the ESP32 creates its own Wi-Fi network.

---

# 🔧 Hardware

* ESP32 development board — 38-pin
* 0.96-inch 4-pin I2C SSD1306 OLED
* USB cable
* Phone, tablet, or computer with Wi-Fi

---

# 🔌 OLED Wiring

| OLED | ESP32   |
| ---- | ------- |
| VCC  | 3.3V    |
| GND  | GND     |
| SCL  | GPIO 22 |
| SDA  | GPIO 21 |

Default configuration:

```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C
```

If your OLED uses address `0x3D`, change:

```cpp
#define OLED_ADDR 0x3C
```

to:

```cpp
#define OLED_ADDR 0x3D
```

---

# 📚 Required Libraries

Install these from the **Arduino IDE Library Manager**:

* **Adafruit GFX Library**
* **Adafruit SSD1306**

ESP32 libraries such as `WiFi.h`, `WebServer.h`, and `Wire.h` are included with the ESP32 Arduino core.

---

# ⚙️ Setup

1. Open `ESP32_OLED_Live_Studio.ino` in Arduino IDE.
2. Select your ESP32 board under:

```text
Tools → Board → ESP32 Arduino
```

3. Select the correct USB port.
4. Upload the sketch.
5. Open Serial Monitor at **115200 baud**.

The ESP32 will start its own Wi-Fi network:

```text
Wi-Fi: ESP32-OLED
Password: 12345678
```

Connect your phone or computer to this network and open:

```text
http://192.168.4.1
```

That's it. No internet connection is required.

---

# ⌨️ Type Mode

Select **Type** in the web interface and start typing.

The OLED updates automatically as the text changes.

### Font Size

Choose between:

```text
1×  2×  3×  4×
```

Larger fonts display fewer characters because the OLED resolution is only **128×64 pixels**.

---

# ✏️ Draw Mode

Select **Draw** and draw directly on the 128×64 canvas.

Supported input:

* Mouse
* Trackpad
* Touchscreen

The drawing is converted to a monochrome bitmap and sent to the ESP32 in real time.

### Controls

**Undo**
Removes the last completed stroke.

**Clear**
Clears both the browser canvas and the OLED.

---

# 🧩 How It Works

The browser drawing canvas uses exactly:

```text
128 × 64 pixels
```

Each pixel is converted to:

```text
0 = OFF
1 = ON
```

The resulting bitmap is Base64 encoded and sent to the ESP32.

The ESP32 decodes the bitmap and updates the SSD1306 OLED.

Text is sent through:

```text
/text
```

Drawing data is sent through:

```text
/draw
```

Updates are throttled slightly during drawing to keep the interface responsive and reduce unnecessary network traffic.

---

# ⚙️ Wi-Fi Configuration

Default settings:

```cpp
const char* AP_SSID = "ESP32-OLED";
const char* AP_PASSWORD = "12345678";
```

Change them if required:

```cpp
const char* AP_SSID = "My-OLED";
const char* AP_PASSWORD = "87654321";
```

---

# 🔤 Text Limitation

The default Adafruit GFX font supports:

* English
* Numbers
* Basic ASCII
* Common supported symbols

Complex Unicode scripts such as **Hindi/Devanagari** are not supported by the default renderer.

A custom bitmap font system is required for Hindi and other complex scripts.

---

# 🛠️ Troubleshooting

### OLED is blank

Check:

```text
VCC → 3.3V
GND → GND
SDA → GPIO 21
SCL → GPIO 22
```

Also check the I2C address:

```cpp
#define OLED_ADDR 0x3C
```

Try `0x3D` if necessary.

### Web page does not open

Make sure your device is connected to:

```text
ESP32-OLED
```

Then open:

```text
http://192.168.4.1
```

### Upload error

If uploading stops with a serial/bootloader error:

* Reconnect the USB cable.
* Select the correct port.
* Close Serial Monitor.
* Hold the **BOOT** button while uploading.
* Try another USB cable.
* Avoid USB hubs.

---

# 📁 Project Structure

```text
ESP32_OLED_Live_Studio/
└── ESP32_OLED_Live_Studio.ino
```

Everything is contained in the single Arduino sketch:

* HTML
* CSS
* JavaScript
* Web server
* Wi-Fi access point
* OLED controller
* Type mode
* Draw mode

---

# 📌 Pin Summary

```text
ESP32
│
├── GPIO 21 ───── OLED SDA
├── GPIO 22 ───── OLED SCL
├── 3.3V ──────── OLED VCC
└── GND ───────── OLED GND
```

---

# 🚀 Quick Start

```text
1. Connect the OLED to the ESP32.
2. Install Adafruit GFX and Adafruit SSD1306.
3. Upload the .ino sketch.
4. Connect to Wi-Fi: ESP32-OLED
5. Open: http://192.168.4.1
6. Type or draw.
7. Watch the OLED update live.
```

---

# ☕ Buy Me a Coffee

If you like this project and want to support it:

```html
<a href="assets/buy-me-a-coffee.png" target="_blank">
  <img src="assets/buy-me-a-coffee.png" alt="Buy Me a Coffee" width="180">
</a>
```

Place your QR/support image here:

```text
assets/buy-me-a-coffee.png
```

Clicking the image will open the QR image in a new browser tab.

---

# 📄 License

This project is provided for **personal, educational, and prototype use**.

Feel free to modify and use the project for your own applications.
