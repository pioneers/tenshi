#include "usb.h"

#include <avr/pgmspace.h>
#include <util/atomic.h>

#include "i2c_register.h"

const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
  .Header = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

  .USBSpecification       = VERSION_BCD(01.10),
  .Class                  = USB_CSCP_NoDeviceClass,
  .SubClass               = USB_CSCP_NoDeviceSubclass,
  .Protocol               = USB_CSCP_NoDeviceProtocol,

  .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

  .VendorID               = 0x03EB,
  .ProductID              = 0x204F,
  .ReleaseNumber          = VERSION_BCD(00.01),

  .ManufacturerStrIndex   = 0x01,
  .ProductStrIndex        = 0x02,
  .SerialNumStrIndex      = NO_DESCRIPTOR,

  .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

#define GENERIC_REPORT_SIZE       16
#define GENERIC_IN_EPADDR         (ENDPOINT_DIR_IN  | 1)
#define GENERIC_EPSIZE            8

const USB_Descriptor_HIDReport_Datatype_t PROGMEM GenericReport[] = {
  HID_RI_USAGE_PAGE(16, 0xFF00), /* Vendor Page 0 */
  HID_RI_USAGE(8, 0x01), /* Vendor Usage 1 */
  HID_RI_COLLECTION(8, 0x01), /* Vendor Usage 1 */
    HID_RI_USAGE(8, 0x00), /* Undefined */
    HID_RI_LOGICAL_MINIMUM(8, 0x00),
    HID_RI_LOGICAL_MAXIMUM(8, 0xFF),
    HID_RI_REPORT_SIZE(8, 0x08),
    HID_RI_REPORT_COUNT(8, GENERIC_REPORT_SIZE),
    HID_RI_FEATURE(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
  HID_RI_END_COLLECTION(0),
};

const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
  .Config =
  {
    .Header = {
      .Size = sizeof(USB_Descriptor_Configuration_Header_t),
      .Type = DTYPE_Configuration
    },

    .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
    .TotalInterfaces        = 1,

    .ConfigurationNumber    = 1,
    .ConfigurationStrIndex  = NO_DESCRIPTOR,

    .ConfigAttributes       =
    (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),

    .MaxPowerConsumption    = USB_CONFIG_POWER_MA(0)
  },

  .HID_Interface =
  {
    .Header = {
      .Size = sizeof(USB_Descriptor_Interface_t),
      .Type = DTYPE_Interface
    },

    .InterfaceNumber        = 0x00,
    .AlternateSetting       = 0x00,

    .TotalEndpoints         = 1,

    .Class                  = HID_CSCP_HIDClass,
    .SubClass               = HID_CSCP_NonBootSubclass,
    .Protocol               = HID_CSCP_NonBootProtocol,

    .InterfaceStrIndex      = NO_DESCRIPTOR
  },

  .HID_GenericHID =
  {
    .Header = {
      .Size = sizeof(USB_HID_Descriptor_HID_t),
      .Type = HID_DTYPE_HID
    },

    .HIDSpec                = VERSION_BCD(01.11),
    .CountryCode            = 0x00,
    .TotalReportDescriptors = 1,
    .HIDReportType          = HID_DTYPE_Report,
    .HIDReportLength        = sizeof(GenericReport)
  },

  .HID_ReportINEndpoint =
  {
    .Header = {
      .Size = sizeof(USB_Descriptor_Endpoint_t),
      .Type = DTYPE_Endpoint
    },

    .EndpointAddress        = GENERIC_IN_EPADDR,
    .Attributes             =
    (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
    .EndpointSize           = GENERIC_EPSIZE,
    .PollingIntervalMS      = 0x05
  },
};

const USB_Descriptor_String_t PROGMEM LanguageString = {
  .Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},

  .UnicodeString          = {LANGUAGE_ID_ENG}
};

const USB_Descriptor_String_t PROGMEM ManufacturerString = {
  .Header                 = {.Size = USB_STRING_LEN(23), .Type = DTYPE_String},

  .UnicodeString          = L"Pioneers in Engineering"
};

const USB_Descriptor_String_t PROGMEM ProductString = {
  .Header                 = {.Size = USB_STRING_LEN(29), .Type = DTYPE_String},

  .UnicodeString          = L"Grizzly Bear Motor Controller"
};

extern uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
    const uint8_t wIndex, const void **const DescriptorAddress,
    uint8_t *const DescriptorMemorySpace) {
  const uint8_t  DescriptorType   = (wValue >> 8);
  const uint8_t  DescriptorNumber = (wValue & 0xFF);

  const void *Address = NULL;
  uint16_t    Size    = NO_DESCRIPTOR;
  uint8_t     MemorySpace;

  switch (DescriptorType) {
    case DTYPE_Device:
      Address = &DeviceDescriptor;
      Size    = sizeof(USB_Descriptor_Device_t);
      MemorySpace = MEMSPACE_FLASH;
      break;
    case DTYPE_Configuration:
      Address = &ConfigurationDescriptor;
      Size    = sizeof(USB_Descriptor_Configuration_t);
      MemorySpace = MEMSPACE_FLASH;
      break;
    case DTYPE_String:
      switch (DescriptorNumber) {
        case 0x00:
          Address = &LanguageString;
          Size    = pgm_read_byte(&LanguageString.Header.Size);
          MemorySpace = MEMSPACE_FLASH;
          break;
        case 0x01:
          Address = &ManufacturerString;
          Size    = pgm_read_byte(&ManufacturerString.Header.Size);
          MemorySpace = MEMSPACE_FLASH;
          break;
        case 0x02:
          Address = &ProductString;
          Size    = pgm_read_byte(&ProductString.Header.Size);
          MemorySpace = MEMSPACE_FLASH;
          break;
      }

      break;
    case HID_DTYPE_HID:
      Address = &ConfigurationDescriptor.HID_GenericHID;
      Size    = sizeof(USB_HID_Descriptor_HID_t);
      MemorySpace = MEMSPACE_FLASH;
      break;
    case HID_DTYPE_Report:
      Address = &GenericReport;
      Size    = sizeof(GenericReport);
      MemorySpace = MEMSPACE_FLASH;
      break;
  }

  *DescriptorAddress = Address;
  *DescriptorMemorySpace = MemorySpace;
  return Size;
}

// This buffer will be sent to the host as a reply to a request
uint8_t replyBuf[GENERIC_REPORT_SIZE];

/* The layout of requests is: 
    Byte 0:           Register
    Byte 1 (bit 6:0): Length of data to read/write
    Byte 1 (bit 7):   R/W flag (1 = write)
    Byte 2-15:        Data
*/
/* The layout of replies is:
    Byte 0:     Length of data read
    Byte 1-15:  Data
*/

void handleUsbRequest(uint8_t *buf) {
  uint8_t reg = buf[0];
  uint8_t len = buf[1] & 0x7F;
  uint8_t rw = buf[1] & 0x80;
  if (rw) {
    // Write request
    if (len > GENERIC_REPORT_SIZE - 2)
      len = GENERIC_REPORT_SIZE - 2;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for (uint8_t i = 0; i < len; i++) {
          set_i2c_reg(reg + i, buf[i + 2]);
        }
      }
  }
  else {
    // Read request
    if (len > GENERIC_REPORT_SIZE - 1)
      len = GENERIC_REPORT_SIZE - 1;
    replyBuf[0] = len;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      for (uint8_t i = 0; i < len; i++) {
        replyBuf[i + 1] = provide_i2c_reg(reg + i);
      }
    }
  }
}

// We cheat and don't handle the interrupt endpoints at all.
// Only the control endpoint can be used.
void EVENT_USB_Device_ControlRequest(void) {
  /* Handle HID Class specific requests */
  switch (USB_ControlRequest.bRequest) {
    case HID_REQ_GetReport:
      if (USB_ControlRequest.bmRequestType ==
          (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
        Endpoint_ClearSETUP();

        /* Write the report data to the control endpoint */
        Endpoint_Write_Control_Stream_LE(&replyBuf, sizeof(replyBuf));
        Endpoint_ClearOUT();
      }

      break;
    case HID_REQ_SetReport:
      if (USB_ControlRequest.bmRequestType ==
          (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
        uint8_t GenericData[GENERIC_REPORT_SIZE];

        Endpoint_ClearSETUP();

        /* Read the report data from the control endpoint */
        Endpoint_Read_Control_Stream_LE(&GenericData, sizeof(GenericData));
        Endpoint_ClearIN();

        handleUsbRequest(GenericData);
      }

      break;
  }
}
