// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#ifndef INC_USB_H_
#define INC_USB_H_

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

#endif  // INC_USB_H_
