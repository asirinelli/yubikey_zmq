#include <yubikey.h>
#include <sqlite3.h>

#define CHK_OK 1
#define CHK_FAIL 0
#define CHK_UNKNOWN -1

typedef struct entry {
  char uid[2*YUBIKEY_UID_SIZE+1];
  char aes[2*YUBIKEY_KEY_SIZE+1];
  uint8_t aes_bin[YUBIKEY_KEY_SIZE];
  int counter;
  int session;
  int id;
} entry_st;

int check_error(char *context, int rc, sqlite3 *db);
void from_bin_to_string(unsigned char *in, char *out, int len);
int check_user_token(char *user, char *token, sqlite3 *db);
int check_token(yubikey_token_st *tok, const char *uid, int counter, int session);
int get_entry_from_id(int id, entry_st *entry, sqlite3 *db);
int update_db(sqlite3 *db, int id, yubikey_token_st *tok);
