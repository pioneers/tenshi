#ifndef UTIL_MACRO_H_
#define UTIL_MACRO_H_
// This file contains various macros that are used repeatedly.
#include <util/atomic.h>

// This macro is used in header files to declare variables intended to be
// accessed as I2C registers. These will require ATOMIC_BLOCK.
// This macro MUST be used with the following _C macro.
// This macro creates 5 functions
//   <type> get_<name>(void)
//   void set_<name>(<type> value)
//   <type> get_<name>_dangerous(void)
//   void set_<name>_dangerous(<type> value)
//   <type> *get_<name>_addr(void)
// The _dangerous versions do not do ATOMIC_BLOCK. They are only
// allowed to be used if interrupts ARE NOT enabled or in an ISR.
// The non _dangerous versions use ATOMIC_FORCEON, so they may only
// be used if interrupts ARE enabled.
// get_<name>_addr returns the address of the actual variable.
// This may only be used when interrupts ARE NOT enabled or in an ISR.
#define DECLARE_I2C_REGISTER(type, name)                  \
  extern type __##name;                                   \
  static inline type get_##name(void) {                   \
    ATOMIC_BLOCK(ATOMIC_FORCEON) {                        \
      return __##name;                                    \
    }                                                     \
    __builtin_unreachable();                              \
  }                                                       \
  static inline type get_##name##_dangerous(void) {       \
    return __##name;                                      \
  }                                                       \
  static inline void set_##name(type value) {             \
    ATOMIC_BLOCK(ATOMIC_FORCEON) {                        \
      __##name = value;                                   \
    }                                                     \
  }                                                       \
  static inline void set_##name##_dangerous(type value) { \
    __##name = value;                                     \
  }                                                       \
  static inline type *get_##name##_addr(void) {           \
    return &__##name;                                     \
  }

// This following macro is used in the .c file. Use it along with the above.
#define DECLARE_I2C_REGISTER_C(type, name) type __##name;

#endif
