sbindir	= /usr/sbin
bindir	= /usr/bin
pamlibdir = /lib/security
DESTDIR =

all:
	@(cd src; $(MAKE) all)
	@(cd src/authn_yubikey; $(MAKE) all)

clean:
	@echo "Cleaning..."
	@(cd src; $(MAKE) clean)
	@(cd src/authn_yubikey; $(MAKE) clean)
	@rm -rf bin lib

install: all
	mkdir -p $(DESTDIR)$(sbindir)
	install bin/yubi_zeromq_server $(DESTDIR)$(sbindir)/yubi_zeromq_server
#	chmod 755 $(DESTDIR)$(sbindir)/yubi_zeromq_server
	mkdir -p $(DESTDIR)$(bindir)
	install bin/test_yubi_server $(DESTDIR)$(bindir)/test_yubi_server
#	chmod 755 $(DESTDIR)$(bindir)/test_yubi_server
	mkdir -p $(DESTDIR)$(pamlibdir)
	install lib/pam_yubikey.so $(DESTDIR)$(pamlibdir)/pam_yubikey.so
#	chmod 644 $(DESTDIR)$(pamlibdir)/pam_yubikey.so
	mkdir -p $(DESTDIR)/var/lib/yubikey-server
	chmod 700 $(DESTDIR)/var/lib/yubikey-server
	install src/yubikey-add-key.py $(DESTDIR)$(sbindir)/yubikey-add-key
	@(cd src/authn_yubikey; $(MAKE) install)

uninstall:
	rm -f $(DESTDIR)$(sbindir)/yubi_zeromq_server
	rm -f $(DESTDIR)$(bindir)/test_yubi_server
	rm -f $(DESTDIR)$(pamlibdir)/pam_yubikey.so
	rm -f $(DESTDIR)$(sbindir)/yubikey-add-key
