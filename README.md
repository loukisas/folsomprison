# FolsomPrison

Folsom prison is a tty  tty login spoofer written in C.

The concept is simple - the attacker logs in his/her account and leaves folsomprison running.

Eventually someone else will walk to this computer and try to login. The victim's credentials will be stored in a file in the attacker's specified directory and the program will log his/hers account out, returning to a real login prompt.

Use it only on systems you have explicit permission to do so.  I am not responsible for malicious use of this program.

### Build

```bash
$ make
```

### Run
```bash
$ folsomprison [-f file] [-h]

Options:
 -f	Where to store the credentials (default ./credentials).
 -h	This message.
```

