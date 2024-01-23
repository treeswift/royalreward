# Modena

Modena is a city in Italy.

## Known issues

1. Only the console interactive mode is currently supported. Prospective command-line options have been documented,
but none (except the installation path) has been implemented so far.

2. Public domain unpacking tools have been incorporated into the repository, but haven't yet been integrated.

The plan for *Modena* is to create a cache subdirectory (e.g. `~Modena`) under the base installation directory
and unpack the original binary into it, for further out-of-place patching. Currently, *Modena* simply expects
`./Modena/U2.UPK` to be the unpacked representation (executable-as-stored).

Also, *Modena* expects that look-up tables that need to be installed into the executable file have fixed locations,
and makes no attempt at locating them based on their presumed contents.

Once the former limitation is resolved and the latter receives a minimal viable preemptive workaround,
this branch can be merged.
