#ifndef AVR_FIXED_H_
#define AVR_FIXED_H_
// This file contains code for fixed point numbers.
// The code is not the most optimal but is less stupid than gcc's output.
// NOTE: Inherited from some other of Robert Ou's projects.
// NOTE: Derived from libfixmath.

typedef signed long FIXED1616;

static inline FIXED1616 int_to_fixed(int i) {
  return ((FIXED1616)i) << 16;
}

static inline int fixed_to_int(FIXED1616 i) {
  return i >> 16;
}

extern FIXED1616 fixed_mult(FIXED1616 inArg0, FIXED1616 inArg1);

#endif  // AVR_FIXED_H_
