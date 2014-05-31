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

#ifndef ENCODER_H_
#define ENCODER_H_
// This file contains code for the wheel encoders.

#include <stdint.h>

#include "util_macro.h"

// Number of counts the encoder has rotated.
DECLARE_I2C_REGISTER(int32_t, encoder_count);

// Called to configure encoder hardware on startup.
extern void init_encoder();

#endif  // ENCODER_H_
