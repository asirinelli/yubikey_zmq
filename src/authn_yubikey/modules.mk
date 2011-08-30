mod_authn_yubikey.la: mod_authn_yubikey.slo yubi_zeromq_client.slo
	$(SH_LINK) $(LIBS) -rpath $(libexecdir) -module -avoid-version  mod_authn_yubikey.lo yubi_zeromq_client.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_authn_yubikey.la
