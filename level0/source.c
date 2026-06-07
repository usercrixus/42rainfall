int __cdecl main(int argc, const char **argv, const char **envp)
{
  _DWORD v4[2]; // [esp+10h] [ebp-10h] BYREF
  int v5; // [esp+18h] [ebp-8h]
  int v6; // [esp+1Ch] [ebp-4h]

  if ( atoi(argv[1]) == 423 )
  {
    v4[0] = _strdup("/bin/sh");
    v4[1] = 0;
    v6 = _getegid();
    v5 = _geteuid();
    _setresgid(v6, v6, v6);
    _setresuid(v5, v5, v5);
    execv("/bin/sh", v4);
  }
  else
  {
    IO_fwrite("No !\n", 1, 5, stderr);
  }
  return 0;
}
