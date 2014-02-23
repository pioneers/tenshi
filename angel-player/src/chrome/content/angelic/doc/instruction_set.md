Angelic VM Spec
===============
# Opcodes
Opcodes are 7 bit numbers mapping to particular functions in the Angelic VM.
The top (8th) bit is used to control value inspection in the C implementation.

## Opcode Loading and Evalutation
Opcodes are loaded in groups of 4 (32 bits) called "bunches." As opcodes are evaluated, the other opcodes are removed from the bunch, and 0s are added at the end. Opcodes may also consume some fixed number of bytes from the bunch, indicated in the rest of this documentation as `[opcode_name arg1 ... argn]`. Instructions should not rely on bytes of 0 being fed into the bunch as opcodes are evaluated (the compiler should not, for example, emit a `li1` at the end of a bunch to load a 0). This implies that opcodes should not receive more than 3 bytes of arguments in this manner.
Opcodes may also remove whole bunches (multiples of 32 bits) from the opcode stream. This is indicated in the rest of this document as `[opcode_name] [arg]`. The number of bunches consumed in this way need not be bounded or fixed at a particular value per opcode.

If the top bit of an opcode is set, the evaluation of that opcode is logged in some VM specific way.
## Angelic VM Instruction Set
This is the number to opcode mapping.

<code>

  * 0x00 : next
  * 0x01 : dup1
  * 0x02 : li4
  * 0x03 : set1
  * 0x04 : pop
  * 0x05 : call1
  * 0x06 : ret
  * 0x07 : eq
  * 0x08 : j1
  * 0x09 : j2
  * 0x0a : j3
  * 0x0b : j4
  * 0x0c : bz1
  * 0x0d : bz2
  * 0x0e : bz3
  * 0x0f : bz4
  * 0x10 : not
  * 0x11 : fadd
  * 0x12 : fsub
  * 0x13 : fmul
  * 0x14 : fdiv
  * 0x15 : fmod
  * 0x16 : iadd
  * 0x17 : isub
  * 0x18 : imul
  * 0x19 : idiv
  * 0x1a : imod
  * 0x1b : uadd
  * 0x1c : usub
  * 0x1d : umul
  * 0x1e : udiv
  * 0x1f : umod
  * 0x20 : refi
  * 0x21 : refd
  * 0x22 : make1
  * 0x23 : pushfree1
  * 0x24 : popfree1
  * 0x25 : clone
  * 0x26 : safe
  * 0x27 : read1
  * 0x28 : write1
  * 0x29 : stack1
  * 0x2a : noop
  * 0x2b : end
  * 0x2c : debug
  * 0x2d : reserved
  * 0x2e : reserved
  * 0x2f : reserved
  * 0x30 : reserved
  * 0x31 : reserved
  * 0x32 : reserved
  * 0x33 : reserved
  * 0x34 : reserved
  * 0x35 : reserved
  * 0x36 : reserved
  * 0x37 : reserved
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

### [dup1 *idx*]
Duplicates the value *idx* places from the top of stack.

### [li4] [*val*]
Causes *val* to be pushed to the top of the stack.

### [set1 *idx*]
Causes the top value of the stack to be written into *idx* from the top of stack. Pop's the top value from the stack.

### [pop]
Removes the top value from the stack.

### [call1 *argc*]
Performs a function call, where the top *argc* arguments on the stack are arguments, and *argc* + 1 is the location of the function to call.

### [ret]
Return from a function call. Saves and restores the top value on the stack.
If there are no further functions to return to, stops the vm.

### [eq]
Pops the top two values on the stack, and pushes whether they are equal.

### [j1 *relative_dest*]
Causes PC to be moved by *relative_dest*, a signed 8 bit integer.

### [j2 *relative_dest0* *relative_dest1*]
Causes PC to be moved by *relative_dest0*:*relative_dest1*, a signed 16 bit integer.

### [j3 *relative_dest0* *relative_dest1* *relative_dest2*]
Causes PC to be moved by *relative_dest0*:*relative_dest1*:*relative_dest2*, a signed 24 bit integer.

### [j4] [*relative_dest*]
Causes PC to be moved by *relative_dest*, a signed 32 bit integer.

### [bz1 *relative_dest*]
Pops the top value on the stack. If that value is zero, causes PC to be moved by *relative_dest*, a signed 8 bit integer.

### [bz2 *relative_dest0* *relative_dest1*]
Pops the top value on the stack. If that value is zero, causes PC to be moved by *relative_dest0*:*relative_dest1*, a signed 16 bit integer.

### [bz3 *relative_dest0* *relative_dest1* *relative_dest2*]
Pops the top value on the stack. If that value is zero, causes PC to be moved by *relative_dest0*:*relative_dest1*:*relative_dest2*, a signed 24 bit integer.

### [bz4] [*relative_dest*]
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
Decrements the reference count of the val on top of stack. Frees if the reference count became 0. Pop's the value off the stack.

### [make1 *argc*]
Constructs an object, where the top *argc* arguments on the stack are fields, and *argc* + 1 is the location of the type to create.

### [pushfree1 *count*]
Push the offset of the top *count* frame location from the top of stack onto the freestack. Intended to be used only for local variables and the destructuring objects.

### [popfree1 *count*]
Pop the top *count* offsets from the freestack, decrementing the references at those locations. Intended to be used only for local variables and destructuring objects.

### [clone]
Pop the top object off the stack. Duplicate it and push the duplicate.

### [safe]
Indicates a safe point. The vm may suspend here, update external data sctructure, etc.

### [read1 *idx*]
Read the field of the object on the top of the stack at index *idx*. *idx* == 1 is the index of the type tag in most objects, and the actual fields start at *idx* == 2.
Pops the object off the stack.

### [write1 *idx*]
Write the second value on the stack to the object on the top of the stack at index *idx*. *idx* == 1 is the index of the type tag in most objects, and the actual fields start at *idx* == 2.
Pops both the object and value off the stack.

### [stack1 *diff*]
Move the stack up or down by *diff*. Note that the pop opcode is a special case, where *diff* = -1.

### [noop]
Does nothing.

### [end]
Stops the VM immediately.

### [debug]
Unspecified debugging related behavior.

### [reserved]
Behavior may be defined at a later date.

