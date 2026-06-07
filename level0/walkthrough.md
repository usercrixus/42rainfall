# level0

We have a binary named `level0`.
Since it's a reverse engineering project, I directly decompile it inside IDA and get the C source we can find in `source.c`.

The solution of this one is straightfoward. We have to give the string `423` as the first argument of the program to get a shell and then read `/home/user/level1/.pass` to get the flag.

---

Why the program `level0` is allowed to read `/home/user/level1/.pass` that is owned by level1 user since we are executing it as level0 user ?

It's because it have the suid bit enabled.

```bash
level0@RainFall:~$ ls -l level0
-rwsr-x---+ 1 level1 users 747441 Mar  6  2016 level0
```

The suid bit is the little `s` we can see in the permission of the file. It means that when we are executing the program, it is runned with the privileges of the file's owner, here level1.

