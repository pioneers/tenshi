#ifndef USB_H_
#define USB_H_

#include <stdint.h>

#include <LUFA/Drivers/USB/USB.h>

typedef struct {
  USB_Descriptor_Configuration_Header_t Config;

  // Generic HID Interface
  USB_Descriptor_Interface_t            HID_Interface;
  USB_HID_Descriptor_HID_t              HID_GenericHID;
  USB_Descriptor_Endpoint_t             HID_ReportINEndpoint;
} USB_Descriptor_Configuration_t;

extern uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
    const uint8_t wIndex, const void **const DescriptorAddress,
    uint8_t *const DescriptorMemorySpace);

void EVENT_USB_Device_ControlRequest(void);

#endif
