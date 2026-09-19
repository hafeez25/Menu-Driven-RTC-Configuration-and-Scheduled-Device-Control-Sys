# Menu-Driven RTC Configuration and Scheduled Device Control System
**ID: V25HE11M13** — LPC2148 / Embedded C / Keil uVision + Flash Magic

## Files

| File | Purpose |
|---|---|
| `main.c` | Banner, run screen, menu, validation, device control |
| `rtc.c/.h` | RTC init, set/get time-date-day, leap year & days-in-month |
| `KPM.c/.h`, `KPM_defines.h` | 4x4 keypad, plus `KeyScanT()` with timeout |
| `LCD.c/.h`, `LCD_defines.h` | 16x2 LCD in 8-bit mode (from your class drivers) |
| `eint.c/.h` | Config switch on EINT0 — ISR only sets a flag |
| `timer.c/.h` | Timer0 free-running 1 ms tick used for input timeouts |
| `iap.c/.h` | In-Application Programming, schedule stored in last flash sector |
| `delay.c/.h`, `types.h`, `defines.h` | Your own class support headers, used exactly as supplied |

> `types.h` has no `#ifndef` guard, so the new headers (`rtc.h`, `KPM.h`,
> `eint.h`, `iap.h`, `timer.h`) do **not** include it. Each `.c` file includes
> `types.h` once, before the rest, exactly like your class files do.

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
If your board has the 32.768 kHz RTC crystal fitted, add `USE_EXT_32KHZ` to the
project defines and the RTC will use it instead of the PCLK prescaler.

## Keypad map

```
7 8 9 /
4 5 6 *
1 2 3 -
C 0 = +        C = clear/re-enter,  = = enter
```
Two-digit fields accept automatically once both digits are typed; `=` accepts a
single-digit entry early.

## Operation

1. **Power on** — line 1 shows `ID : V25HE11M13`, line 2 scrolls the project
   title, then the run screen appears.
2. **Run screen** — line 1: `HH:MM:SS DDD ON/OFF`. Line 2 alternates every
   3 seconds between `DD/MM/YYYY` and `ONhh:mm OFhh:mm`.
3. **Press the config switch** → EINT0 fires, the ISR only sets `menuRequest`,
   and `main()` opens the menu: `1:RTC  2:SCHED  3:EXIT`.
4. **Option 1** edits hour, minute, second, day, date, month, year. Each field
   is range-checked; the date is re-checked against the chosen month and year,
   so 29 February is only accepted in a leap year.
5. **Option 2** edits the ON and OFF hour/minute. Equal ON and OFF times are
   rejected. The schedule is written to flash by IAP, so it survives a reset.
6. **Timeout** — 10 s with no key on the menu, or 15 s on a field, returns to
   the run screen automatically.

Device window: ON time inclusive, OFF time exclusive. `09:00 → 17:00` runs the
LED from 09:00:00 to 16:59:59. If ON is later than OFF (`22:00 → 06:00`) the
window crosses midnight. Until both the RTC and a schedule are valid, the LED
stays OFF.

## Flash / IAP note

The settings record lives in **sector 26 at `0x0007C000`** — the last user
sector of the LPC2148's 500 KB user flash, well above the code that starts at
`0x00000000`. Keep the image far below that address so the two never overlap
(check the Keil map file if the project grows). Interrupts are disabled around
the erase/write sequence, and a magic word plus checksum guard against a
half-written record. An external EEPROM is left as future scope.

## Schedule storage: RAM by default, so nothing hangs

`USE_IAP` in `iap.h` is **0 by default**. Saving the schedule writes to RAM
only — instant, and it can never hang the board. The trade-off: it's lost on
reset/power-off. Everything else (menu, validation, device control) behaves
identically either way.

This is exactly what was causing "stuck at OFF MIN": once OFF MIN was
entered, the code immediately tried to write the schedule to flash via IAP,
and that write locks up the CPU unless a specific one-time Keil setting has
been changed first (below). With `USE_IAP = 0` this can't happen.

### Enabling flash persistence (`USE_IAP = 1`), to fully match the brief
Your project brief asks for the schedule to be stored via IAP in the last
flash sector so it survives a reset. To turn that on safely:
1. In Keil, open `Startup.s` (or its Configuration Wizard view).
2. Lower `Stack_Top` from `0x40008000` to `0x40007FE0` (LPC2148 has 32 KB RAM
   starting at `0x40000000`).
3. Rebuild, then set `#define USE_IAP 1` in `iap.h`.

Why this is needed: the IAP bootloader borrows the **top 32 bytes of
on-chip RAM**, which is exactly where Keil's default stack sits — the two
collide during the flash write and lock up the CPU. Moving the stack down
by 32 bytes fixes it permanently. Only set `USE_IAP` to `1` after doing this
step, or every save will hang again.

## Build

Keil uVision → new project → NXP LPC2148 → add all `.c` files → Target: Xtal
12.0 MHz, tick **Use MicroLIB** off is fine, generate HEX file → build → flash
the HEX with Flash Magic (ISP, 19200 baud, Erase blocks used by Hex File only,
so the saved schedule in sector 26 is not wiped).
