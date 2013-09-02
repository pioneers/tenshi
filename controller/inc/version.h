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
