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

// Contains defines and declarations for the embedded version number
// information. This information is automatically inserted into the binary by
// the build system.

#ifndef INC_VERSION_H_
#define INC_VERSION_H_

#include <stdint.h>

// The structure of this field is as follows:
//   20 bytes git hash
//    2 bytes major version
//    2 bytes minor version
//    2 bytes build number
//    2 bytes patch number
//    1 byte flags
//    3 bytes magic (used for setting signed-off flag)
extern const uint8_t VERSION_INFORMATION[32];

#define VERSION_GIT_HASH_OFFSET     0
#define VERSION_MAJOR_OFFSET        20
#define VERSION_MINOR_OFFSET        22
#define VERSION_BUILD_OFFSET        24
#define VERSION_PATCH_OFFSET        26
#define VERSION_FLAGS_OFFSET        28

// This is a debug build. DEBUG is defined.
#define VERSION_FLAG_DEBUG              0b00000001
// This build was built by the Jenkins -committed project.
#define VERSION_FLAG_CI_BUILD           0b00000010
// This build is from a dirty working directory.
#define VERSION_FLAG_DIRTY_WORKING_DIR  0b00000100
// This build has been approved by the project maintainers.
#define VERSION_FLAG_SIGNED_OFF_BUILD   0b10000000

#endif  // INC_VERSION_H_
