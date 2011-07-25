DEBUG   = -g
WARNS   = -Wall
CC      = gcc
CFLAGS  = $(DEBUG) $(WARNS) `pkg-config --cflags sqlite3 libzmq` -fPIC
LD      = gcc
LDF_SRV	= `pkg-config --libs sqlite3 libzmq` -lyubikey
LDF_CLI	= `pkg-config --libs libzmq`

OUT     = yubi_zeromq_server test_server pam_yubikey.so

OBJ_SRV = sql_yubi_utils.o yubi_zeromq_server.o

OBJ_CLI	= test_server.o yubi_zeromq_client.o

OBJ_PAM = pam_yubikey.o yubi_zeromq_client.o

all: $(OUT)

yubi_zeromq_server: $(OBJ_SRV)
	$(LD) $(LDF_SRV) $(OBJ_SRV) -o $@

test_server: $(OBJ_CLI)
	$(LD) $(LDF_CLI) $(OBJ_CLI) -o $@

pam_yubikey.so: $(OBJ_PAM)
	$(LD) -shared -o $@ $(OBJ_PAM) -lpam $(LDF_CLI)

%.o:    %.c
	$(CC) -c $(INC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OUT) $(OBJ_SRV) $(OBJ_CLI) $(OBJ_PAM) *~
