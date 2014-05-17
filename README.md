keepassx
========

KeePassX is a cross platform port of the windows application “Keepass Password Safe”.


The ploki/keepassx fork
=======================

The purpose of this fork is to add TOTP verification code generation to keepassx
(a.k.a. google-authenticator) using oath-toolkit.


How it works for the end user
=============================

When an entry starts with "totp+" its password is used to generate the verification code when
a "copy password to clipboard" is triggered.
The password field of the entry has to be used to store the TOTP seed.

Note that two entries per sites using TOTP will be needed.
one the verification code              (e.g. "totp+https://www.github.com/" )
and the other for the regular password (e.g. "https://www.github.com" )


How it is implemented
=====================

The quick and dirty way.
