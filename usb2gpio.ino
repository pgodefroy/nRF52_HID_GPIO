#include "Adafruit_TinyUSB.h"

#define CMD_SET_ONE     0x10
#define CMD_CLR_ONE     0x11
#define CMD_GROUP_SET   0x20
#define CMD_READ_MULTI  0x21
#define CMD_COMBO       0x22
#define CMD_PULSE_ONE   0x30  // New Command
#define RESP_STATUS     0xA1

Adafruit_USBD_HID usb_hid;
uint8_t const hid_report_descriptor[] = {
  TUD_HID_REPORT_DESC_GENERIC_INOUT(64)
};

uint8_t in_report[64];  // IN response buffer

void setup() {
  usb_hid.setReportDescriptor(hid_report_descriptor, sizeof(hid_report_descriptor)); // Set report descriptor
  usb_hid.begin();  // Initialize TinyUSB

  TinyUSBDevice.begin();

  // Optional: pre-configure known pins
  for (int i = 0; i < 32; i++) {
    NRF_GPIO->DIRSET = (1 << i);  // Default to OUTPUT
  }
}

void loop() {
  // Everything handled via interrupts
}

void tud_hid_set_report_cb(uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer,
                           uint16_t bufsize)
{
  if (bufsize < 1) return;

  uint8_t cmd = buffer[0];

  // Set single pin HIGH
  if (cmd == CMD_SET_ONE && bufsize >= 2) {
    NRF_GPIO->DIRSET = (1 << buffer[1]);
    NRF_GPIO->OUTSET = (1 << buffer[1]);
  }

  // Set single pin LOW
  else if (cmd == CMD_CLR_ONE && bufsize >= 2) {
    NRF_GPIO->DIRSET = (1 << buffer[1]);
    NRF_GPIO->OUTCLR = (1 << buffer[1]);
  }

  // Set multiple pins HIGH/LOW
  else if (cmd == CMD_GROUP_SET && bufsize >= 3) {
    uint8_t count = buffer[1];
    for (uint8_t i = 0; i < count && (2 + i*2 + 1) < bufsize; ++i) {
      uint8_t pin = buffer[2 + i*2];
      uint8_t val = buffer[3 + i*2];
      NRF_GPIO->DIRSET = (1 << pin);
      if (val)
        NRF_GPIO->OUTSET = (1 << pin);
      else
        NRF_GPIO->OUTCLR = (1 << pin);
    }
  }

  // Read multiple GPIO pins
  else if (cmd == CMD_READ_MULTI && bufsize >= 2) {
    uint8_t count = buffer[1];
    in_report[0] = RESP_STATUS;
    uint32_t ts = micros();
    memcpy(&in_report[1], &ts, 4);

    for (uint8_t i = 0; i < count && (2 + i) < bufsize; ++i) {
      uint8_t pin = buffer[2 + i];
      in_report[5 + i] = (NRF_GPIO->IN & (1 << pin)) ? 1 : 0;
    }

    usb_hid.sendReport(0, in_report, 5 + count);
  }

  // Set output mask + read input mask + timestamp
  else if (cmd == CMD_COMBO && bufsize >= 9) {
    uint32_t set_mask  = *((uint32_t*)&buffer[1]);
    uint32_t read_mask = *((uint32_t*)&buffer[5]);

    NRF_GPIO->OUTSET = set_mask;

    uint32_t pin_states = NRF_GPIO->IN;

    in_report[0] = RESP_STATUS;
    uint32_t ts = micros();
    memcpy(&in_report[1], &ts, 4);
    memcpy(&in_report[5], &pin_states, 4);

    usb_hid.sendReport(0, in_report, 9);
  }

  // Timed Pulse Command
  else if (cmd == CMD_PULSE_ONE && bufsize >= 6) {
    uint8_t pin = buffer[1];
    uint32_t duration = *((uint32_t*)&buffer[2]);

    NRF_GPIO->DIRSET = (1 << pin);
    NRF_GPIO->OUTSET = (1 << pin);
    delayMicroseconds(duration);  // Pulse duration
    NRF_GPIO->OUTCLR = (1 << pin);  // Set pin LOW after duration
  }
}
