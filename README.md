# string16le
extract strings (not only English strings) from UTF16LE binary files (e.g. windows executable) and printed as UTF8 strings.
The difference between strings-utf16 and the utility iconv with flag "-c" is that strings-utf16 produce an output that can
be piped to grep command.

# Compiling
To compile under linux, issue for example the command:
```make buf_size=1024 strings-utf16```

# Running
To run, issue for example the command:
```./strings-utf16 -l fr_FR.UTF8 dll/fr-FR/kernel32.dll.mui```
