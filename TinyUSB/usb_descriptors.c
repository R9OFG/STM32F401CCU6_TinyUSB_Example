/*
 * usb_descriptors.c
 *
 * Author: R9OFG.RU
 *
 * MCU: STM32F401CCU6
 * Составное устройство: CDC + UAC1 (радиоприемник)
 */

#include "tusb.h"
#include <string.h>

//--------------------------------------------------------------------+
// Device Descriptor (с IAD)
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0xEF,     // Miscellaneous Device
    .bDeviceSubClass    = 0x02,     // Common Class
    .bDeviceProtocol    = 0x01,     // IAD present
    .bMaxPacketSize0    = 64,
    .idVendor           = 0xCafe,
    .idProduct          = 0x4003,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor (183 bytes total)
//--------------------------------------------------------------------+
uint8_t const desc_configuration[] =
{
  // Configuration descriptor (9 bytes)
  0x09, 0x02, 183, 0x00, 0x04, 0x01, 0x00, 0x80, 0x32,

  // IAD для CDC (8 bytes)
  0x08, 0x0B, 0x00, 0x02, 0x02, 0x02, 0x01, 0x04,

  // CDC Control Interface (Interface 0)
  0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x04,
  0x05, 0x24, 0x00, 0x10, 0x01,
  0x05, 0x24, 0x01, 0x00, 0x01,
  0x04, 0x24, 0x02, 0x02,
  0x05, 0x24, 0x06, 0x00, 0x01,
  0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x10,

  // CDC Data Interface (Interface 1)
  0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x04,
  0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,
  0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00,

  // IAD для аудио (8 bytes)
  0x08, 0x0B, 0x02, 0x02, 0x01, 0x01, 0x00, 0x05,

  //------------------------------------------------------------------
  // Audio Control Interface (Interface 2)
  //------------------------------------------------------------------

  0x09, 0x04, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x05,

  // Class-Specific AC Interface Header
  // bcdADC=1.00
  // wTotalLength=39 (0x27)
  // bInCollection=1
  // baInterfaceNr=3 (Audio Streaming)
  0x09, 0x24, 0x01, 0x00, 0x01, 0x27, 0x00, 0x01, 0x03,

  // Input Terminal (ID=1, type=0x0710 Radio Receiver)
  0x0C, 0x24, 0x02,
  0x01,             // bTerminalID
  0x10, 0x07,       // wTerminalType = Radio Receiver (0x0710)
  0x00,             // bAssocTerminal
  0x02,             // bNrChannels (Stereo)
  0x03, 0x00,       // wChannelConfig (Left Front | Right Front)
  0x00,             // iChannelNames
  0x00,             // iTerminal

  // Feature Unit (ID=3, source=1)
  0x09, 0x24, 0x06,
  0x03,             // bUnitID
  0x01,             // bSourceID = Input Terminal
  0x01,             // bControlSize
  0x01,             // bmaControls(0) Mute
  0x00,             // bmaControls(1)
  0x00,             // iFeature

  // Output Terminal (ID=2, type=USB Streaming)
  // bSourceID = 3 (Feature Unit)
  0x09, 0x24, 0x03,
  0x02,             // bTerminalID
  0x01, 0x01,       // wTerminalType = USB Streaming
  0x00,             // bAssocTerminal
  0x03,             // bSourceID = Feature Unit (FIX)
  0x00,             // iTerminal

  //------------------------------------------------------------------
  // Audio Streaming Interface
  //------------------------------------------------------------------

  // Zero Bandwidth (Alt 0)
  0x09, 0x04, 0x03, 0x00, 0x00, 0x01, 0x02, 0x00, 0x05,

  // Operational (Alt 1)
  0x09, 0x04, 0x03, 0x01, 0x01, 0x01, 0x02, 0x00, 0x05,

  // AS General (bTerminalLink=2)
  0x07, 0x24, 0x01,
  0x02,             // Output Terminal ID
  0x01,
  0x01,
  0x00,

  // Format Type I
  // Stereo, 16-bit, 48kHz
  0x0B, 0x24, 0x02,
  0x01,             // FORMAT_TYPE_I
  0x02,             // bNrChannels
  0x02,             // SubframeSize (2 bytes)
  0x10,             // BitResolution (16 bit)
  0x01,             // bSamFreqType
  0x80, 0xBB, 0x00, // 48000 Hz

  // Isochronous IN Endpoint (192 bytes)
  0x09, 0x05,
  0x83,             // EP 3 IN
  0x05,             // Isochronous, Async
  0xC0, 0x00,       // wMaxPacketSize = 192
  0x01,             // bInterval = 1 ms
  0x00,
  0x00,

  // Class-Specific Endpoint
  0x07, 0x25, 0x01,
  0x00,
  0x00,
  0x00,
  0x00
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index;
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
static char const* string_desc_arr[] =
{
  (const char[]) { 0x09, 0x04 },  // 0: Language (English)
  "R9OFG",                        // 1: Manufacturer
  "SDR_DEV",                      // 2: Product
  "1234567890",                   // 3: Serial
  "SDR_DEV_CDC_Interface",        // 4: CDC Interface
  "SDR_DEV_UAC1_Interface"        // 5: Audio Interface
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  uint8_t chr_count;

  if (index == 0)
  {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }
  else
  {
    if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
      return NULL;

    const char* str = string_desc_arr[index];
    chr_count = (uint8_t) strlen(str);
    if (chr_count > 31) chr_count = 31;

    for (uint8_t i = 0; i < chr_count; i++)
    {
      _desc_str[1 + i] = str[i];
    }
  }

  _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
  return _desc_str;
}
