# level0

## 1. Connect to the VM

The default password for `level0` is `level0`:

```bash
ssh level0@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

## 3. Analyse the binary

The binary is owned by `level1` and has the SUID bit set:

```bash
level0@RainFall:~$ ls -l level0
-rwsr-x---+ 1 level1 users 747441 Mar  6  2016 level0
```

The `s` in the owner permissions signals the SUID bit. When the program runs,
the kernel substitutes the file owner's UID (`level1`) for the caller's UID
(`level0`). The process therefore has `level1` privileges and can read
`/home/user/level1/.pass`.

The source shows that `main` converts its first argument to an integer with
`atoi` and compares the result to the hard-coded value `423`:

```c
if (atoi(argv[1]) == 423)
```

When the check succeeds, the program calls:

```c
execv("/bin/sh", argv);
```

which starts a shell inheriting the effective UID of the process.

## 4. Exploit the binary

Pass `423` as the first argument:

```bash
./level0 423
```

The program spawns a shell with `level1` effective privileges. Verify and read
the next password:

```bash
id
cat /home/user/level1/.pass
```

The `id` output contains `euid=2021(level1)`, confirming the shell has the
required privileges.

The same operation can be performed non-interactively:

```bash
echo 'cat /home/user/level1/.pass' | ./level0 423
```

This prints the password stored in `flag`.
