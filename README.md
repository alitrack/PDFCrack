# PDFCrack — DEPRECATED / ARCHIVED

> ⚠️ **This project is no longer maintained and has been archived.**

A Password Recovery Tool for PDF-files, originally forked from [pdfcrack.sourceforge.net](http://pdfcrack.sourceforge.net/) (v0.19).

---

## Why this project is archived

PDFCrack served its purpose, but the landscape has moved on:

### What PDFCrack could do
- Recover user/owner passwords from PDFs using the Standard Security Handler
- Supports **revision 2, 3, 4, and 5** (RC4-40, RC4-128, SHA-256)
- Wordlist and brute-force attacks
- Save/resume cracking jobs

### What PDFCrack **cannot** do (and why it's time to move on)
- ❌ **No revision 6 (AES-256 / PDF 2.0)** support — all modern PDFs since ~2017 use R6
- ❌ **Single-threaded only** — no multi-core or GPU acceleration
- ❌ **Cannot decrypt document content** — only recovers passwords, won't decrypt AES-encrypted streams
- ❌ **Primitive PDF parser** — fragile, doesn't handle linearized/streaming PDFs
- ❌ **No mask attacks, no real permutation engine, no distributed cracking**
- ❌ **Stalled since 2020** — upstream is also unmaintained

---

## Where to go instead

### For password cracking:

| Tool | Language | Stars | Why it's better |
|------|----------|-------|-----------------|
| **[pdfrip](https://github.com/mufeedvh/pdfrip)** | Rust | 1.4k+ | Multi-threaded, R6 support, mask/date/number attacks, checkpoint/resume, actively maintained |
| **[hashcat](https://github.com/hashcat/hashcat)** | C | 34k+ | GPU-accelerated, industry standard, supports PDF modes 10400–10700 |
| **[John the Ripper](https://github.com/openwall/john)** | C | 10k+ | GPU support via OpenCL, broad format support |

### For PDF encryption/decryption as a library:
- **[oxide-pdf](https://github.com/alitrack/oxide-pdf)** (Rust) — PDF toolkit monorepo, encryption support planned
- **[qpdf](https://github.com/qpdf/qpdf)** (C++) — battle-tested PDF transformation library

### For simply removing known-password protection:
- `qpdf --password=XXXX --decrypt input.pdf output.pdf`

---

## Historical note

This fork added **Windows support** to the original pdfcrack 0.19 codebase. The original tool was written by Henning Norén (2006–2020) as one of the first open-source PDF password recovery tools. It was small, dependency-free, and served the community well for over a decade.

The repository is kept in archived (read-only) state for historical reference.

---

*Archived: June 2026*
