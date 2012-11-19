#include "avr-fixed.h"

FIXED1616 fixed_mult(FIXED1616 inArg0, FIXED1616 inArg1) {
  register unsigned long _a = (inArg0 >= 0) ? inArg0 : (-inArg0);
  register unsigned long _b = (inArg1 >= 0) ? inArg1 : (-inArg1);
  unsigned char needsflipsign = (inArg0 >= 0) != (inArg1 >= 0);

  register FIXED1616 result;

  asm volatile(

    // This algorithm works by multiplying bytes
    // together in a way that is similar to manual
    // long division.
    // Basically it starts on the most significant
    // bytes in the integer part of the fixed
    // value (the D byte) and shifts the result
    // to the left as it goes, then shifts it back.

    // Clear result.
    "eor %A[result], %A[result]"                "\n\t"
    "eor %B[result], %B[result]"                "\n\t"
    "eor %C[result], %C[result]"                "\n\t"
    "eor %D[result], %D[result]"                "\n\t"

    // Clear r18 through r22.
    "eor r18, r18"                              "\n\t"
    "eor r19, r19"                              "\n\t"
    "eor r20, r20"                              "\n\t"
    "eor r21, r21"                              "\n\t"
    "eor r22, r22"                              "\n\t"

    //// i = 5 ///////////////////////////////////////
    // The result from these multiplications end up in
    // the high bit of the integer part of the result.
    // Note that if any bits are simultaneously set in
    // argA.d and argB.d, the result overflows, so
    // that multiplication isn't performed.
    // Note that the multiplication can still overflow
    // from large values in the C bytes, so it isn't
    // easy to check for overflow using the D bytes.

    // result.b:result.a = argA.c * argB.d
    "mul %C[argA], %D[argB]"                    "\n\t"
    "mov %A[result], r0"                        "\n\t"
    "mov %B[result], r1"                        "\n\t"

    // result.c:result.b:result.a += argA.d * argB.c
    "mul %D[argA], %C[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result: d, c, b = c, b, a
    // (shift result left by 1 byte)
    "mov %D[result], %C[result]"                "\n\t"
    "mov %C[result], %B[result]"                "\n\t"
    "mov %B[result], %A[result]"                "\n\t"
    // These mov's can be eliminated by changing the
    // target registers above.
    // They were originally present to help checking
    // for overflow, by repeatedly checking the
    // D byte.

    // Clear result.a
    "mov %A[result], r22"                       "\n\t"

    //// i = 4 ///////////////////////////////////////

    // result.c:result.b:result.a += argA.b * argB.d
    "mul %B[argA], %D[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result.c:result.b:result.a += argA.c * argB.c
    "mul %C[argA], %C[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result.c:result.b:result.a += argA.d * argB.b
    "mul %D[argA], %B[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result: d, c, b = c, b, a
    // (shift result left by 1 byte)
    "mov %D[result], %C[result]"                "\n\t"
    "mov %C[result], %B[result]"                "\n\t"
    "mov %B[result], %A[result]"                "\n\t"
    // TODO: this overwrites the old result D byte.
    // Why is it calculated at i = 5 then?

    // Clear result.a
    "mov %A[result], r22"                       "\n\t"

    //// i = 3 ///////////////////////////////////////

    // result.c:result.b:result.a += argA.a * argB.d
    "mul %A[argA], %D[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result.c:result.b:result.a += argA.b * argB.c
    "mul %B[argA], %C[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result.c:result.b:result.a += argA.c * argB.b
    "mul %C[argA], %B[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result.c:result.b:result.a += argA.d * argB.a
    "mul %D[argA], %A[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result: d, c, b = c, b, a
    // (shift result left by 1 byte)
    "mov %D[result], %C[result]"                "\n\t"
    "mov %C[result], %B[result]"                "\n\t"
    "mov %B[result], %A[result]"                "\n\t"
    // Once again, result.d is overwritten. Why?

    // Clear result.a
    "mov %A[result], r22"                       "\n\t"

    //// i = 2 ///////////////////////////////////////

    // result.c:result.b:result.a += argA.a * argB.c
    "mul %A[argA], %C[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result.c:result.b:result.a += argA.b * argB.b
    "mul %B[argA], %B[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // result.c:result.b:result.a += argA.c * argB.a
    "mul %C[argA], %A[argB]"                    "\n\t"
    "add %A[result], r0"                        "\n\t"
    "adc %B[result], r1"                        "\n\t"
    "adc %C[result], r22"                       "\n\t"

    // low is stored in r18-r21

    //// i = 1 ///////////////////////////////////////

    // r19:r18 = argA.a * argB.b
    "mul %A[argA], %B[argB]"                    "\n\t"
    "mov r18, r0"                               "\n\t"
    "mov r19, r1"                               "\n\t"

    // This can overflow ... why is the carry ignored?

    // r20:r19:r18 += argA.b * argB.a
    "mul %B[argA], %A[argB]"                    "\n\t"
    "add r18, r0"                               "\n\t"
    "adc r19, r1"                               "\n\t"
    "adc r20, r22"                              "\n\t"

    // r21, r20, r19, r18 = r20, r19, r18, r22 (0)
    // Shift low to the left (?) by one byte.
    "mov r21, r20"                              "\n\t"
    "mov r20, r19"                              "\n\t"
    "mov r19, r18"                              "\n\t"
    "mov r18, r22"                              "\n\t"

    //// i = 0 ///////////////////////////////////////

    // Here we multiply argA.a and argB.a, the result
    // of which always underflows, and is thus only
    // used for rounding.
    // The rounding method used is to add 0x80 to the
    // result of the multiplication, then add with
    // carry r22 (equal to zero) r22 to r20, which
    // is then added to result.A.

    // r20:r19:r18 += argA.a * argB.a
    "mul %A[argA], %A[argB]"                    "\n\t"
    "add r18, r0"                               "\n\t"
    "adc r19, r1"                               "\n\t"
    "adc r20, r22"                              "\n\t"

    // low += 0x8000;
    "ldi r22, 0x80"                             "\n\t"
    "add r19, r22"                              "\n\t"
    "eor r22, r22"                              "\n\t"
    "adc r20, r22"                              "\n\t"

    // mid += (low >> 16);
    "add %A[result], r20"                       "\n\t"
    "adc %B[result], r21"                       "\n\t"
    "adc %C[result], r22"                       "\n\t"
    "adc %D[result], r22"                       "\n\t"

    "eor r1, r1"                                "\n\t"

    : [result] "=&r"(result)
    : [argA] "r"(_a), [argB] "r"(_b)
    : "r18", "r19", "r20", "r21",   "r22"
  );

  // Figure out the sign of result
  if (needsflipsign) {
    result = -result;
  }

  return result;
}
