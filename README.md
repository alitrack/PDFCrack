# PDFCrack
A Password Recovery Tool for PDF-files

fork from http://pdfcrack.sourceforge.net/ (0.19)

PDFCrack is a GNU/Linux (other POSIX-compatible systems should work too) tool for recovering passwords and content from PDF-files. It is small, command line driven without external dependencies. The application is Open Source (GPL).

### Features

- Supports the standard security handler (revision 2, 3 and 4) on all known PDF-versions
- Supports cracking both owner and user passwords
- Both wordlists and brute forcing the password is supported
- Simple permutations (currently only trying first character as Upper Case)
- Save/Load a running job
- Simple benchmarking
- Optimize search for owner-password when user-password is known


## Compile

### Linux or macOS

```
make
```

### Windows
```
 __WIN32__=1 make
```

## Usage

```
$ pdfcrack --help
```
