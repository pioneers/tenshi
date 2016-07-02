Angelic VM Spec
===============
# General
The Angelic VM is a stack machine. The primary implementation is in C, and it
is likely that large parts of this spec will not apply for any other VM (i.e.
the Javascript VM).

There are four value types which can be located on the stack, and no tracking of these types is done.
The types are object, int, float, and uint.
The following holds for all (signed) int values:

    i - 1 <= i
    i + 1 >= i

uint overflows as defined in the ISO C standard.
The size of these values is dependent on the VM, but is at least 32 bits.
In general, preforming operations intended for one type on another has undefined behavior (i.e. no checking is done).
However, objects carry type tags, so if the value is known to be an object, some checking and dynamic dispatch can be done.

The VM is not required to perform any garbage collection beyond reference counting.

# Stack
There are currently four stacks in the VM. Whenever a function is called, these stacks are checked to ensure they have enough space for the function being called. If any are not, the VM attempts to grow that stack.
The first stack is the main stack, which is used to hold values, and is directly available to the executing program.
The second stack is the function call stack, which is not directly accessible to the program, but is used in function calls and returns, exception handling, and code patching.
The third stack is the freestack, which holds stack locations where local variables might have references to objects. Note that unlike stack positions in opcodes, these locations are relative to the stack base, which is the address on the main stack of the first argument to the current function.
The fourth stack is the exception handling stack. It cannot be accessed directly, but can be indirectly pushed to and popped.

# Opcodes
Opcodes are 7 bit numbers mapping to particular functions in the Angelic VM.
The top (8th) bit is used to control value inspection in the C implementation.

## Opcode Loading and Evalutation
Opcodes are loaded in groups of 4 (32 bits) called "bunches." As opcodes are evaluated, the other opcodes are removed from the bunch, and 0s are added at the end. Opcodes may also consume some fixed number of bytes from the bunch, indicated in the rest of this documentation as `[opcode_name arg1 ... argn]`. Instructions should not rely on bytes of 0 being fed into the bunch as opcodes are evaluated (the compiler should not, for example, emit a `li_1` at the end of a bunch to load a 0). This implies that opcodes should not receive more than 3 bytes of arguments in this manner.
Opcodes may also remove whole bunches (multiples of 32 bits) from the opcode stream. This is indicated in the rest of this document as `[opcode_name] [arg]`. The number of bunches consumed in this way need not be bounded or fixed at a particular value per opcode.

If the top bit of an opcode is set, the evaluation of that opcode is logged in some VM specific way.
## Angelic VM Instruction Set
This is the number to opcode mapping.

<code>

  * 0x00 : next
  * 0x01 : dup_1
  * 0x02 : li_w
  * 0x03 : set_1
  * 0x04 : pop
  * 0x05 : call_1
  * 0x06 : ret
  * 0x07 : eq
  * 0x08 : j_1
  * 0x09 : j_2
  * 0x0a : j_3
  * 0x0b : j_w
  * 0x0c : bz_1
  * 0x0d : bz_2
  * 0x0e : bz_3
  * 0x0f : bz_w
  * 0x10 : not
  * 0x11 : f.add
  * 0x12 : f.sub
  * 0x13 : f.mul
  * 0x14 : f.div
  * 0x15 : f.mod
  * 0x16 : i.add
  * 0x17 : i.sub
  * 0x18 : i.mul
  * 0x19 : i.div
  * 0x1a : i.mod
  * 0x1b : u.add
  * 0x1c : u.sub
  * 0x1d : u.mul
  * 0x1e : u.div
  * 0x1f : u.mod
  * 0x20 : refi
  * 0x21 : refd
  * 0x22 : make_1
  * 0x23 : pushfree_1
  * 0x24 : popfree_1
  * 0x25 : clone
  * 0x26 : safe
  * 0x27 : read_1
  * 0x28 : write_1
  * 0x29 : stack_1
  * 0x2a : noop
  * 0x2b : end
  * 0x2c : debug
  * 0x2d : f2i
  * 0x2e : i2f
  * 0x2f : band
  * 0x30 : bor
  * 0x31 : bxor
  * 0x32 : bnot
  * 0x33 : bsl
  * 0x34 : bsrl
  * 0x35 : bsra
  * 0x36 : catch_w
  * 0x37 : throw
  * 0x38 : reserved
  * 0x39 : reserved
  * 0x3a : reserved
  * 0x3b : reserved
  * 0x3c : reserved
  * 0x3d : reserved
  * 0x3e : reserved
  * 0x3f : reserved
  * 0x40 : reserved
  * 0x41 : reserved
  * 0x42 : reserved
  * 0x43 : reserved
  * 0x44 : reserved
  * 0x45 : reserved
  * 0x46 : reserved
  * 0x47 : reserved
  * 0x48 : reserved
  * 0x49 : reserved
  * 0x4a : reserved
  * 0x4b : reserved
  * 0x4c : reserved
  * 0x4d : reserved
  * 0x4e : reserved
  * 0x4f : reserved
  * 0x50 : reserved
  * 0x51 : reserved
  * 0x52 : reserved
  * 0x53 : reserved
  * 0x54 : reserved
  * 0x55 : reserved
  * 0x56 : reserved
  * 0x57 : reserved
  * 0x58 : reserved
  * 0x59 : reserved
  * 0x5a : reserved
  * 0x5b : reserved
  * 0x5c : reserved
  * 0x5d : reserved
  * 0x5e : reserved
  * 0x5f : reserved
  * 0x60 : reserved
  * 0x61 : reserved
  * 0x62 : reserved
  * 0x63 : reserved
  * 0x64 : reserved
  * 0x65 : reserved
  * 0x66 : reserved
  * 0x67 : reserved
  * 0x68 : reserved
  * 0x69 : reserved
  * 0x6a : reserved
  * 0x6b : reserved
  * 0x6c : reserved
  * 0x6d : reserved
  * 0x6e : reserved
  * 0x6f : reserved
  * 0x70 : reserved
  * 0x71 : reserved
  * 0x72 : reserved
  * 0x73 : reserved
  * 0x74 : reserved
  * 0x75 : reserved
  * 0x76 : reserved
  * 0x77 : reserved
  * 0x78 : reserved
  * 0x79 : reserved
  * 0x7a : reserved
  * 0x7b : reserved
  * 0x7c : reserved
  * 0x7d : reserved
  * 0x7e : reserved
  * 0x7f : reserved

</code>

### [next]
Causes the next bunch of opcodes to be loaded. Any further opcodes in this bunch will be skipped.

### [dup_1 *idx*]
Duplicates the value *idx* places from the top of stack.

### [li_w] [*val*]
Causes *val* to be pushed to the top of the stack.

### [set_1 *idx*]
Causes the top value of the stack to be written into *idx* from the top of stack. Pops the top value from the stack.

### [pop]
Removes the top value from the stack.

### [call_1 *argc*]
Performs a function call, where the top *argc* arguments on the stack are arguments, and *argc* + 1 is the location of the function to call.

### [ret]
Return from a function call. Saves and restores the top value on the stack.
If there are no further functions to return to, stops the vm.

### [eq]
Pops the top two values on the stack, and pushes whether they are equal.

### [j_1 *relative_dest*]
Causes PC to be moved by *relative_dest*, a signed 8 bit integer.

### [j_2 *relative_dest0* *relative_dest1*]
Causes PC to be moved by *relative_dest0*:*relative_dest1*, a signed 16 bit integer.

### [j_3 *relative_dest0* *relative_dest1* *relative_dest2*]
Causes PC to be moved by *relative_dest0*:*relative_dest1*:*relative_dest2*, a signed 24 bit integer.

### [j_w] [*relative_dest*]
Causes PC to be moved by *relative_dest*, a signed 32 bit integer.

### [bz_1 *relative_dest*]
Pops the top value on the stack. If that value is zero, causes PC to be moved by *relative_dest*, a signed 8 bit integer.

### [bz_2 *relative_dest0* *relative_dest1*]
Pops the top value on the stack. If that value is zero, causes PC to be moved by *relative_dest0*:*relative_dest1*, a signed 16 bit integer.

### [bz_3 *relative_dest0* *relative_dest1* *relative_dest2*]
Pops the top value on the stack. If that value is zero, causes PC to be moved by *relative_dest0*:*relative_dest1*:*relative_dest2*, a signed 24 bit integer.

### [bz_w] [*relative_dest*]
Pops the top value on the stack. If that value is zero, causes PC to be moved by *relative_dest*, a signed 32 bit integer.

### [not]
Pops the top value on the stack. If it is 0, pushes 1. Otherwise pushes 0.

### [fadd]
Pops the top two values on the stack. Pushes the sum of these values.

### [fsub]
Pops the top two values on the stack. Pushes the difference of these values.

### [fmul]
Pops the top two values on the stack. Pushes the product of these values.

### [fdiv]
Pops the top two values on the stack. Pushes the quotient of these values.

### [fmod]
Pops the top two values on the stack. Pushes the first number reduced modulo the second.

### [iadd]
Pops the top two values on the stack. Pushes the sum of these values.

### [isub]
Pops the top two values on the stack. Pushes the difference of these values.

### [imul]
Pops the top two values on the stack. Pushes the product of these values.

### [idiv]
Pops the top two values on the stack. Pushes the quotient of these values.

### [imod]
Pops the top two values on the stack. Pushes the first number reduced modulo the second.

### [uadd]
Pops the top two values on the stack. Pushes the sum of these values.

### [usub]
Pops the top two values on the stack. Pushes the difference of these values.

### [umul]
Pops the top two values on the stack. Pushes the product of these values.

### [udiv]
Pops the top two values on the stack. Pushes the quotient of these values.

### [umod]
Pops the top two values on the stack. Pushes the first number reduced modulo the second.

### [refi]
Increments the reference count of the val on top of stack.

### [refd]
Decrements the reference count of the val on top of stack. Frees if the reference count became 0. Pops the value off the stack.

### [make_1 *argc*]
Constructs an object, where the top *argc* arguments on the stack are fields, and *argc* + 1 is the location of the type to create.

### [pushfree_1 *count*]
Push the offset of the top *count* frame location from the top of stack onto the freestack. Intended to be used only for local variables and the destructuring objects.

### [popfree_1 *count*]
Pop the top *count* offsets from the freestack, decrementing the references at those locations. Intended to be used only for local variables and destructuring objects.

### [clone]
Pop the top object off the stack. Duplicate it and push the duplicate.

### [safe]
Indicates a safe point. The vm may suspend here, update external data sctructure, etc.

### [read_1 *idx*]
Read the field of the object on the top of the stack at index *idx*. *idx* == 1 is the index of the type tag in most objects, and the actual fields start at *idx* == 2.
Pops the object off the stack.

### [write_1 *idx*]
Write the second value on the stack to the object on the top of the stack at index *idx*. *idx* == 1 is the index of the type tag in most objects, and the actual fields start at *idx* == 2.
Pops both the object and value off the stack.

### [stack_1 *diff*]
Move the stack up or down by *diff*. Note that the pop opcode is a special case, where *diff* = -1.

### [noop]
Does nothing.

### [end]
Stops the VM immediately.

### [debug]
Unspecified debugging related behavior.

### [f2i]
Pops the top value from the stack (a float) and converts it to an integer.

### [i2f]
Pops the top value from the stack (an integer) and converts it to an float.

### [band]
Pops the top two values from the stack (both integers) and pushed the binary and of the two.

### [bor]
Pops the top two values from the stack (both integers) and pushed the binary or of the two.

### [bxor]
Pops the top two values from the stack (both integers) and pushed the binary xor of the two.

### [bnot]
Pops the top value from the stack (an integer) and pushed the binary inverse of it.

### [bsl]
Pops the top two values from the stack (both integers) and pushed the the second value bit shifted to the left by the first value.

### [bsrl]
Pops the top two values from the stack (both integers) and pushed the the second value bit shifted to the right by the first value, without doing sign extension.

### [bsra]
Pops the top two values from the stack (both integers) and pushed the the second value bit shifted to the right by the first value, doing sign extension.

### [catch_w] [*offset*]
Pops the top value on the stack, which should be a type tag. Push a handler onto the exception handling stack. If an exception matching the type tag occurs, the top handler will be invoked by jumping to the location the handler was registered plus *offset*.

### [throw]
Pops the top value on the stack, and use it as an exception. Find the top handler on the exception handling stack which matches the type tag. Unwind function frames, using the freestack to decrease the reference counts until we hit the handler's function's frame. Push the exception back on the stack, and jump to the address specified by the handler.

### [reserved]
Behavior may be defined at a later date.

