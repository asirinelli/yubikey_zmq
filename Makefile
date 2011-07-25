prefix	= /usr/local
sbindir	= $(prefix)/sbin

all:
	@(cd src; $(MAKE) all)

clean:
	@echo "Cleaning..."
	@(cd src; $(MAKE) clean)
	@rm -rf bin lib
