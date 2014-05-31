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

#ifndef INC_ADDR_JUMPER_H_
#define INC_ADDR_JUMPER_H_
// This file contains code for the I2C address jumpers.

#define ADDR_COMBO_1  (0x09 << 1)
#define ADDR_COMBO_2  (0x0A << 1)
#define ADDR_COMBO_3  (0x0B << 1)
#define ADDR_COMBO_4  (0x0C << 1)
#define ADDR_COMBO_5  (0x0D << 1)
#define ADDR_COMBO_6  (0x0E << 1)
#define ADDR_COMBO_7  (0x0F << 1)

// Called to read jumpers. Returns an I2C address suitable for TWAR.
unsigned char determine_addr(void);

#endif  // INC_ADDR_JUMPER_H_
