DEBUG   = 
WARNS   = -Wall
CC      = gcc
CFLAGS  = $(DEBUG) $(WARNS) `pkg-config --cflags sqlite3 libzmq` -fPIC
LD      = gcc
LDF_SRV	= `pkg-config --libs sqlite3 libzmq` -lyubikey
LDF_CLI	= `pkg-config --libs libzmq`

SERVER = ../bin/yubi_zeromq_server
PAM_MODULE = ../lib/pam_yubikey.so
CLIENT = ../bin/test_yubi_server

OUT	= $(SERVER) $(PAM_MODULE) $(CLIENT)

OBJ_SRV = sql_yubi_utils.o yubi_zeromq_server.o

OBJ_CLI	= test_server.o yubi_zeromq_client.o

OBJ_PAM = pam_yubikey.o yubi_zeromq_client.o

all: $(OUT)

$(SERVER): $(OBJ_SRV)
	@echo LD $@
	@mkdir -p ../bin
	@$(LD) $(LDF_SRV) $(OBJ_SRV) -o $@

$(CLIENT): $(OBJ_CLI)
	@echo LD $@
	@mkdir -p ../bin
	@$(LD) $(LDF_CLI) $(OBJ_CLI) -o $@

$(PAM_MODULE): $(OBJ_PAM)
	@echo LD $@
	@mkdir -p ../lib
	@$(LD) -shared -o $@ $(OBJ_PAM) -lpam $(LDF_CLI)

%.o:    %.c
	@echo CC $@
	@$(CC) -c $(CFLAGS) $<

clean:
	@echo "Cleaning src/"
	@rm -f $(OUT) *~ *.o 
