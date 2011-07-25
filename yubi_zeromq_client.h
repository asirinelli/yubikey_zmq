#define TOKEN_OK "OK"
#define BAD_TOKEN "FAIL"
#define UNKNOWN_USER "UNKNOWN"

#define CHK_OK 1
#define CHK_FAIL 0
#define CHK_UNKNOWN -1

int ask_server(const char *user, const char *token, const char *server);
