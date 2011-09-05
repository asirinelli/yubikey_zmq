====================
 Yubikey zmq server
====================

Copyright (C) 2011 Antoine Sirinelli <antoine@monte-stello.com>.

Licensed under the GPL (Version 2 or greater).

This set of programs have been written to allow a modular use of
authentication using Yubikeys_. A server verifies tokens submitted by
different means. Communications with the server rely on the ZeroMQ_
library. Clients submits one-time passwords to the server which check
their validity.

Theses programs have been written in order to offer a completely
independent authentication process without relying on third-party
services. This scheme also assumes that the Yubikeys do not present
their own serial number when used which mean that the user has to give
his login in order to be authenticated.

Programs
========

yubi_zeromq_server
------------------

This is the server. It is written in C and has a small memory
footprint. It uses ZeroMQ_ in a *Request/Reply* mode and stores
credentials in a SQLite_ database.

pam_yubikey.so
--------------

This a module for the linux PAM. Any program using the PAM interface
can then use one time password from the Yubikey. The recommended
policy is to couple it with a traditional password.

mod_authn_yubikey.so
--------------------

This is a module for the Apache_ webserver. Yubikeys can then be used
in classic web authentication.

update_yubi_db
--------------

This is simple utility written to manage the server database. With
this program, users can be created, deleted or updated in the
database.

generate_random_token
---------------------

This utility generates a random identity and AES key, stores them in a
Yubikey_ and save the credentials in a file ready to be used by
update_yubi_db in order to import a new user in the database. It uses
the `Yubikey personalization`_ library. The random generator is the
kernel.

Compilation
===========

``make`` should be able to compile all the modules. Nevertheless, the
following dependencies are needed (as Debian packages):

- libsqlite3-dev: SQLite_ development files (used by the server to
  store the credentials)
- libzmq-dev: ZeroMQ_ development files (used by all modules for communication
  between the server and the clients)
- libpam0g-dev: PAM development files (used by the PAM module only)
- pkg-config
- libyubikey-dev: Yubikey_ development files (used by the server to
  validate tokens)
- apache2--dev: Apache_ development files (used by the Apache module)
- libykpers-1-dev: `Yubikey personalization`_ development files (used
  by generate_random_token)


Installation
============

For Debian (and Ubuntu) user, a crude script is provided to generate a
``.deb`` package. This package contains all the modules and
programs. It install also the server database and init script to be
run at startup.

ToDo
====

* Divide Debian package into sub-packages
* Write a better documentation
* Comment the code
* Write a Nginx_ module
* Write a module to interact with a VPN

yubi_zeromq_server
------------------

* Add an option to encrypt the database.

pam_yubikey.so
--------------

* Pass the server location in the PAM arguments instead of being
  hard-coded.
* Implement a timeout in case the server does not respond.

mod_authn_yubikey.so
--------------------

* Pass options to the modules through Apache config files instead of
  being hard-coded (server location, grace time, IP database
  directory)
* Implement a timeout in case the server does not respond.

update_yubi_db
--------------

* Add an option to test an unknown key against all the key stored in
  the database.

generate_random_token
---------------------

* Isolate the bug that raise an error when writing the configuration
  in the Yubikey with the testing Debian package.


.. _Yubikey: http://yubico.com/yubikey
.. _Yubikeys: http://yubico.com/yubikey
.. _ZeroMQ: http://www.zeromq.org/
.. _SQLite: http://www.sqlite.org/
.. _Apache: http://httpd.apache.org/
.. _Yubikey personalization: https://github.com/Yubico/yubikey-personalization
.. _Nginx: http://wiki.nginx.org/
