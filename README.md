# nRF52_HID_GPIO
USB2GPIO Pro is a command-driven, ultra-low-latency USB HID device

# USB2GPIO Pro – User Manual  
**Version 1.0** | For nRF52840 with Arduino & TinyUSB

## Overview

**USB2GPIO Pro** is a command-driven, ultra-low-latency USB HID device that provides:
- High-speed **GPIO output control**
- Simultaneous **GPIO input reading**
- **Microsecond-precision timestamps**
- USB HID communication (driverless on all OSes)

Ideal for digital signal triggering, automation, timing-sensitive GPIO control, and lab interfaces.

## How It Works

- **USB HID OUT reports** (64 bytes) from the host to the device.
- Replies via **USB HID IN reports** when required (for input/timestamp feedback).
- Every command begins with a **1-byte command code**, followed by command-specific payload.

The device processes HID commands using direct register access (`NRF_GPIO`) for maximum performance (sub-microsecond pin access).

## USB Report Format

### OUT Report (Host → Device)
- 64-byte HID output report
- Format: `[CMD, param1, param2, ...]`

### IN Report (Device → Host)
- 64-byte HID input report (when needed)
- Format: `[RESP_STATUS, timestamp (4B), data...]`

## Supported Commands

| Command | Code | Direction | Description |
|---------|------|-----------|-------------|
| `CMD_SET_ONE` | `0x10` | OUT | Set single pin HIGH |
| `CMD_CLR_ONE` | `0x11` | OUT | Set single pin LOW |
| `CMD_GROUP_SET` | `0x20` | OUT | Set multiple pins HIGH/LOW |
| `CMD_READ_MULTI` | `0x21` | OUT + IN | Read multiple input pins with timestamp |
| `CMD_COMBO` | `0x22` | OUT + IN | Set multiple outputs + read pin states + timestamp |
| `CMD_PULSE_ONE` | `0x30` | OUT | Timed Pulse on pin |
| `RESP_STATUS` | `0xA1` | IN | Response to read/combo commands |

### CMD_SET_ONE (`0x10`)
- **Purpose:** Set a single GPIO pin HIGH
- **Format:** `[0x10, pin_number]`
- **Response:** None

### CMD_CLR_ONE (`0x11`)
- **Purpose:** Set a single GPIO pin LOW
- **Format:** `[0x11, pin_number]`
- **Response:** None

### CMD_GROUP_SET (`0x20`)
- **Purpose:** Set multiple pins HIGH or LOW
- **Format:** `[0x20, count, pin1, val1, pin2, val2, ...]`
  - `count`: Number of pin/value pairs
  - `val`: 1 = HIGH, 0 = LOW
- **Response:** None

### CMD_READ_MULTI (`0x21`)
- **Purpose:** Read multiple GPIO input pins + return timestamp
- **Format:** `[0x21, count, pin1, pin2, ...]`
- **Response:**

| Byte | Meaning              |
|------|----------------------|
| 0    | `0xA1` (RESP_STATUS) |
| 1–4  | Timestamp (`micros()`) |
| 5+   | Pin states (1 byte each) |

### CMD_COMBO (`0x22`)
- **Purpose:** Set output mask + read input mask + timestamp
- **Format (9 bytes):**
  `[0x22, set_mask(4B), read_mask(4B)]`
  - `set_mask`: GPIO bits to set HIGH
  - `read_mask`: GPIO bits to read (entire 32-bit mask allowed)
- **Response:**

| Byte | Meaning              |
|------|----------------------|
| 0    | `0xA1` (RESP_STATUS) |
| 1–4  | Timestamp (`micros()`) |
| 5–8  | GPIO input state (`NRF_GPIO->IN`) |

### CMD_PULSE_ONE (`0x30`)
- **Purpose:** Set a pin HIGH for an exact duration, then LOW
- **Format:** `[0x30, pin, duration (4B little-endian)]`
  - Pin is set HIGH immediately
  - Duration in microseconds (e.g., `500` = 0.5 ms)
  - After delay, pin is set LOW automatically
- **Response:** None

**Example:**
`[0x30, 13, 0xF4, 0x01, 0x00, 0x00]` → Pulse pin 13 for **500 µs**

### Use Case:
Generate logic triggers, strobes, or timing pulses from host software.

## Timing & Latency

| Stage                    | Time (typical)       |
|--------------------------|----------------------|


# Python Host Script Explanation for USB2GPIO (Timed Pulse Command)

This document explains the structure and functionality of the Python Host Script that communicates with an **nRF52840** USB device to control GPIO pins using HID commands. The script includes features such as setting pins HIGH/LOW, reading multiple pins, grouping pin updates, and a **timed pulse** command.

## **1. Script Overview**

The Python script communicates with a USB device running a custom HID interface. The host sends HID reports to the device, which in turn processes them and controls GPIO pins based on the commands received.

### **Supported Commands:**
- **CMD_SET_ONE**: Set a pin HIGH.
- **CMD_CLR_ONE**: Set a pin LOW.
- **CMD_GROUP_SET**: Set multiple pins to HIGH or LOW.
- **CMD_READ_MULTI**: Read the state of multiple pins.
- **CMD_COMBO**: Set some pins and read others in a single command.
- **CMD_PULSE_ONE**: Pulse a pin HIGH for a specific duration in microseconds.

---

## **2. Script Functions**

### **`send_command(device, cmd, data)`**

This function sends a command to the USB device and waits for a response. It prepares the report by combining the command and data, sends it to the device, and then reads the response.

- **Parameters:**
  - `device`: The HID device object to send the command to.
  - `cmd`: The command to be sent to the device.
  - `data`: The data associated with the command.

- **Returns**: The device’s response to the command.

### **`set_pin_high(device, pin)`**

This function sends the **CMD_SET_ONE** command to set a specific pin HIGH.

- **Parameters**:
  - `device`: The HID device object.
  - `pin`: The GPIO pin to be set HIGH.

- **Usage**: This sets the specified pin to a HIGH state.

- **Example**:
  ```python
  set_pin_high(device, 13)  # Sets pin 13 HIGH

