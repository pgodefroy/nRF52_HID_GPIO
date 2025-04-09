import time
import struct
import hid

# USB Vendor and Product ID (replace with your own values if needed)
VENDOR_ID = 0x239A
PRODUCT_ID = 0x8029

# Command codes
CMD_SET_ONE = 0x10
CMD_CLR_ONE = 0x11
CMD_GROUP_SET = 0x20
CMD_READ_MULTI = 0x21
CMD_COMBO = 0x22
CMD_PULSE_ONE = 0x30  # New command for timed pulses

RESP_STATUS = 0xA1

# Function to send a command to the USB device
def send_command(device, cmd, data):
    report = [cmd] + data
    device.write(report)
    time.sleep(0.01)  # Small delay to allow processing time
    return device.read(64)  # Read the response from the device

# Function to set a pin HIGH
def set_pin_high(device, pin):
    response = send_command(device, CMD_SET_ONE, [pin])
    print(f"Set pin {pin} HIGH. Response: {response}")

# Function to set a pin LOW
def set_pin_low(device, pin):
    response = send_command(device, CMD_CLR_ONE, [pin])
    print(f"Set pin {pin} LOW. Response: {response}")

# Function to pulse a pin for a specific duration (in microseconds)
def pulse_pin(device, pin, duration_us):
    # Pack the data (pin number + duration in microseconds)
    pulse_data = [pin] + list(struct.unpack("<4B", struct.pack("<I", duration_us)))  # Convert duration to bytes
    response = send_command(device, CMD_PULSE_ONE, pulse_data)
    print(f"Pulsed pin {pin} for {duration_us} microseconds. Response: {response}")

# Function to set multiple pins to HIGH/LOW
def group_set(device, pins_values):
    # pins_values should be a list of tuples (pin, value), where value is 1 (HIGH) or 0 (LOW)
    data = [len(pins_values)] + [item for sublist in pins_values for item in sublist]
    response = send_command(device, CMD_GROUP_SET, data)
    print(f"Group set response: {response}")

# Function to read multiple pins
def read_pins(device, pins):
    response = send_command(device, CMD_READ_MULTI, [len(pins)] + pins)
    print(f"Read pins {pins}. Response: {response}")

# Function to combine set and read GPIO operations
def combo_set_and_read(device, set_mask, read_mask):
    # Pack the masks into the data
    data = list(struct.unpack("<4B", struct.pack("<I", set_mask))) + list(struct.unpack("<4B", struct.pack("<I", read_mask)))
    response = send_command(device, CMD_COMBO, data)
    print(f"Combo set and read response: {response}")

# Find and open the USB HID device
def open_device():
    device = None
    for dev in hid.enumerate():
        if dev['vendor_id'] == VENDOR_ID and dev['product_id'] == PRODUCT_ID:
            device = hid.Device(dev['vendor_id'], dev['product_id'])
            break
    if device is None:
        print("Device not found!")
        return None
    print("Device opened successfully")
    return device

def main():
    # Open the USB device
    device = open_device()
    if device is None:
        return

    # Example commands

    # Set pin 13 HIGH
    set_pin_high(device, 13)

    # Set pin 13 LOW
    set_pin_low(device, 13)

    # Pulse pin 13 for 500 microseconds
    pulse_pin(device, 13, 500)

    # Group set pins 5 (HIGH), 6 (LOW)
    group_set(device, [(5, 1), (6, 0)])

    # Read pins 5, 6, 13
    read_pins(device, [5, 6, 13])

    # Combo: Set pin 5 HIGH, 6 LOW and read the input state for pins 5 and 6
    combo_set_and_read(device, 0x20, 0x30)  # Example: Set pin 5 (0x20), 6 (0x30)

    device.close()

if __name__ == "__main__":
    main()
