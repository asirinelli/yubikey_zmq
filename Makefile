prefix	= /usr/local
sbindir	= $(prefix)/sbin
bindir	= $(prefix)/bin
pamlibdir = /lib/security
DESTDIR =

all:
	@(cd src; $(MAKE) all)

clean:
	@echo "Cleaning..."
	@(cd src; $(MAKE) clean)
	@rm -rf bin lib

install: all
	mkdir -p $(sbindir)
	install bin/yubi_zeromq_server $(DESTDIR)$(sbindir)/yubi_zeromq_server
	chmod 755 $(DESTDIR)$(sbindir)/yubi_zeromq_server
	install bin/test_yubi_server $(DESTDIR)$(bindir)/test_yubi_server
	chmod 755 $(DESTDIR)$(bindir)/test_yubi_server
	install lib/pam_yubikey.so $(DESTDIR)$(pamlibdir)/pam_yubikey.so
	chmod 644 $(DESTDIR)$(pamlibdir)/pam_yubikey.so

uninstall:
	rm -f $(DESTDIR)$(sbindir)/yubi_zeromq_server
	rm -f $(DESTDIR)$(bindir)/test_yubi_server
	rm -f $(DESTDIR)$(pamlibdir)/pam_yubikey.so
