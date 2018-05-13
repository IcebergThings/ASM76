---
layout: default
title: ASM76 Specification
---

This document describes both the VM/76 virtual machine platform and the assembly language itself.

Definitions and conventions in this specification
-------------------------------------------------

- **Bit** is defined as usual.
- A **byte** consists of 8 bits.
- An **int** consists of 4 bytes.
- A **long** consists of 8 bytes.
- **0x** leads a hexadecimal number, e.g. 0x1F means 1F<sub>16</sub> (31<sub>10</sub>).
- **_x_.._y_** indicates a range that is inclusive at both ends, i.e. [*x*, *y*].
- **_x_\..._y_** indicates a range that includes *x* but excludes *y*, i.e. [*x*, *y*).

Note that:

- The term **word** doesn't exist here.
- **0** doesn't mean octal numbers in this document. For example, 0017 stands for 17<sub>10</sub>, not 15<sub>10</sub>.

76 Virtual Machine and VM/76 CPU
--------------------------------

### Introduction
- The virtual machine compiles (assembles) ASM76 assembly code and runs the compiled binary code.
- **ASM76** is the assembly language for VM/76 CPU.
	- In order to simplify the assembler, the syntax is very fixed.
	- It has fixed length instruction mnemonics: they are exactly 4 characters long. The assembler implementation is able to process any mnemonic length from 1 to 12 though.
- **VM/76 CPU** is the virtual CPU.
	- It is little-endian.
		- It can only be emulated on native little-endian machines at present.
		- Running the virtual machine on big-endian machines will result unspecifiedly.
	- We use multiple CPU instances in practice. One CPU is usually assigned exclusively to one thread, one sprite, etc.
	- Since it isn't used in real world, it is *quite different* from the CPUs from common ones, such as Intel x86 series CPUs.

### Instruction structure
An instruction is internally represented by a sequence of 10 bytes. Each instruction consists of three parts: an opcode and exactly two operands. For opcodes that need less than two operands, zero bytes are usually used to fill the unused part.

<table class="center">
	<tr>
		<th>Byte #</th>
		<td>0</td>
		<td>1</td>
		<td>2</td>
		<td>3</td>
		<td>4</td>
		<td>5</td>
		<td>6</td>
		<td>7</td>
		<td>8</td>
		<td>9</td>
	</tr>
	<tr>
		<th></th>
		<td colspan="2">Opcode</td>
		<td colspan="4">Operand 1</td>
		<td colspan="4">Operand 2</td>
	</tr>
</table>

### Instruction sets
A variety of instruction sets are provided.

- **[76-Base](#76-base)** is capable of manipulating registers, memory and 8-, 32- and 64-bit integers.
- **[76-Float](#76-float)** is able to deal with floating point values.
- **[76-Vector](#76-vector)** can do vector mathematics.
- **[BIOS Instructions](#bios-instructions)** provides a BIOS interface. Just joking.

### Registers
- There are 112 registers in all, namely $0, $1, $2, …, $111.
- Every register holds a byte.
- The method of manipulation is the same among all registers.
- Register $100 and above have special uses, though you can modify them as if they are ordinary ones.

Special register | Purpose | Default value
---------------- | ------- | -------------
$100..$103 | Instruction Pointer | 0x01000000
$104..$107 | Stack Pointer | 0x01003000
[$109](#109) | A ⋚ B Flag | 0
[$110](#110) | Instruction Set Flags | 0

#### $109
This flag register is used in [CMPx](#cmpx).

#### $110
This flag register is used to enable or disable certain instruction sets. Each bit links with an instruction set.

Bit 0 is originally intended for 76-Base, but since that set can't be disabled, it has been of no use.

Bit # | Instruction set
----- | ---------------
0 | N/A
1 | [76-Float](#76-float)
2 | [76-Vector](#76-vector)
3..6 | *(reserved)*
7 | [BIOS Instructions](#bios-instructions)

### 32-bit memory address

Address range | Size | Name | Usage
------------- | ---- | ---- | -----
0x0\...0x400000 | 4MB | Global memory | For sharing data between CPU instances
0x400000\...0x1000000 | 12MB | IO | For transferring data between the CPU (ASM76) and the outside world (VMDE)
0x1000000\...∞ | depends\* | Local memory | For private use inside a CPU instance

<small>\* The size of the local memory is 16KB by default, which can be resized through [LCMM](#lcmm).</small>

The starting of the local memory is used to store the program. Thanks the machine architecture which does not have a protection system, the program can modify itself freely at runtime.

The ASM76 language syntax
-------------------------

```asm
# ASM76 Example
# Hash signs start comments.
# Only whole line comments are allowed.

# Hexadecimal numbers.
LCMM 0x100

# Decimal numbers and registers.
# Parameters are separated by spaces, not commas.
DATI 1 $1
ADDL $1 $1
ADDL $1 $1
ADDL $1 $1
ADDL $1 $1

# A tag (label) can be used as an address.
JMPA [PlaceToStart]
PUSH $1, 4

# You can make adress tags by using [].
# Use [some_name] to tag the current adress.
[PlaceToStart]

# Register No. is decimal.
MVRL $1 $31
DATI 4 $11
DIVL $1 $11
HALT

# You can place data after HALT.
# Since the machine is HALTed, they will not get executed.
RAWB 'A'
RAWB 0xFF
RAWI 0xFFFFFFFF

# You can use lower or upper case letters in hexadecimal numbers.
# Of course, we don't recommend you to mix them.
RAWL 0xfFfFfFfFfFfFfFfF
FILL 0xABCaa312341

# String literals are also supported.
# Although it's WIP.
FILL "A slow lazy fox swirled across the brown dog."
```

Appendix: object code format
----------------------------

The virtual machine can load a packed program format called the VM/76 object code. It is usually stored in a .obj file, which contains a program. The implementation has a ObjectCode module that deals with this format.

### The format

Offset | Description | Example value | Explanation
------ | ----------- | ------------- | -----------
0 | Magic number | `A3 EF A3 E2 56 4D 37 36` | ‘ｏｂVM76’ in GB2312
8 | Program size | `1E 00 00 00` | 30 bytes, excluding the header and this size field
12 | Instructions | `01 00 07 00 00 00 06 00 00 00`<br>`00 00 00 00 00 00 00 00 00 00`<br>`28 00 00 00 00 00 00 00 00 00` | ADDL $7 $6<br>NOOP<br>HALT
~ | Random data | `01 23 45 67 89 AB CD` | won't be loaded


Instruction set reference
=========================

Conventions
-----------

- The instruction mnemonic must be capital.
- Here's a list of common postfixes in mnemonics.
	- ‘B’, ‘I’ and ‘L’ stands for byte, int and long, respectively.
		- For simplicity, they may be replaced by ‘x’ in this reference.
		- E.g. MOV**B** MOV**I** MOV**L** → MOVx
	- ‘R’ means register and ‘A’ means address.
		- E.g. JMP**R** JMP**A**
	- ‘_’ (underline) is used as a blank filler in mnemonics.
		- E.g. OR**\_**L POP**\_**
- Since a register can only hold one byte, when doing calculations more than one byte, it will start from the specified register and use the following consecutive registers.
	- For example, `MOVL $0 $10` makes $0\...$8 all moved to $10\...$18.
- Italic font (*A*) denotes a parameter.
- Italic font prepended with a dollar sign (*$A*) denotes a register parameter.

Pseudo instructions
-------------------

These are not actually executable instructions, but rather like assembler macros.

### Embedding raw data

Instruction | Description
----------- | -----------
[RAWD](#rawd) | directly embed a piece of data into the program
[FILL](#fill) | same as above, without having to specify the size

#### RAWD/FILL
These are the most powerful instruction in the computer world. With them, you can create anything directly.

For these instructions, the assembler will go back to work with bytes and insert the data directly into the program so that they can be used by the program by accessing the local memory — you usually want to prepend an address tag to get the address.

In order to be aligned to 10 bytes, which is the size of an instruction, there is only RAWD. RAWB/RAWI/RAWL are absent and RAWD accepts five ‘double-bytes.’

The FILL instruction is rather special: it accepts both integer constants and string literals as its ‘operand’. This makes printing functions easier to use. Note that it does not append a zero byte to the string literals automatically and you must do it yourself — though zero bytes are quite often encountered in the machine code and it'll stop printing very soon after getting through the end of the string.

76-Base
-------

76-Base is the most essential instruction set. It is enabled by default, You don't need to enable it and you can not turn it off in any way.

### Memory and registers
To simplify the description, we'll use some word macros.

- **$A** ⩴ the value in register $A
- **[A]** ⩴ the value in the memory address A
- **[$A]** ⩴ the value in the memory address specified by the value in register $A
- **A ← B** ⩴ put B into A (assignment)

Instruction | Description
----------- | -----------
[LCMM](#lcmm) _size\_in\_bytes_ | set local memory size
[DATx](#datx) _data_ _$A_ | *$A* ← *data*
[LDxA](#ldxa) _A_ _$B_ | *$B* ← [*A*]
[LDxR](#ldxr) _$A_ _$B_ | *$B* ← [*$A*]
SLxA _A_ _$B_ | [*A*] ← *$B*
SLxR _$A_ _$B_ | [*$A*] ← *$B*
MOVx _A_ _B_ | [*B*] ← [*A*]
MVRx _$A_ _$B_ | *$B* ← *$A*
MVPx _$A_ _$B_ | [*$B*] ← [*$A*]

#### LCMM
Specifiy the local memory size. It does not has a maximum limit in theory and is 16KB by default. As the command runs, the data in the memory will be cleared and initialized with zeros. Then it will copy the original data into the memory. If the original data is longer, it will be truncated.

#### DATx
Note that due to the limitation of [the instruction size](#instruction-structure), the DATL instruction *doesn't exist*.

#### LDxR
For example:

```asm
DATI 0x00FF0000 $0
LDLR $0 $12
```

After executing the piece of code above, 8 bytes of data from addresses 0x00FF0000..0x00FF0007 will be stored in $12..$19.

### Basic algebra
All mathematical operations takes the registers as unsigned numbers.

Instruction | Description
----------- | -----------
ADDx _$A_ _$B_ | *$A* ← *$A* + *$B*
MINx _$A_ _$B_ | *$A* ← *$A* − *$B*
MTPx _$A_ _$B_ | *$A* ← *$A* × *$B*
[DIVx](#divxmodx) _$A_ _$B_ | *$A* ← ⌊*$A* ÷ *$B*⌋
[MODx](#divxmodx) _$A_ _$B_ | *$A* ← *$A* mod *$B*
[CMPx](#cmpx) _$A_ _$B_ | compare two long/int/byte arithmetically

#### DIVx/MODx
- The division throws the remainder away.
- The modulo throws the quotient away.

#### CMPx
Compare *$A* to *$B*. It updates $109 according to the result of comparision.

$109 | Meaning
---- | -------
0 | *$A* < *$B*
1 | *$A* = *$B*
2 | *$A* > *$B*

### Logical operations

Instruction | Description
----------- | -----------
ANDx _$A_ _$B_ | bitwise logical AND
OR_x _$A_ _$B_ | bitwise logical inclusive OR
XORx _$A_ _$B_ | bitwise logical exclusive OR
[NOTx](#notx) _$A_ | boolean NOT for long/int/byte

#### NOTx
Means `$A = !$A;` in C.

- If *$A* = 0, *$A* will become 1.
- If *$A* ≠ 0, *$A* will become 0.

### Flow control

Instruction | Description
----------- | -----------
NOOP | waste some time
HALT | halt the CPU and stop
JMPR _$A_ | jump to memory address stored in *$A*
JMPA _address_ | jump to memory address *address*
JILR/JIER/JIGR _$A_ | jump to memory address stored in *$A* if $109/$110/$111 = 0xFF
JILA/JIEA/JIGA _address_ | jump to *address* if $109/$110/$111 = 0xFF
CALR _$A_ | jump to memory address stored in *$A* and push the next instruction's address into stack
CALA _address_ | jump to *address* and push the next instruction's address into stack
RETN | `POP_ $100`
PUSH _$A_ _length_ | push registers from *$A*\...$(*A* + *length*) onto the stack
POP_ _$A_ _length_ | pop data from stack to registers *$A*\...$(*A* + *length*)

76-Float
--------

76-Float provides instructions for processing floating point values.

You need to set bit 1 of $110 to 1 first in order to make it work.

```asm
# Example code to enable 76-Float
PUSH $99 1
DATB 0x2 $99
OR_B $110 $99
POP $99 1
```

### Basic floating point arithmetic

Instruction | Description
----------- | -----------
|

76-Vector
---------

76-Vector provides instructions for calculating vectors, just as in the OpenGL shader language.

You need to set bit 2 of $110 to 1 first in order to make it work.

### Basic vector arithmetic

Instruction | Description
----------- | -----------
|

BIOS Instructions
-----------------

**BIOS** is an acronym for Basic Input/Output System, of course.

Interrupts are handled by the firmware attached to the VM. A BIOS can contain at most 255 interrupt handler numbered from 1 to 255. Interrupt #0 is defined as the null call. An interrupt accepts an address as its parameter.

You need to set bit 7 of $110 to 1 first in order to make it work.

### BIOS Access

Instruction | Description
----------- | -----------
[INTX](#intxintr) _function\_id_ _address_ | Do an operation provided by the BIOS
[INTR](#intxintr) _$A_ _address_ | Same as above, but the function ID is put in *$A*

#### INTX/INTR

VMDE provides the following BIOS functions.

*Note that functions 1–8 are unimplemented now.*

Function ID | BIOS Function | Description
----------- | ------------- | -----------
0   | null(uintptr_t a)     | Return NOT (*a* XOR 0x76ABCDEF) — the null call
1   | putc(uint8_t c)       | Print *c* as a byte character
2   | puts(uint32_t addr)   | Print a null-terminated string starting from memory address *addr*
3   | putlx(uint64_t x)     | Print *x* as a hexadecimal number
4   | putix(uint32_t x)     | Print *x* as a hexadecimal number
5   | putbx(uint8_t x)      | Print *x* as a hexadecimal number
6   | putl(uint64_t x)      | Print *x* as a decimal number
7   | puti(uint32_t x)      | Print *x* as a decimal number
8   | putb(uint8_t x)       | Print *x* as a decimal number
11  | GDrawable_render      | Documentation needed
12  | GDrawable_renderOnce  | Idem
13  | GDrawable_batch       | Idem
14  | GDrawable_batchOnce   | Idem
