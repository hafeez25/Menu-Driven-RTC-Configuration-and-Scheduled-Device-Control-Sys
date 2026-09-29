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
- [Block & Hardware Conncetions](#%EF%B8%8F-block--circuit-diagrams)
- [Hardware Requirements](#-hardware-requirements)
- [Software Requirements](#-software-requirements)
- [Pin Mapping](#-pin-mapping)
- [Circuit Connections](#-circuit-connections)
- [Getting Started](#-getting-started)
- [Firmware Architecture](#%EF%B8%8F-firmware-architecture)
- [What You See on the LCD](#%EF%B8%8F-what-you-see-on-the-lcd)
- [Menu System](#-menu-system)
- [Keypad Guide](#%EF%B8%8F-keypad-guide)
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
<img width="2232" height="1356" alt="image" src="https://github.com/user-attachments/assets/17ead945-ef89-49ef-82ac-dcbcb0080502" />



- **Circuit diagram** — pin-level view: LPC2148 to LCD, keypad, switch, device



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

<img width="1920" height="1280" alt="image" src="https://github.com/user-attachments/assets/46799464-6ae1-491b-a372-0a81b0c9e305" />


| LPC2148 | LCD pin |
|---|---|
| P0.8 – P0.15 | D0 – D7 |
| P0.16 | RS |
| P0.17 | RW |
| P0.18 | EN |

### 2. 4×4 matrix keypad

<img width="1920" height="1280" alt="image" src="https://github.com/user-attachments/assets/8ee54aa3-f250-4cf8-97d5-c9698150a2c3" />


| LPC2148 | Keypad |
|---|---|
| P1.16 – P1.19 | Rows R0 – R3 |
| P1.20 – P1.23 | Columns C0 – C3 |

### 3. Device output (active LOW)

<img width="1920" height="1280" alt="image" src="https://github.com/user-attachments/assets/ce52baa9-1f85-463e-8afe-f3e2b5559634" />


| LPC2148 | LED(Active Low) |
|---|---|
| P0.4 | Anode (+) of LED|
| GND | Cathode (−) of LED |

P0.4 LOW = device ON, P0.4 HIGH = device OFF.

### 4. Config switch (EINT0)

<img width="1920" height="1280" alt="image" src="https://github.com/user-attachments/assets/fd49b5e0-cb19-4b9c-a268-8c0cd4c39019" />


| LPC2148 | SWITCH (Active Low) |
|---|---|
| P0.1 | One terminal to SWITCH |
| GND | Another terminal to SWITCH |

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

**1. Run Screen**, alternates every 3 s:
```
00:00:47 THU OFF
SCHEDULE NOT SET
```
```
RTC NOT SET
PRESS SW TO SET
```
<img width="321" height="157" alt="Screenshot 2026-09-29 200423" src="https://github.com/user-attachments/assets/65dfad52-1bae-407e-bca2-9aa401857291" />
<img width="315" height="160" alt="Screenshot 2026-09-29 200013" src="https://github.com/user-attachments/assets/ff53ce1d-a873-4c30-a4a6-6c6c0bf6b075" />

**2. Main menu** (after the config switch):
```
1:RTC 2:SCHD
3:PIN 4:EXIT S:
```
<img width="321" height="161" alt="Screenshot 2026-09-29 200305" src="https://github.com/user-attachments/assets/a8e9d2cd-dbc5-487b-9053-6e58e815ee6c" />

**3. For Field entry of RTC Before We need to Enter 4-Digit pin**:
```
ENTER RTC PIN-4D
>1234_
```
<img width="311" height="167" alt="Screenshot 2026-09-29 200053" src="https://github.com/user-attachments/assets/323af62d-f2b9-45ce-84fd-53871f2b6d51" />

**4. RTC will get locked (If we Enter invalid pin more the 3 times)**
```            
WRONG PIN 
TRY AGAIN 
```
<img width="327" height="157" alt="Screenshot 2026-09-29 231124" src="https://github.com/user-attachments/assets/0ab88358-8914-490d-8a75-4669900f5a23" />
              
```            
TOO MANY TRIES
LOCKED 30 SEC
```
<img width="322" height="168" alt="Screenshot 2026-09-29 200325" src="https://github.com/user-attachments/assets/5bf5fb4a-bce1-4759-b389-081c19331721" />

```            
RTC LOCKED
WAIT 27 SEC   _
```
<img width="316" height="161" alt="Screenshot 2026-09-29 231138" src="https://github.com/user-attachments/assets/e49ef06a-4986-41ec-aa7d-b30f5133cb9e" />

**5. Update New RTC pin(We need to enter the previous pin to change)**
```
ENTER RTC PIN-4D
>1234_
```
<img width="311" height="167" alt="Screenshot 2026-09-29 200053" src="https://github.com/user-attachments/assets/0e60bd8b-6dce-4deb-aea0-ad994687c2ea" />

```
NEW PIN(4 DIG)
>1111_
```
<img width="313" height="166" alt="Screenshot 2026-09-29 200109" src="https://github.com/user-attachments/assets/edc708a9-b955-4e23-bd1b-4fd005e59f45" />

```
CONFIRM NEW PIN
>1111_
```
<img width="316" height="165" alt="Screenshot 2026-09-29 200130" src="https://github.com/user-attachments/assets/507ecf73-a263-4a85-8183-4b6edc44c023" />

```
PIN CHANGED
SUCCESSFULLY
```
<img width="321" height="167" alt="Screenshot 2026-09-29 200146" src="https://github.com/user-attachments/assets/3798a0bb-1bed-4186-bf05-8c56d3f9c8f8" />

**6. Field entry of RTC(Enter the RTC pin and Enter the Filed's)**
```
SET HOUR(0-23)
>
```

<img width="326" height="167" alt="Screenshot 2026-09-29 201526" src="https://github.com/user-attachments/assets/5480efa8-4c5f-4292-9ebf-e38ae486d5c3" />

```
SET MIN(0-59)
>
```
<img width="317" height="162" alt="Screenshot 2026-09-29 201608" src="https://github.com/user-attachments/assets/85ce6217-5947-4de1-b212-8a591c99ec28" />

```
SET SEC(0-59)
>
```
<img width="317" height="162" alt="Screenshot 2026-09-29 201625" src="https://github.com/user-attachments/assets/01c5ddb9-691f-4c60-adcf-9fe0c0fb7f6e" />

```
DAY(0SUN/6-SAT)
>
```
<img width="321" height="166" alt="Screenshot 2026-09-29 201642" src="https://github.com/user-attachments/assets/33f6c69e-5263-48b0-9670-9c7cde534a81" />

```
SET DATE(1-31)
>
```
<img width="316" height="160" alt="Screenshot 2026-09-29 201656" src="https://github.com/user-attachments/assets/d719ceb4-b36b-4d46-832f-c8b7612d2e5b" />

```
SET MONTH(1-12)
>
```
<img width="315" height="167" alt="Screenshot 2026-09-29 201746" src="https://github.com/user-attachments/assets/27849957-a04e-4a60-a2de-b800a847df6f" />

```
YEAR(2000-2030)
>
```
<img width="320" height="162" alt="Screenshot 2026-09-29 201824" src="https://github.com/user-attachments/assets/86126515-a221-47bd-996f-48c7f238a832" />

```
RTC UPDATED
SUCCESSFULLY
```
<img width="321" height="161" alt="Screenshot 2026-09-29 201848" src="https://github.com/user-attachments/assets/781f44f6-4eb2-437e-9650-6dc7ba8ce9d6" />

**6. Schedule the RTC**
```
ONE HOUR(0-23)
>_
```
<img width="321" height="158" alt="Screenshot 2026-09-29 201924" src="https://github.com/user-attachments/assets/14f84887-2503-443e-b45c-5db6d5d4a366" />

```
ONE MIN(0-59)
>_
```
<img width="320" height="172" alt="Screenshot 2026-09-29 201942" src="https://github.com/user-attachments/assets/fd3d3dc0-26dd-4482-aae9-329e0fecb9e7" />

```
OFF HOUR(0-23)
>_
```
<img width="338" height="161" alt="Screenshot 2026-09-29 201957" src="https://github.com/user-attachments/assets/aa624a21-9500-4b78-80ec-9a4f46f82d4a" />

```
OFF MIN(0-59)
>_
```
<img width="322" height="175" alt="Screenshot 2026-09-29 202033" src="https://github.com/user-attachments/assets/55363d4f-3fa1-45ee-8914-f03658d33026" />












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
  
##  Author
@Mohammad Hafeez
