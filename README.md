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




# Python Host Script Explanation for USB2GPIO

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
  ```

### **`set_pin_low(device, pin)`**

This function sends the **CMD_CLR_ONE** command to set a specific pin LOW.

- **Parameters**:
  - `device`: The HID device object.
  - `pin`: The GPIO pin to be set LOW.

- **Usage**: This sets the specified pin to a LOW state.

- **Example**:
  ```python
  set_pin_low(device, 13)  # Sets pin 13 LOW
  ```

### **`pulse_pin(device, pin, duration_us)`**

This function sends the **CMD_PULSE_ONE** command, which pulses a specified pin HIGH for a given duration in microseconds. After the duration, it sets the pin LOW.

- **Parameters**:
  - `device`: The HID device object.
  - `pin`: The GPIO pin to pulse.
  - `duration_us`: The duration to pulse the pin HIGH in microseconds.

- **Usage**: This will send a timed pulse command to the device. The pin will go HIGH for the specified duration and then go LOW.

- **Example**:
  ```python
  pulse_pin(device, 13, 500)  # Pulses pin 13 for 500 microseconds
  ```

### **`group_set(device, pins_values)`**

This function sends the **CMD_GROUP_SET** command, which allows multiple pins to be set to either HIGH or LOW. It takes a list of tuples, where each tuple contains a pin number and a value (1 for HIGH, 0 for LOW).

- **Parameters**:
  - `device`: The HID device object.
  - `pins_values`: A list of tuples where each tuple has the pin number and its value (0 for LOW, 1 for HIGH).

- **Usage**: This command sets the specified pins to the specified values.

- **Example**:
  ```python
  group_set(device, [(5, 1), (6, 0)])  # Sets pin 5 HIGH and pin 6 LOW
  ```

### **`read_pins(device, pins)`**

This function sends the **CMD_READ_MULTI** command to read the state of multiple pins. The function returns the input states of the specified pins.

- **Parameters**:
  - `device`: The HID device object.
  - `pins`: A list of pins to read the states of.

- **Usage**: This command reads and returns the states of the specified pins.

- **Example**:
  ```python
  read_pins(device, [5, 6, 13])  # Reads the states of pins 5, 6, and 13
  ```

### **`combo_set_and_read(device, set_mask, read_mask)`**

This function sends the **CMD_COMBO** command, which combines setting the state of some pins and reading the state of others. It takes two masks: one for setting the pins and another for reading the pins.

- **Parameters**:
  - `device`: The HID device object.
  - `set_mask`: A 32-bit mask that specifies which pins to set.
  - `read_mask`: A 32-bit mask that specifies which pins to read.

- **Usage**: This command sets the specified pins (based on the `set_mask`) and reads the input states of the pins (based on the `read_mask`).

- **Example**:
  ```python
  combo_set_and_read(device, 0x20, 0x30)  # Sets pin 5 and reads the state of pin 6
  ```

---

## **3. Example Commands**

### **Setting Pins**

- To set pin 13 HIGH:
  ```python
  set_pin_high(device, 13)
  ```
- To set pin 13 LOW:
  ```python
  set_pin_low(device, 13)
  ```

### **Pulsing Pins**

- To pulse pin 13 for 500 microseconds:
  ```python
  pulse_pin(device, 13, 500)
  ```

### **Grouping Pin Sets**

- To set pin 5 HIGH and pin 6 LOW:
  ```python
  group_set(device, [(5, 1), (6, 0)])
  ```

### **Reading Pins**

- To read the states of pins 5, 6, and 13:
  ```python
  read_pins(device, [5, 6, 13])
  ```

### **Combo Commands**

- To set pins 5 and 6 HIGH and read pin 6:
  ```python
  combo_set_and_read(device, 0x20, 0x30)
  ```

---

## **4. Usage**

1. **Install the `hid` library**: Make sure you have the `hid` library installed using `pip install hid`.
2. **Run the script**: Execute the script and it will automatically connect to the USB HID device and perform the operations defined in the `main` function.
3. **Modify the example commands**: You can modify the `main()` function to test different commands by changing pin numbers and durations.

---

## **5. Conclusion**

This Python host script allows for easy control of GPIO pins on the nRF52840 using USB HID commands. It supports both simple pin control (set HIGH/LOW) and more advanced features like reading pin states, grouped pin updates, and timed pulses. The flexibility of HID commands provides a reliable and efficient way to interface with the device over USB.

