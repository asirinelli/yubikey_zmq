prefix	= /usr/local
sbindir	= $(prefix)/sbin
bindir	= $(prefix)/bin
pamlibdir = /lib/security

all:
	@(cd src; $(MAKE) all)

clean:
	@echo "Cleaning..."
	@(cd src; $(MAKE) clean)
	@rm -rf bin lib

install: all
	mkdir -p $(sbindir)
	install bin/yubi_zeromq_server $(sbindir)/yubi_zeromq_server
	chmod 755 $(sbindir)/yubi_zeromq_server
	install bin/test_yubi_server $(bindir)/test_yubi_server
	chmod 755 $(bindir)/test_yubi_server
	install lib/pam_yubikey.so $(pamlibdir)/pam_yubikey.so
	chmod 644 $(pamlibdir)/pam_yubikey.so

uninstall:
	rm -f $(sbindir)/yubi_zeromq_server
	rm -f $(bindir)/test_yubi_server
	rm -f $(pamlibdir)/pam_yubikey.so
