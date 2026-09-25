# Menu-Driven RTC Configuration and Scheduled Device Control System
**ID: V25HE11M13** — LPC2148 / Embedded C / Keil uVision + Flash Magic

## Files

| File | Purpose |
|---|---|
| `main.c` | Banner, run screen, menu, PIN check, validation, device control |
| `rtc.c/.h` | RTC init, set/get time-date-day, leap year & days-in-month |
| `KPM.c/.h`, `KPM_defines.h` | 4x4 keypad, plus `KeyScanT()` with timeout |
| `LCD.c/.h`, `LCD_defines.h` | 16x2 LCD in 8-bit mode (from your class drivers) |
| `eint.c/.h` | Config switch on EINT0 — ISR only sets a flag |
| `timer.c/.h` | Timer0 free-running 1 ms tick used for input timeouts and the PIN lockout |
| `iap.c/.h` | In-Application Programming, schedule stored in last flash sector |
| `delay.c/.h`, `types.h`, `defines.h` | Your own class support headers, used exactly as supplied |

> `types.h` has no `#ifndef` guard, so the new headers (`rtc.h`, `KPM.h`,
> `eint.h`, `iap.h`, `timer.h`) do **not** include it. Each `.c` file includes
> `types.h` once, before the rest, exactly like your class files do.

## Hardware connections
<img width="850" height="612" alt="image" src="https://github.com/user-attachments/assets/16984f41-93bc-47d2-aa41-6dfe4f66b6b2" />

| Signal | Pins |
|---|---|
| LCD data D0–D7 | P0.8 – P0.15 |
| LCD RS / RW / EN | P0.16 / P0.17 / P0.18 |
| Keypad rows R0–R3 | P1.16 – P1.19 |
| Keypad cols C0–C3 | P1.20 – P1.23 |
| Config switch | P0.1 (EINT0, active low, to GND) |
| Device output | P0.4 (**active LOW** — pin driven LOW turns the device ON, driven HIGH turns it OFF) |

Clock assumption: FOSC 12 MHz, PLL ×5 → CCLK 60 MHz, VPBDIV 4 → PCLK 15 MHz.
If your board has the 32.768 kHz RTC crystal fitted, add `USE_EXT_32KHZ` to the
project defines and the RTC will use it instead of the PCLK prescaler.

> The device pin logic was changed from active-HIGH to **active-LOW**
> (`DEVICE_ON()` now pulls P0.4 low, `DEVICE_OFF()` drives it high) to match
> the board's relay/LED module. If you swap modules again, this is the only
> place to change — the two one-line macros near the top of `main.c`.

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
   3 seconds between `DD/MM/YYYY` and the schedule, shown as
   `HH:MMON HH:MMOFF` (ON time first, then OFF time — kept to 16 characters
   so it fits the line exactly).
3. **Press the config switch** → EINT0 fires, the ISR only sets `menuRequest`,
   and `main()` opens the menu: `1:RTC  2:SCHED  3:EXIT`.
4. **Option 1 (RTC)** first asks for a **4-digit PIN** (default `1234`,
   changeable in `main.c` via `#define RTC_PIN`). Get it right and you move on
   to hour, minute, second, day, date, month, year:
   - Day of week is prompted as `(0SUN...6SAT)DAY`, range 0–6.
   - Year is prompted as `(2000-2030)YEAR`, range restricted to 2000–2030.
   - Each field is range-checked; the date is re-checked against the chosen
     month and year, so 29 February is only accepted in a leap year.
   - Enter the PIN wrong **3 times in a row** and RTC editing **locks for
     1 minute**, with a live countdown shown on the LCD. It unlocks itself
     automatically once the minute is up — no reset needed.
5. **Option 2 (SCHED)** edits the ON and OFF hour/minute (no PIN needed).
   Equal ON and OFF times are rejected. The schedule is written to flash by
   IAP, so it survives a reset.
6. **Timeout** — 10 s with no key on the menu, and 10 s per field (including
   the PIN field) — either one drops you back to the menu / run screen
   automatically. The `>` prompt is shown on line 2 while a field is waiting
   for input.

Device window: ON time inclusive, OFF time exclusive. `09:00 → 17:00` runs the
device from 09:00:00 to 16:59:59. If ON is later than OFF (`22:00 → 06:00`)
the window crosses midnight. Until both the RTC and a schedule are valid, the
device stays OFF.

## RTC PIN

- Default PIN: **1234** — change it by editing `#define RTC_PIN` near the top
  of `main.c` and rebuilding.
- 3 wrong attempts in a row locks RTC editing for 60 seconds
  (`PIN_MAX_TRY` / `PIN_LOCK_MS` in `main.c`).
- The lock is tracked in RAM only (via the free-running Timer0 tick), so a
  reset also clears any active lockout.

## Time/date across power-off

The RTC calendar registers (`HOUR`, `MIN`, `SEC`, `DOM`, `MONTH`, `YEAR`,
`DOW`) live in the LPC2148's **VBAT domain**, separate from the main 3.3 V
supply. As long as VBAT is backed by a battery/supercap, the clock keeps
running with the board fully powered off, and `main()`'s `IsRTCValid()` check
means a valid saved time is never overwritten on the next boot. Without a
battery on VBAT, the registers lose their value when power is removed and the
board falls back to the default `01/01/2026 00:00:00 THU` on next boot.

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

### Enabling flash persistence (`USE_IAP = 1`), to fully match the brief
Your project brief asks for the schedule to be stored via IAP in the last
flash sector so it survives a reset. Before turning this on, **check your
flash size in Keil** (Project → Options for Target → Target tab → IROM1
Size) and make sure it matches `CFG_SECTOR`/`CFG_ADDR` in `iap.h`:

| IROM1 Size | Flash size | Last sector | CFG_SECTOR | CFG_ADDR       |
|---|---|---|---|---|
| `0x40000`  | 256 KB     | sector 14   | `14`       | `0x00038000`   |
| `0x80000`  | 512 KB     | sector 26   | `26`       | `0x0007C000`   |

`iap.h` currently ships set for a **256 KB** device (`CFG_SECTOR 14`,
`CFG_ADDR 0x00038000`), matching an `IROM1` size of `0x40000`. If your
board's IROM1 is `0x80000` (512 KB) instead, change those two lines to the
512 KB row above.

Getting this wrong is what actually caused the earlier "stuck at OFF MIN"
hang: `iap.h` originally assumed 512 KB flash while the project's actual
flash was 256 KB, so the write landed past the end of real flash. That
triggers a Data Abort exception, and this startup file's abort handler is
just `B DAbt_Handler` — an infinite loop — which is exactly what a hang
right after confirming the schedule looks like. It was **not** a stack
problem, despite that being the first, reasonable-looking suspect.

Once the sector matches your real flash size, set `#define USE_IAP 1` in
`iap.h` and rebuild.

## Build

Keil uVision → new project → NXP LPC2148 → add all `.c` files → Target: Xtal
12.0 MHz, tick **Use MicroLIB** off is fine, generate HEX file → build → flash
the HEX with Flash Magic (ISP, 19200 baud, Erase blocks used by Hex File only,
so the saved schedule in sector 26 is not wiped).
