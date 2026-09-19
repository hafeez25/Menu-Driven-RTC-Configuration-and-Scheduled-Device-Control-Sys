# Menu-Driven RTC Configuration and Scheduled Device Control System

**ID: V25HE11M13** — LPC2129/LPC2148 · Embedded C · Keil uVision + Flash Magic

![Hardware block diagram](hardware_block_diagram.svg)

## Aim

To develop a menu-driven RTC configuration and scheduled device control
system using the LPC2148/LPC2129 microcontroller. The system displays the
current date and time on an LCD and allows users to configure RTC settings
and device ON/OFF times through keypad-operated menus. It automatically
switches the device ON and OFF according to the user-defined schedule.

## Objectives

1. Display RTC information (date, time) on an LCD.
2. Allow users to modify RTC settings via a 4x4 keypad.
3. Provide functionality to set device ON and OFF times.
4. Control a device's operational state based on the programmed timing.

## Requirements

**Hardware:** LPC2148/LPC2129, 16x2 LCD, 4x4 matrix keypad, LED, config
switch, USB-UART converter / DB-9 cable.

**Software:** Embedded C programming, Keil uVision, Flash Magic.

## Project workflow

At startup, the controller initializes the RTC, LCD, keypad, switch input,
and device output. The device stays OFF until a valid RTC time and a
schedule are both available.

During normal operation, the RTC keeps the current time, day and date, and
the controller continuously reads and displays it on the LCD. The display
alternates between clock information and the device's ON/OFF schedule to
suit the limited screen space.

The controller continuously compares the current RTC time against the
programmed operating period. The ON time is included in that period, and
the OFF time is excluded — e.g. an ON time of 09:00:00 and an OFF time of
17:00:00 runs the device from 09:00:00 up to just before 17:00:00. The
schedule repeats every day.

Pressing the configuration switch activates the menu: **1: Edit RTC**,
**2: Edit Device ON/OFF Times**, **3: Exit**, navigated with the keypad. The
switch is wired to an external interrupt; the ISR only records the request,
and the main program handles the menu and keypad interaction.

**Edit RTC** lets the user set hour, minute, second, day, date, month and
year. Every entry is validated, including days-per-month and leap years,
before the RTC is updated; invalid entries are rejected and re-prompted.

**Edit Device ON/OFF Times** lets the user set the activation and
deactivation times. Each entry is validated before the schedule is accepted.
If ON is later than OFF, the period crosses midnight (e.g. 22:00:00 to
06:00:00). Identical ON and OFF times are rejected as ambiguous.

The LED stays ON throughout the scheduled period and OFF outside it,
demonstrating automatic device control.

## Files

| File | Purpose |
|---|---|
| `main.c` | Banner, run screen, menu, validation, device control |
| `rtc.c/.h` | RTC init, set/get time-date-day, leap year & days-in-month |
| `KPM.c/.h`, `KPM_defines.h` | 4x4 keypad, plus `KeyScanT()` with timeout |
| `LCD.c/.h`, `LCD_defines.h` | 16x2 LCD in 8-bit mode |
| `eint.c/.h` | Config switch on EINT0 — ISR only sets a flag |
| `timer.c/.h` | Timer0 free-running 1 ms tick used for input timeouts |
| `iap.c/.h` | In-Application Programming, schedule stored in flash |
| `delay.c/.h`, `types.h`, `defines.h` | Support headers |
| `hardware_block_diagram.svg` | Block diagram for this README |

> `types.h` has no `#ifndef` guard, so `rtc.h`, `KPM.h`, `eint.h`, `iap.h`,
> and `timer.h` do **not** include it. Each `.c` file includes `types.h`
> once, before the rest.

## Hardware connections

| Signal | Pins |
|---|---|
| LCD data D0–D7 | P0.8 – P0.15 |
| LCD RS / RW / EN | P0.16 / P0.17 / P0.18 |
| Keypad rows R0–R3 | P1.16 – P1.19 |
| Keypad cols C0–C3 | P1.20 – P1.23 |
| Config switch | P0.1 (EINT0, active low, to GND) |
| Device LED | P0.4 (active high, with series resistor) |

Clock assumption: FOSC 12 MHz, PLL ×5 → CCLK 60 MHz, VPBDIV 4 → PCLK 15 MHz.
RTC runs from PCLK through the built-in prescaler by default, needing no
external 32.768 kHz crystal. Define `USE_RTC_XTAL` in `rtc.c` if your board
does have that crystal fitted and you'd rather use it.

> Add a photo of your actual wired-up board here for the GitHub repo
> (`hardware_photo.jpg`), alongside this block diagram.

## Keypad map

```
7 8 9 /
4 5 6 *
1 2 3 -
C 0 = +        C = clear/re-enter,  = = enter
```
Two-digit fields accept automatically once both digits are typed; `=` accepts
a single-digit entry early.

## Operation

1. **Power on** — line 1 shows `ID : V25HE11M13`, line 2 scrolls the project
   title, then the run screen appears.
2. **Run screen** — line 1: `HH:MM:SS DDD ON/OFF`. Line 2 alternates every
   3 seconds between `DD/MM/YYYY` and `ON09:00 OFF17:00`.
3. **Press the config switch** → EINT0 fires, the ISR only sets `menuRequest`,
   and `main()` opens the menu: `1:RTC  2:SCHED  3:EXIT`.
4. **Option 1** edits hour, minute, second, day, date, month, year. Each field
   is range-checked; the date is re-checked against the chosen month and year,
   so 29 February is only accepted in a leap year.
5. **Option 2** edits the ON and OFF hour/minute. Equal ON and OFF times are
   rejected.
6. **Timeout** — 10 s with no key on the menu, or 15 s on a field, returns to
   the run screen automatically.

Device window: ON time inclusive, OFF time exclusive. `09:00 → 17:00` runs the
LED from 09:00:00 to 16:59:59. If ON is later than OFF (`22:00 → 06:00`) the
window crosses midnight. Until both the RTC and a schedule are valid, the LED
stays OFF.

## Schedule storage: RAM by default, so nothing hangs

`USE_IAP` in `iap.h` is **0 by default**. Saving the schedule writes to RAM
only — instant, and it can never hang the board. The trade-off: it's lost on
reset/power-off. Everything else (menu, validation, device control) behaves
identically either way.

### Enabling flash persistence (`USE_IAP = 1`), to fully match the brief

The brief requires the schedule to be stored via IAP in the last flash
sector so it survives a reset. Before turning this on, **check your flash
size in Keil** (Project → Options for Target → Target tab → IROM1 Size) and
make sure it matches `CFG_SECTOR`/`CFG_ADDR` in `iap.h`:

| IROM1 Size | Flash size | Last sector | CFG_SECTOR | CFG_ADDR       |
|---|---|---|---|---|
| `0x40000`  | 256 KB     | sector 14   | `14`       | `0x00038000`   |
| `0x80000`  | 512 KB     | sector 26   | `26`       | `0x0007C000`   |

`iap.h` currently ships set for a **256 KB** device (`CFG_SECTOR 14`,
`CFG_ADDR 0x00038000`), matching an `IROM1` size of `0x40000`. If your
board's IROM1 is `0x80000` (512 KB) instead, change those two lines to the
512 KB row above.

Writing to a sector past the end of real flash triggers a Data Abort
exception, and the Keil startup file's abort handler is `B DAbt_Handler` —
an infinite loop — so a mismatched sector looks exactly like the board
"freezing" right after the schedule is confirmed.

Once the sector matches your real flash size, set `#define USE_IAP 1` in
`iap.h` and rebuild. The settings record is guarded with a magic word plus
checksum against a half-written record, and interrupts are disabled around
the erase/write sequence. An external EEPROM is left as future scope.

## Build

Keil uVision → new project → select your device (LPC2129 or LPC2148) →
add all `.c` files → Target: Xtal 12.0 MHz → generate HEX file → build →
flash the HEX with Flash Magic (ISP, 19200 baud, "Erase blocks used by Hex
File only" so a saved schedule in flash isn't wiped on re-flash).
