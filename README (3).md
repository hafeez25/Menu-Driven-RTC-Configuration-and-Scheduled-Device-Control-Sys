# ⏰ Menu-Driven RTC Configuration and Scheduled Device Control System
**ID: V25HE11M13** · LPC2148 (ARM7TDMI-S) · Embedded C · Keil µVision · Flash Magic

An embedded automation project on the NXP LPC2148. A 16×2 LCD shows the live
date and time. A 4×4 keypad menu — locked behind a 4-digit PIN — lets you set
the clock and a daily ON/OFF schedule. The board then switches a device
(LED, relay, or similar) automatically according to that schedule.

## 🚀 Project at a Glance

| | |
|---|---|
| **What it does** | Shows clock → PIN-protected configuration → controls a device by schedule |
| **Brain** | LPC2148 (ARM7TDMI-S, 60 MHz) with on-chip RTC |
| **Input** | 4×4 keypad + one config push-button (EINT0) |
| **Output** | 16×2 LCD + device output (active LOW) |
| **Language / tools** | Embedded C · Keil µVision · Flash Magic |
| **Source** | Multi-file: `main.c` + `rtc`, `KPM`, `LCD`, `eint`, `timer`, `iap`, `delay` drivers |

## 📑 Table of Contents

- [Aim](#-aim)
- [Objectives](#-objectives)
- [Features](#-features)
- [Block & Circuit Diagrams](#%EF%B8%8F-block--circuit-diagrams)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [Pin Mapping](#-pin-mapping)
- [Circuit Connections](#-circuit-connections)
- [Getting Started](#-getting-started)
- [Firmware Architecture](#%EF%B8%8F-firmware-architecture)
- [What You See on the LCD](#%EF%B8%8F-what-you-see-on-the-lcd)
- [Menu System](#-menu-system)
- [Keypad Guide](#%EF%B8%8F-keypad-guide)
- [Schedule Logic](#%EF%B8%8F-schedule-logic)
- [RTC PIN and Lockout](#-rtc-pin-and-lockout)
- [Input Validation](#-input-validation)
- [Time and Date Across Power-Off](#-time-and-date-across-power-off)
- [Testing Checklist](#-testing-checklist)
- [Troubleshooting](#%EF%B8%8F-troubleshooting)
- [Project Structure](#-project-structure)
- [Source Code Overview](#-source-code-overview)
- [Technical Specifications](#%EF%B8%8F-technical-specifications)
- [Known Limitations](#%EF%B8%8F-known-limitations)
- [Future Enhancements](#-future-enhancements)

## 🎯 Aim

To develop a menu-driven RTC configuration and scheduled device control
system on the LPC2148, with PIN-protected access to the clock settings.

## 📋 Objectives

- Display RTC information (time, date, day of week) on a 16×2 LCD.
- Let the user edit the RTC (hour, minute, second, day, date, month, year)
  through a 4×4 keypad menu, gated by a 4-digit PIN.
- Let the user set a daily device ON and OFF time.
- Automatically switch the device according to the programmed schedule.
- Use interrupt-driven menu entry (EINT0), with the ISR doing the least
  possible work.
- Validate every entry and require explicit confirmation before anything
  is written to the RTC or the schedule.

## ✨ Features

| Feature | Description |
|---|---|
| Live RTC display | `HH:MM:SS DDD ON/OFF` on line 1, alternates date / schedule on line 2 |
| Interrupt-driven menu | EINT0 ISR only sets a flag; all menu work runs in `main()` |
| PIN-protected RTC edit | 4-digit PIN required before editing the clock |
| PIN change | Change the PIN from the menu (old PIN required first) |
| Lockout | 3 wrong PIN attempts locks RTC editing for 30 seconds, with a countdown |
| Explicit confirm | A field is only accepted once `=` is pressed — nothing updates on the fly |
| Backspace | `C` erases one digit at a time before cancelling the whole field |
| Smart schedule logic | Handles same-day and overnight (crosses-midnight) windows |
| Strict validation | Rejects out-of-range values, invalid dates, and identical ON/OFF times |
| Non-blocking timeout | Timer0 tick drives a 10 s timeout on the menu and on every field |
| Flash persistence (IAP) | Optional: schedule survives a reset, stored in the last flash sector |
| Modular drivers | Separate LCD, keypad, RTC, timer, interrupt, and IAP modules |

## 🗺️ Block & Circuit Diagrams

Two reference diagrams for this project:
- **Block diagram** — functional view: power, MCU core, input, output, flash storage
  <img width="2232" height="1356" alt="image" src="https://github.com/user-attachments/assets/d190f319-ac4c-4fba-ad5f-f8152153c2ff" />

- **Circuit diagram** — pin-level view: LPC2148 to LCD, keypad, switch, device
  <img width="1600" height="1045" alt="WhatsApp Image 2026-09-29 at 8 39 01 PM" src="https://github.com/user-attachments/assets/27253416-1344-41d5-9018-4edbecfc2525" />



## 🧩 Hardware Requirements

| Component | Specification / Notes | Qty |
|---|---|---|
| LPC2148 development board | ARM7TDMI-S, 60 MHz (PLL), on-chip RTC | 1 |
| 16×2 character LCD | HD44780-compatible, 8-bit interface | 1 |
| 4×4 matrix keypad | Membrane or tactile | 1 |
| LED or relay module | Represents / drives the controlled device, **active LOW** | 1 |
| Push-button switch | Wired to EINT0 (P0.1) | 1 |
| ISP/USB-UART interface | For Flash Magic programming | 1 |
| Power supply | 3.3 V regulated (+5 V for LCD if required) | 1 |
| Potentiometer (10 kΩ) | LCD contrast (V0 pin) | 1 |
| Resistor 330 Ω | Series resistor, if using an LED | 1 |
| Backup battery (coin cell, 3 V) | On VBAT — keeps RTC running when powered off | 1 |
| Pull-up resistors (10 kΩ) | Config switch, keypad columns — if not already on-board | as needed |

## 💻 Software Requirements

| Tool | Purpose |
|---|---|
| Keil µVision | Compile, link, debug |
| Flash Magic | Program the LPC2148 over ISP |
| `lpc21xx.h` | Peripheral register definitions |
| Embedded C | Application language |

## 📌 Pin Mapping

| Function | LPC2148 pin | Direction | Notes |
|---|---|---|---|
| LCD data D0–D7 | P0.8 – P0.15 | Output | 8-bit parallel |
| LCD RS | P0.16 | Output | Register select |
| LCD RW | P0.17 | Output | Held LOW (write only) |
| LCD EN | P0.18 | Output | Enable strobe |
| Keypad rows R0–R3 | P1.16 – P1.19 | Output | Driven low one at a time |
| Keypad columns C0–C3 | P1.20 – P1.23 | Input | Read back during scan |
| Config switch | P0.1 (EINT0) | Input | Falling-edge interrupt |
| Device output | P0.4 | Output | **Active LOW**: LOW = ON, HIGH = OFF |

> P0.14 doubles as the ISP-entry pin on this chip — it is **not** used here
> (LCD uses P0.8–P0.18), so there's no conflict with entering ISP mode.

## 🔌 Circuit Connections

### 1. 16×2 LCD (HD44780)

| LPC2148 | LCD pin |
|---|---|
| P0.8 – P0.15 | D0 – D7 |
| P0.16 | RS |
| P0.17 | RW |
| P0.18 | EN |
| +5 V | VCC (pin 2), backlight A (pin 15, through ~150 Ω if needed) |
| GND | VSS (pin 1), backlight K (pin 16) |
| Pot wiper | V0 / contrast (pin 3) |

### 2. 4×4 matrix keypad

| LPC2148 | Keypad |
|---|---|
| P1.16 – P1.19 | Rows R0 – R3 |
| P1.20 – P1.23 | Columns C0 – C3 |

### 3. Device output (active LOW)

```
Relay module VCC → +5 V        LED option:
Relay module GND → GND         P0.4 ──[330 Ω]──►|── GND
Relay module IN  → P0.4        (lights when P0.4 = LOW)
```
P0.4 LOW = device ON, P0.4 HIGH = device OFF.

### 4. Config switch (EINT0)

```
3.3 V ──[10 kΩ]──┬── P0.1 (EINT0)
                 │
              [switch]
                 │
                GND
```
Reads HIGH normally; pressing it pulls P0.1 LOW and fires the interrupt.

## 🏁 Getting Started

1. **Get the files** — all `.c`/`.h` sources plus this README.
2. **Collect the hardware** — see the table above.
3. **Wire the circuit** — as in Circuit Connections. Double-check LCD
   D0–D7 → P0.8–P0.15 and that rows/columns aren't swapped on the keypad.
4. **Install software** — Keil µVision (with LPC2148 device support) and
   Flash Magic.
5. **Create the Keil project** — New µVision Project → NXP → LPC2148 →
   accept the startup file copy → add all the `.c` files to the Source
   Group → Options for Target → Xtal 12.0 MHz → tick Create HEX File.
6. **Build** — F7. `0 Error(s)` produces the `.hex`.
7. **Flash** — put the board in ISP mode, open Flash Magic, select
   LPC2148, the correct COM port, 12 MHz oscillator, the generated `.hex`,
   tick "Erase blocks used by Hex File only" (so a saved schedule in flash
   isn't wiped), then Start. Reset into normal run mode afterwards.
8. **First run** — power on, see the clock screen, press the config
   switch, enter the default PIN `1234`, set the time, set a schedule a
   minute or two ahead, exit, and watch the device switch on schedule.

## 🏗️ Firmware Architecture

Three layers:
- **Application** (`main.c`) — banner, run screen, menu, PIN check, field
  validation, device control.
- **Drivers** — `rtc`, `KPM`, `LCD`, `eint`, `timer`, `iap`, `delay`, each
  with its own `.c`/`.h`.
- **Hardware** — LPC2148 peripheral registers via `lpc21xx.h`.

Main loop, every pass:
```c
GetRTCTimeInfo(...); GetRTCDateInfo(...); GetRTCDay(...);
UpdateDevice();   // compare time to schedule, drive the output
ShowStatus();     // redraw only what changed
if(menuRequest) { menuRequest = 0; ShowMenu(); refreshAll = 1; }
```
The EINT0 ISR only sets `menuRequest = 1` — all keypad and LCD work for the
menu happens back in `main()`.

## 🖥️ What You See on the LCD

**Run screen**, alternates every 3 s:
```
12:45:30 THU
24/09/2026
```
```
12:45:30 THU
09:00ON 17:00OFF
```

**Main menu** (after the config switch):
```
1:RTC 2:SCHD
3:PIN 4:EXIT S:
```

**Field entry** (example: setting the hour):
```
SET HOUR(0-23)
> 10
```

## 🎛️ Menu System

| Option | Function |
|---|---|
| 1 | Edit RTC (PIN required) |
| 2 | Edit device schedule |
| 3 | Change PIN (current PIN required) |
| 4 | Exit to run screen |

**Edit RTC fields**

| Field | Range |
|---|---|
| Hour | 0 – 23 |
| Minute | 0 – 59 |
| Second | 0 – 59 |
| Day of week | 0 – 6 (0 = SUN … 6 = SAT) |
| Date | 1 – 31, cross-checked against month/year (29 Feb only in leap years) |
| Month | 1 – 12 |
| Year | 2000 – 2030 |

**Edit schedule fields**

| Field | Range |
|---|---|
| ON hour | 0 – 23 |
| ON minute | 0 – 59 |
| OFF hour | 0 – 23 |
| OFF minute | 0 – 59 |

Identical ON and OFF times are rejected.

## ⌨️ Keypad Guide

```
        C0  C1  C2  C3
R0      7   8   9   /
R1      4   5   6   *
R2      1   2   3   -
R3      C   0   =   +
```

| Key | Meaning |
|---|---|
| 0 – 9 | Digit entry |
| `=` | Confirm the field — nothing is accepted until this is pressed |
| `C` | Erase the last digit; on an empty field, cancels and re-prompts |
| `/ * - +` | Not used by the current menu logic |

## ⏱️ Schedule Logic

- ON time is inclusive, OFF time is exclusive: `09:00 → 17:00` runs the
  device from 09:00:00 to 16:59:59.
- Same-day (ON < OFF): device ON when `now ≥ ON AND now < OFF`.
- Overnight (ON > OFF): device ON when `now ≥ ON OR now < OFF`.
  Example: `22:00 → 06:00` runs from 22:00 until 05:59 the next morning.
- Identical ON/OFF times are rejected at entry.
- Until both the RTC and a schedule are valid, the device stays OFF.

```
Hour       0  3  6  9  12 15 18 21
Same-day   ░░░░░░░░░████████░░░░░░░   ON=09:00 OFF=17:00
Overnight  ██████░░░░░░░░░░░░░░░░██   ON=22:00 OFF=06:00
```

## 🔐 RTC PIN and Lockout

- Default PIN: **1234** (`RTC_PIN_DEFAULT` in `main.c`).
- Required before editing the RTC, and before setting a new PIN.
- 3 wrong attempts in a row locks RTC/PIN editing for **30 seconds**, with a
  live countdown shown on the LCD; it unlocks automatically.
- The PIN and any lock state live in RAM only — a reset or power cycle
  resets the PIN back to 1234 and clears any lock.

## ✅ Input Validation

| Field | Validation |
|---|---|
| Hour | 0 – 23 |
| Minute / Second | 0 – 59 |
| Day of week | 0 – 6 |
| Date | 1 – 31, checked against days-in-month and leap years |
| Month | 1 – 12 |
| Year | 2000 – 2030 |
| ON / OFF times | Range-checked, rejected if identical |
| PIN | Must be a 4-digit match; 3 wrong tries triggers a 30 s lock |
| Any field | Not committed until `=` is pressed; `C` lets you correct a typo first |

## 🔋 Time and Date Across Power-Off

- The RTC's calendar registers live in the LPC2148's **VBAT domain**. With a
  battery on VBAT, they keep their value when the main supply is off, and
  `main()`'s validity check means a good saved time is never overwritten
  on the next boot.
- By default the RTC is clocked from PCLK, which stops when the board is
  off — so the time is **kept** but does not **advance** while powered
  down; it resumes from the same instant at power-on.
- To have the clock keep advancing while off, fit a 32.768 kHz crystal on
  RTCX1/RTCX2 and enable `#define USE_RTC_XTAL` in `rtc.c`.
- Without a VBAT battery, the registers are lost at power-off and the board
  falls back to the default `01/01/2026 00:00:00 THU`.

## 🧪 Testing Checklist

| # | Test | Expected result |
|---|---|---|
| 1 | Power on | Clock screen appears |
| 2 | Wait a few seconds | Line 2 alternates date / schedule |
| 3 | Press the config switch | Menu appears |
| 4 | Select 1, wrong PIN × 3 | RTC locks for 30 s with countdown |
| 5 | Select 1, correct PIN, Hour = 24 | Rejected, re-prompted |
| 6 | Set a valid time, confirm | Clock updates |
| 7 | Start typing, press `C` repeatedly | Digits erase one at a time, then field cancels |
| 8 | Select 2, ON = OFF | Rejected |
| 9 | Select 2, ON 1 min ahead, OFF 2 min ahead | Accepted, device switches on schedule |
| 10 | Select 3, change PIN, re-enter menu with new PIN | Accepted |
| 11 | Leave a field idle 10 s | Returns to menu |
| 12 | Leave the menu idle 10 s | Returns to run screen |
| 13 | Power off and on (VBAT fitted) | Time/date unchanged |

## 🛠️ Troubleshooting

| Problem | Check |
|---|---|
| LCD blank or dark boxes | Contrast pot, VCC/GND, RS/RW/EN wiring |
| LCD shows garbage | D0–D7 order (P0.8–P0.15), RW tied correctly |
| Keypad wrong/no keys | Row/column order, pull-ups on columns |
| Config switch does nothing | Pull-up on P0.1, switch to GND |
| Device output inverted | Confirm active-LOW wiring; swap `DEVICE_ON()`/`DEVICE_OFF()` in `main.c` if needed |
| Locked out of RTC menu | Wait 30 s for the lock to clear |
| Forgot the PIN | Default is 1234 unless changed; a full reflash resets it |
| Date resets to 01/01/2026 | No battery on VBAT |
| Schedule lost after reset | `USE_IAP` is 0 in `iap.h` |
| Board won't run after reset | Check P0.14 isn't held low at reset (ISP entry pin) |

## 📁 Project Structure

```
project/
├── main.c            # Application: menu, PIN, validation, device control
├── rtc.c / rtc.h      # RTC init, get/set time-date-day, leap year helpers
├── KPM.c / KPM.h / KPM_defines.h   # Keypad scanning
├── LCD.c / LCD.h / LCD_defines.h   # LCD driver
├── eint.c / eint.h    # EINT0 config switch
├── timer.c / timer.h  # Timer0 free-running tick (timeouts, PIN lock)
├── iap.c / iap.h       # Flash schedule storage (IAP)
├── delay.c / delay.h   # Blocking delays
├── types.h             # Type aliases (no include guard, by design)
└── defines.h            # Bit-manipulation macros
```

## 🔍 Source Code Overview

| Module / function | Purpose |
|---|---|
| `ShowBanner`, `ShowStatus` | Startup banner, run-screen rendering |
| `GetValue`, `ReadField` | Prompted, validated, confirm-with-`=` field entry |
| `VerifyPin`, `ChangePin` | PIN check and PIN change, with lockout |
| `EditRTC`, `EditSchedule` | Menu options 1 and 2 |
| `ShowMenu` | Menu display and dispatch |
| `UpdateDevice` | Schedule-vs-clock comparison, drives the output |
| `RTC_Init`, `SetRTC*`, `GetRTC*`, `IsRTCValid`, `DaysInMonth` | RTC driver |
| `InitKPM`, `KeyScanT` | Keypad scan with timeout |
| `EINT0_Enable`, `eint0_isr` | Config-switch interrupt |
| `InitTimer0`, `GetTicks`, `Elapsed` | 1 ms tick for all timeouts and the PIN lock |
| `IAP_SaveSchedule`, `IAP_LoadSchedule` | Optional flash persistence for the schedule |

## ⚙️ Technical Specifications

| Parameter | Value |
|---|---|
| Microcontroller | NXP LPC2148 (ARM7TDMI-S) |
| System clock (CCLK) | 60 MHz (PLL ×5 from 12 MHz crystal) |
| Peripheral clock (PCLK) | 15 MHz |
| RTC clock source | PCLK-derived prescaler by default; 32.768 kHz optional (`USE_RTC_XTAL`) |
| LCD interface | 8-bit parallel (HD44780) |
| Keypad | 4×4 matrix scan |
| Interrupt | EINT0, falling edge, VIC-vectored |
| Timer | Timer0, free-running 1 ms tick |
| Flash sector for schedule | Last user sector (14 @ 256 KB / 26 @ 512 KB), optional IAP |
| Programming interface | ISP via Flash Magic |

## ⚠️ Known Limitations

- The PIN and its lock state are RAM-only; both reset on power-cycle.
- Schedule persistence via IAP is optional (`USE_IAP` defaults to 0).
- By default the RTC doesn't advance while the board is powered off
  (needs the 32.768 kHz crystal option for that).
- Single daily ON/OFF schedule — no per-day-of-week variation yet.

## 🚀 Future Enhancements

- Save the PIN to flash so it survives a reset.
- Multiple or per-weekday schedules.
- UART logging of configuration changes.
- External battery-backed RTC (e.g. DS1307) as a fallback.
- Modular unit tests for the validation and schedule logic.
