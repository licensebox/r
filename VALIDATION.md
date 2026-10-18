# Validation — 2026-10-18

PASS: R 4.3.3 package build/install, repeated creation/close, finalization, callback exception containment, idempotent close, and all 69 shared protocol checks.

The C core was built as a shared library from the public SDK checkout. Test keys, times, identities and loopback server data are synthetic. Linux x86-64 only; other OS/runtime paths are unvalidated. No remote release or registry publication is performed by these scripts.
