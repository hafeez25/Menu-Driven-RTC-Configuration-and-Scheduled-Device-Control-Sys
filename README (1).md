# ⏰ Menu-Driven RTC Configuration and Scheduled Device Control System

An embedded automation project built using the **NXP LPC2148 (ARM7
TDMI-S)** microcontroller.

The system displays the real-time clock on a **16×2 LCD**, allows the
user to configure the RTC and daily device ON/OFF schedule using a **4×4
keypad**, and automatically controls an LED according to the programmed
schedule.

![Hardware Setup](1000048757.jpg)

------------------------------------------------------------------------

## 🚀 Project at a Glance

  -----------------------------------------------------------------------
  What it does                        Shows RTC → configures
                                      time/schedule → controls device
                                      automatically
  ----------------------------------- -----------------------------------
  **Microcontroller**                 NXP LPC2148 -- ARM7 TDMI-S

  **System Clock**                    60 MHz

  **RTC**                             On-chip Real-Time Clock

  **Input**                           4×4 matrix keypad + EINT0 push
                                      button

  **Output**                          16×2 LCD + LED/device

  **Language**                        Embedded C

  **IDE**                             Keil µVision

  **Programming Tool**                Flash Magic

  **Interface**                       GPIO, EINT0, RTC, Timer, IAP
  -----------------------------------------------------------------------

------------------------------------------------------------------------

## 📑 Table of Contents

-   [Aim](#-aim)
-   [Objectives](#-objectives)
-   [Features](#-features)
-   [Block Diagram](#-block-diagram)
-   [Hardware Requirements](#-hardware-requirements)
-   [Software Requirements](#-software-requirements)
-   [Pin Mapping](#-pin-mapping)
-   [Circuit Connections](#-circuit-connections)
-   [Getting Started](#-getting-started)
-   [Firmware Architecture](#-firmware-architecture)
-   [Project Workflow](#-project-workflow)
-   [LCD Output](#-lcd-output)
-   [Menu System](#-menu-system)
-   [Keypad Guide](#-keypad-guide)
-   [Schedule Logic](#-schedule-logic)
-   [Input Validation](#-input-validation)
-   [Testing Checklist](#-testing-checklist)
-   [Troubleshooting](#-troubleshooting)
-   [Project Structure](#-project-structure)
-   [Source Code Overview](#-source-code-overview)
-   [Technical Specifications](#-technical-specifications)
-   [Known Limitations](#️-known-limitations)
-   [Future Enhancements](#-future-enhancements)

------------------------------------------------------------------------

## 🎯 Aim

To develop a **menu-driven RTC configuration and scheduled device
control system** using the LPC2148 microcontroller.

The system:

-   Displays the current date and time on a 16×2 LCD.
-   Allows the user to configure RTC settings.
-   Allows the user to configure daily device ON/OFF times.
-   Automatically controls the device according to the configured
    schedule.
-   Uses an interrupt-driven switch to enter the configuration menu.
-   Validates user-entered values before applying changes.

------------------------------------------------------------------------

## 📋 Objectives

1.  Display RTC time, date, and day of the week.
2.  Configure hour, minute, day, date, month, and year.
3.  Configure device ON and OFF hours/minutes.
4.  Control an LED as a representative device/load.
5.  Implement a 4×4 matrix keypad interface.
6.  Use EINT0 for interrupt-driven menu entry.
7.  Support schedules that cross midnight.
8.  Validate numeric inputs and reject invalid values.
9.  Provide automatic menu timeout.
10. Store schedule information in Flash using IAP when enabled.

------------------------------------------------------------------------

## ✨ Features

  -----------------------------------------------------------------------
  Feature                             Description
  ----------------------------------- -----------------------------------
  **Live RTC Display**                Displays HH:MM:SS, day and date

  **RTC Configuration**               Edit hour, minute, day, date, month
                                      and year

  **Schedule Configuration**          Set daily ON and OFF times

  **Automatic Device Control**        LED follows the configured RTC
                                      schedule

  **4×4 Keypad**                      Used for menu navigation and
                                      numeric entry

  **EINT0 Menu Entry**                Push button opens configuration
                                      menu

  **Input Validation**                Rejects invalid time/date/schedule
                                      values

  **Overnight Schedule**              Supports schedules such as
                                      22:00--06:00

  **Menu Timeout**                    Automatically exits inactive
                                      configuration mode

  **Flash Persistence**               Optional IAP-based schedule storage

  **LCD Interface**                   8-bit parallel HD44780-compatible
                                      LCD
  -----------------------------------------------------------------------

------------------------------------------------------------------------

## 🖼️ Block Diagram

``` text
                    ┌──────────────────────┐
                    │      LPC2148         │
                    │    ARM7 TDMI-S       │
                    │                      │
                    │   RTC / GPIO /       │
                    │   Timer / EINT0      │
                    └──────────┬───────────┘
                               │
             ┌─────────────────┼─────────────────┐
             │                 │                 │
             ▼                 ▼                 ▼
      ┌─────────────┐   ┌─────────────┐   ┌─────────────┐
      │   4×4       │   │   16×2 LCD  │   │    LED /    │
      │   Keypad    │   │             │   │   Device    │
      └─────────────┘   └─────────────┘   └─────────────┘
             │
             ▼
      ┌─────────────┐
      │ EINT0 Push  │
      │   Button    │
      └─────────────┘
```

------------------------------------------------------------------------

## 🧩 Hardware Requirements

  Component                   Specification                        Qty
  --------------------------- ---------------------------------- -----
  LPC2148 Development Board   ARM7 TDMI-S                            1
  16×2 LCD                    HD44780 compatible                     1
  4×4 Matrix Keypad           Matrix type                            1
  LED                         Device/load indication                 1
  Push Button                 EINT0 menu switch                      1
  Resistor                    220 Ω for LED                          1
  Potentiometer               LCD contrast                           1
  USB-to-UART / RS-232        Programming/debugging interface        1
  Connecting Wires            As required                          ---
  Power Supply                As required by development board       1

------------------------------------------------------------------------

## 💻 Software Requirements

  Tool / Library     Purpose
  ------------------ ----------------------------------------
  **Keil µVision**   Embedded C development and compilation
  **Flash Magic**    Programming the LPC2148
  **Embedded C**     Firmware development
  **lpc21xx.h**      LPC2148 register definitions
  **Proteus**        Optional simulation/debugging

------------------------------------------------------------------------

## 📌 Pin Mapping

  Function       LPC2148 Pin   Direction   Description
  -------------- ------------- ----------- ------------------
  LCD D0--D7     P0.8--P0.15   Output      8-bit LCD data
  LCD RS         P0.17         Output      Register Select
  LCD EN         P0.18         Output      Enable
  Keypad Row 0   P1.16         Output      Row scanning
  Keypad Row 1   P1.17         Output      Row scanning
  Keypad Row 2   P1.18         Output      Row scanning
  Keypad Row 3   P1.19         Output      Row scanning
  Keypad Col 0   P1.20         Input       Column detection
  Keypad Col 1   P1.21         Input       Column detection
  Keypad Col 2   P1.22         Input       Column detection
  Keypad Col 3   P1.23         Input       Column detection
  Device / LED   P1.30         Output      Device control
  EINT0 Switch   P0.16         Input       Menu interrupt

> **Note:** Verify the exact pin mapping against the firmware and
> development-board schematic before wiring.

------------------------------------------------------------------------

## 🔌 Circuit Connections

### 1. 16×2 LCD

  LPC2148               LCD
  --------------------- ---------------
  P0.8--P0.15           D0--D7
  P0.17                 RS
  P0.18                 EN
  GND                   R/W
  VCC                   VCC
  GND                   GND
  Potentiometer wiper   V0 / Contrast

### 2. 4×4 Keypad

  LPC2148        Keypad
  -------------- ----------------
  P1.16--P1.19   Rows R0--R3
  P1.20--P1.23   Columns C0--C3

### 3. Device / LED

``` text
LPC2148 P1.30
      │
     220Ω
      │
    LED Anode
    LED Cathode
      │
     GND
```

``` text
P1.30 = HIGH → LED ON
P1.30 = LOW  → LED OFF
```

### 4. EINT0 Push Button

``` text
P0.16 / EINT0 ───── Push Button ───── GND
```

A pull-up configuration can be used so that pressing the button produces
the required interrupt edge.

------------------------------------------------------------------------

## 🏁 Getting Started

### Step 1 --- Create the Keil Project

1.  Open **Keil µVision**.
2.  Create a new project.
3.  Select **NXP LPC2148**.
4.  Add the project `.c` source file.
5.  Add the required startup file.
6.  Set the crystal frequency to **12 MHz**.
7.  Enable **Create HEX File** in the Output settings.

### Step 2 --- Build

Press:

``` text
F7 → Build Target
```

The project should build without errors.

### Step 3 --- Program the LPC2148

1.  Connect the programming interface.
2.  Put the development board into the required programming/ISP mode.
3.  Open Flash Magic.
4.  Select **LPC2148**.
5.  Select the correct COM port.
6.  Set oscillator frequency according to the board.
7.  Select the generated `.hex` file.
8.  Start programming.
9.  Return the board to execution mode.
10. Reset the board.

------------------------------------------------------------------------

## 🏗️ Firmware Architecture

``` text
┌──────────────────────────────────────┐
│          Application Layer           │
│  Display / Menu / Schedule Control   │
└──────────────────┬───────────────────┘
                   │
┌──────────────────▼───────────────────┐
│           Driver Layer               │
│ LCD / Keypad / RTC / Timer / EINT0  │
└──────────────────┬───────────────────┘
                   │
┌──────────────────▼───────────────────┐
│          LPC2148 Hardware            │
│ GPIO / RTC / Timer / VIC / IAP      │
└──────────────────────────────────────┘
```

------------------------------------------------------------------------

## 🔄 Project Workflow

### 1. System Initialization

At startup the firmware initializes:

-   GPIO
-   LCD
-   Keypad
-   RTC
-   Timer
-   EINT0 interrupt
-   Optional Flash/IAP configuration

### 2. Main Loop

Conceptually:

``` c
while(1)
{
    Display();

    if(menu_request_flag)
    {
        Handle_Menu();
    }
}
```

### 3. Display

The display function:

-   Reads RTC values.
-   Formats the time/date.
-   Updates the LCD.
-   Shows schedule information.
-   Checks the current time against the configured schedule.
-   Controls the device/LED.

### 4. Interrupt

The EINT0 ISR should remain short. It sets a software flag, and the main
loop handles the menu.

``` text
Push Button
     │
     ▼
   EINT0
     │
     ▼
Interrupt Service Routine
     │
     ▼
Set Menu Flag
     │
     ▼
Main Loop
     │
     ▼
Open Configuration Menu
```

------------------------------------------------------------------------

## 🖥️ LCD Output

### Clock Screen

``` text
┌────────────────┐
│11:26:33 TUE ON │
│ON11:26 OFF11:27│
└────────────────┘
```

The exact displayed format depends on the firmware version.

### Schedule Screen

Example:

``` text
┌────────────────┐
│ON : 09:00      │
│OFF: 17:00      │
└────────────────┘
```

### Main Menu

``` text
┌────────────────┐
│1. EDIT RTC     │
│2. EDIT SCHEDULE│
└────────────────┘
```

------------------------------------------------------------------------

## 🎛️ Menu System

### Main Menu

    Option Function
  -------- ----------------------
         1 Edit RTC
         2 Edit Device Schedule
         3 Exit

### Edit RTC Menu

    Option Field         Valid Range
  -------- ------------- -------------
         1 Hour          00--23
         2 Minute        00--59
         3 Day of Week   0--6
         4 Date          01--31
         5 Month         01--12
         6 Year          2000--2099
         7 Exit          ---

### Edit Schedule Menu

    Option Field        Valid Range
  -------- ------------ -------------
         1 ON Hour      00--23
         2 ON Minute    00--59
         3 OFF Hour     00--23
         4 OFF Minute   00--59
         5 Exit         ---

------------------------------------------------------------------------

## ⌨️ Keypad Guide

Typical keypad mapping:

``` text
┌─────┬─────┬─────┬─────┐
│  7  │  8  │  9  │  /  │
├─────┼─────┼─────┼─────┤
│  4  │  5  │  6  │  *  │
├─────┼─────┼─────┼─────┤
│  1  │  2  │  3  │  -  │
├─────┼─────┼─────┼─────┤
│  C  │  0  │  =  │  +  │
└─────┴─────┴─────┴─────┘
```

  Key    Function
  ------ -----------------------------------------
  0--9   Numeric input
  `=`    Confirm value
  `C`    Cancel / exit
  `+`    Confirm / scroll down depending on menu
  `-`    Scroll up depending on menu

------------------------------------------------------------------------

## ⏱️ Schedule Logic

The device operates according to the configured ON and OFF times.

### Same-Day Schedule

Example:

``` text
ON  = 09:00
OFF = 17:00
```

The device is:

``` text
09:00:00 ≤ Current Time < 17:00:00
```

So the LED remains ON from 09:00:00 until 16:59:59.

### Overnight Schedule

Example:

``` text
ON  = 22:00
OFF = 06:00
```

The device remains ON:

``` text
22:00 → 23:59
00:00 → 05:59
```

### Decision Logic

``` text
             Is ON < OFF?
                /     \
              YES      NO
               │        │
               ▼        ▼
        Current >= ON   Current >= ON
        AND             OR
        Current < OFF   Current < OFF
               │        │
               └────┬───┘
                    ▼
              Device ON/OFF
```

Identical ON and OFF times are rejected.

------------------------------------------------------------------------

## ✅ Input Validation

  Parameter     Validation
  ------------- ------------------------
  Hour          0--23
  Minute        0--59
  Day           0--6
  Date          1--31
  Month         1--12
  Year          2000--2099
  ON/OFF time   Valid range + ON ≠ OFF

Example invalid inputs:

``` text
Hour   = 24  → Rejected
Minute = 60  → Rejected
Month  = 13  → Rejected
Year   = 1999 → Rejected
ON     = OFF → Rejected
```

The old value remains unchanged when an invalid value is entered.

------------------------------------------------------------------------

## 🧪 Testing Checklist

    \# Test                     Expected Result
  ---- ------------------------ --------------------------------
     1 Power ON                 Clock appears
     2 Wait                     Display updates continuously
     3 Press EINT0              Main menu opens
     4 Enter Hour = 24          Rejected
     5 Enter Minute = 60        Rejected
     6 Enter Month = 13         Rejected
     7 Enter Year = 1999        Rejected
     8 Enter valid time         RTC updates
     9 Press `C` during input   Entry cancelled
    10 ON = OFF                 Schedule rejected
    11 Same-day schedule        LED follows schedule
    12 Overnight schedule       LED remains ON across midnight
    13 Exit menu                Normal display resumes
    14 Leave menu inactive      Timeout returns to normal mode

------------------------------------------------------------------------

## 🛠️ Troubleshooting

  -----------------------------------------------------------------------
  Problem                 Possible Cause          Solution
  ----------------------- ----------------------- -----------------------
  LCD blank               Contrast/power/wiring   Adjust V0 and check
                                                  connections

  Garbage LCD characters  Incorrect data wiring   Verify D0--D7 order

  Keypad not working      Row/column wiring       Verify P1.16--P1.23

  Menu not opening        EINT0 wiring            Check P0.16 and switch

  LED not turning ON      Schedule/wiring         Verify time and LED
                                                  connection

  RTC incorrect           RTC not configured      Check RTC
                                                  initialization

  Flash programming fails ISP/COM settings        Check board mode and
                                                  COM port

  Keil build fails        Project/device          Check LPC2148 target
                          configuration           and startup file
  -----------------------------------------------------------------------

------------------------------------------------------------------------

## 📁 Project Structure

``` text
Menu-Driven-RTC-Scheduled-Device-Control/
│
├── Menu-Driven-RTC-Scheduled-Device-Control.c
├── README.md
│
├── Images/
│   ├── Hardware_Setup.jpg
│   ├── Block_Diagram.png
│   ├── Circuit_LCD.png
│   ├── Circuit_Keypad.png
│   ├── Circuit_LED.png
│   ├── Circuit_Switch.png
│   ├── Clock_View.png
│   ├── Schedule_View.png
│   ├── Main_Menu.png
│   └── Device_ON_OFF.png
│
└── docs/
    └── Project_Documentation.pdf
```

------------------------------------------------------------------------

## 🔍 Source Code Overview

  Module / Function Group   Purpose
  ------------------------- ------------------------------------
  Clock Configuration       FOSC, CCLK and PCLK configuration
  GPIO                      LED, switch and peripheral control
  LCD Driver                Commands, data and string display
  Keypad Driver             Matrix scanning and key detection
  RTC Driver                Time/date configuration
  EINT0                     Interrupt-driven menu entry
  Timer                     Delays and menu timeout
  Schedule Logic            ON/OFF time comparison
  Menu System               RTC and schedule configuration
  IAP                       Optional Flash persistence

------------------------------------------------------------------------

## ⚙️ Technical Specifications

  Parameter         Value
  ----------------- ------------------------------
  Microcontroller   NXP LPC2148
  CPU Core          ARM7 TDMI-S
  System Clock      60 MHz
  Crystal           12 MHz
  RTC               On-chip RTC
  LCD               16×2, 8-bit
  Keypad            4×4 matrix
  Interrupt         EINT0
  Device Output     LED
  Programming       Flash Magic / UART interface
  Language          Embedded C
  IDE               Keil µVision

------------------------------------------------------------------------

## ⚠️ Known Limitations

-   Date validation currently treats all months as having 31 days.
-   Leap-year validation can be added.
-   Only one daily ON/OFF schedule is supported.
-   RTC time may need to be restored after complete power loss unless
    backup storage is implemented.
-   The LED represents the actual controlled device/load.
-   The implementation can be split into separate driver files for
    better maintainability.

------------------------------------------------------------------------

## 🚀 Future Enhancements

-   Full calendar validation.
-   Leap-year support.
-   Multiple daily schedules.
-   Weekly schedule configuration.
-   RTC backup battery.
-   EEPROM/Flash storage for multiple profiles.
-   UART event logging.
-   Password-protected configuration.
-   Buzzer/status indicators.
-   Real relay or appliance control.
-   Modular `lcd.c/.h`, `keypad.c/.h`, `rtc.c/.h`, etc.
-   Improved low-power operation.

------------------------------------------------------------------------

## 🎓 Interview / Project Explanation

### What is the project?

> This project is a menu-driven RTC-based automation system using the
> LPC2148 ARM7 microcontroller. It displays the current date and time on
> a 16×2 LCD, allows the user to configure the RTC and daily ON/OFF
> schedule through a 4×4 keypad, and automatically controls an LED
> according to the configured time.

### Why RTC?

> RTC provides continuous timekeeping, which allows the microcontroller
> to perform operations based on real-world time instead of software
> delay loops.

### Why EINT0?

> EINT0 is used to enter the configuration menu through an external push
> button without continuously polling the button in the main
> application.

### Why a keypad?

> The 4×4 matrix keypad provides a simple user interface for selecting
> menu options and entering time and schedule values.

### How is the LED controlled?

> The firmware continuously compares the current RTC time with the
> configured ON and OFF times. If the current time is inside the
> scheduled interval, the LED is turned ON; otherwise it is turned OFF.

------------------------------------------------------------------------

## 📄 License

This project is intended for **educational and embedded-systems learning
purposes**.

------------------------------------------------------------------------

## 🙏 Acknowledgements

-   NXP LPC2148 / ARM7 TDMI-S documentation
-   Keil µVision
-   Flash Magic
-   Embedded C and microcontroller development resources
-   Vector India Embedded Systems training environment

------------------------------------------------------------------------

## 👨‍💻 Author

**Mohammad Hafeez**

Embedded Systems Learner\
NXP LPC2148 \| ARM7 \| Embedded C \| RTC \| GPIO \| Keypad \| LCD
