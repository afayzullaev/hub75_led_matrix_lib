# HUB75 LED Matrix Library for ESP32 (ESP-IDF)

📟 A high-performance C library for controlling RGB LED matrices (HUB75) using ESP32 and the ESP-IDF framework.

---

## 📌 Features

- ✅ Compatible with HUB75 64x32 RGB LED panels
- ✅ Fast SPI (half-duplex) transmission
- ✅ Timer-based frame refresh and animation support
- ✅ Multi-color font rendering
- ✅ Multiple font sizes: **6x5**, **8x8**, **11x11**, **16x15**, **21x21**
- ✅ Per-letter RGB control
- ✅ Modular framebuffer: `rBuff`, `gBuff`, `bBuff`
- ✅ Utility drawing and animation functions
- ✅ Optimized for real-time performance

---

## 🛠️ Wiring (Example)

| Signal | Function             | GPIO |
|--------|----------------------|------|
| R1     | Red (upper)          | 11   |
| CLK    | Clock                | 12   |
| LAT    | Latch                | 13   |
| OE     | Output Enable        | 14   |
| A      | Row select A         | 1    |
| B      | Row select B         | 2    |
| C      | Row select C         | 9    |
| D      | Row select D         | 10   |

Edit these definitions in `hub75.h` as needed.

---

## 🔌 HUB75 Signal Flow (Daisy-Chaining LEDs)

### Порядок прохождения данных через сдвиговые регистры матрицы

| Вход      | Выход     |
|-----------|-----------|
| B1_IN     | G1_OUT    |
| G1_IN     | R1_OUT    |
| R2_IN     | B1_OUT    |
| G2_IN     | R2_OUT    |
| B2_IN     | G2_OUT    |

▶️ **Это означает:**

- Вы отправляете **R1**, затем **G1**, затем **B1** — и они проходят сквозь сдвиговые регистры.
- Затем то же самое происходит с **R2**, **G2**, **B2** — для нижней половины.
- Эти цепочки влияют на порядок и формат отправки данных в массив `row_data`.


---

## ⚙️ Initialization

In your `main.c`:

```c
#include "hub75.h"

void app_main(void) {
    hub75_init();          // GPIO setup
    hub75_timer_init();    // SPI + timer config
}
```
🖋️ Displaying Text
```c
HUB75_CLEAR();

Puts_STR_8("Hello", 5, 0, 0, 1, 0, 0);  // Red
Puts_STR_8("World", 5, 0, 10, 0, 1, 0); // Green

HUB75_PAINT_STR_CPY(); // Copy buffers to display
```

🧠 **Memory Structure**
```c
char rBuff[HEIGHT][WIDTH/8];  // Red channel
char gBuff[HEIGHT][WIDTH/8];  // Green channel
char bBuff[HEIGHT][WIDTH/8];  // Blue channel
char row_data[16][48];        // SPI transfer row cache
```
🔁 **Refresh System**

Uses two hardware timers:
	•	Display refresh — every 500 µs
	•	Animation trigger — every 16 ms

Timers are initialized via hub75_timer_init().

🧰 **To Do**
	•	I2S parallel output mode
	•	UTF-8/multilingual support
	•	Brightness control
	•	Line and rectangle primitives

👨‍💻 **Author**

Developed by Alisher Fayzullaev
ESP32 embedded developer / Indie Hacker