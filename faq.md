# Frequently Asked Questions

## Is pdfcrack illegal to use?

It depends on where you live, but probably not as long as you only use it for documents that you have owners permission to use it on. In Sweden (where the author of pdfcrack lives) the usage is regulated by [Lag (1960:729), 6 a kap.](http://www.notisum.se/rnp/SLS/LAG/19600729.HTM) and most of the countries in EU have similar laws. The only supported usage of PDFCrack is, and will be, to recover passwords and/or content that you own but have lost access to.

## Who might want to use PDFCrack?

Anyone who has forgotten his/her password to a PDF-document [s]he created. Data-archaeologists, Computer Forensics Professionals, people who want to test their password-strength and probably many more.

## What would be a strong password for my PDF-file?

128 Bit, Revision 3 with at least 8 characters (preferably with numbers, both upper and lowercase and non alphabetic characters). Any 40-bit can be broken by a brute-force search of the keyspace and even though it is not yet supported by PDFCrack there are commercial applications that can break those within a couple of days.

## Why did you write PDFCrack?

To learn more about the security of PDF-documents and as I was unable to open a pdf-file that I was the owner of, I began to write it. Later I understood that the problem with my pdf-file was a encoding-issue (the password was in iso-latin 1, and my viewer was trying with utf-8) but then I had already started, and as there was no program that could recover PDF-passwords that was Open Source or free so I thought that it might be of use for others and the rest is history.

## Can I change/override/remove the copy/print/etc. restrictions with PDFCrack?

No, and support for that will not be added. If you want to recover a document you can recover the owner-password and then use another tool for decrypting and/or changing the permissions. PDFCrack is a tool for recovering the information with the owners consent, not overriding the allowed usage. It would probably also be illegal to distribute PDFCrack in many countries if this was added.

## How do I save the state of my pdfcrack job?

Abort pdfcrack by sending a *SIGINT*-signal to the pdfcrack process. On many platforms this is done by pressing Ctrl-C in the terminal that runs the software and on most Unix/Linux systems you can also do *kill -2 PID* where *PID* is the Process ID of pdfcrack.

## Does PDFCrack support bruteforcing the RC4 Keyspace?

Not yet. But if good support to handle the pdf-structure is added it would be a simple thing to add.

## What is the default character set when bruteforcing?

Only the standard alphanumerical characters ([a-zA-Z0-9]). This is to speed up the process for almost all standard documents. You can define your own character set by using *-c* as a argument followed by all the characters you wish to include in your set. Please note that you must have the correct encoding set (usually iso latin 1) and escape all special characters correctly.

## Does PDFCrack support distributed and/or multicore/multicpu-systems?

Not yet. The project is open for contributions! ;)
Distributed workloads can be done by some clever working with the save-files though but I have not tried it.

## GUI?

No, but you are welcome to write a frontend for it

## Does it work in Windows?

Not yet. Feel free to make it work and send a patch. It can be built under [Cygwin](http://www.cygwin.com/) though and a kind user posted some unofficial binaries on the [project forums](http://sourceforge.net/forum/?group_id=168561) that you might want to try.