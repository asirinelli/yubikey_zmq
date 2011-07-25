#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sql_yubi.h"

int check_error(char *context, int rc, sqlite3 *db)
{
  if (rc > 0 && rc < 100)
    {
      fprintf(stderr, "%s: %s\n", context, sqlite3_errmsg(db));
      //sqlite3_close(db);
      return 1;
  }
  return 0;
}

void from_bin_to_string(unsigned char *in, char *out, int len)
{
  int ii;

  for (ii=0; ii<len; ii++)
    snprintf(out + 2*ii, 3, "%02x", in[ii]);
  out[2*len] = 0;
  }

int check_user_token(char *user, char *token, sqlite3 *db)
{
  char *request;
  sqlite3_stmt *ppStmt;
  entry_st entry;
  yubikey_token_st tok;
  int id, rc;

  request = sqlite3_mprintf("SELECT id FROM tokens WHERE user=%Q;", user);

  rc = sqlite3_prepare_v2(db, request, -1,
			  &ppStmt, NULL);
  sqlite3_free(request);
  if(check_error("Error in preparing SELECT", rc, db))
    return CHK_FAIL;

  rc = sqlite3_step(ppStmt);
  if (rc != SQLITE_ROW)
    {
      sqlite3_finalize(ppStmt);
      return CHK_UNKNOWN;
    }
 if (strlen(token) != 32)
   {
     sqlite3_finalize(ppStmt);
     return CHK_FAIL;
   }

  while (rc == SQLITE_ROW)
    {
      id = sqlite3_column_int(ppStmt, 0);
      get_entry_from_id(id, &entry, db);
      yubikey_parse ((uint8_t *) token, entry.aes_bin, &tok);
      rc = check_token(&tok, entry.uid, entry.counter, entry.session);
      if (rc == CHK_OK)
	{
	  rc = update_db(db, id, &tok);
	  sqlite3_finalize(ppStmt);
	  return rc;
	}
      rc = sqlite3_step(ppStmt);
    }
  sqlite3_finalize(ppStmt);
  return CHK_FAIL;
}

int check_token(yubikey_token_st *tok, const char *uid,
		int counter, int session)
{
  char tok_uid[2*YUBIKEY_UID_SIZE+1];
  if (!yubikey_crc_ok_p ((uint8_t *)  tok))
    return CHK_FAIL;
  from_bin_to_string(tok->uid, tok_uid, YUBIKEY_UID_SIZE);

  if(strncmp(tok_uid, uid, YUBIKEY_UID_SIZE))
    return CHK_FAIL;
  if (yubikey_counter (tok->ctr) > counter)
    return CHK_OK;
  if (yubikey_counter (tok->ctr) < counter)
    return CHK_FAIL;
  if (tok->use > session)
    return CHK_OK;

  return CHK_FAIL;
}

int get_entry_from_id(int id, entry_st *entry, sqlite3 *db)
{
  char *request;
  char *uid, *aes;
  int rc;
  sqlite3_stmt *ppStmt;


  request = sqlite3_mprintf("SELECT uid, aes, counter, session FROM tokens WHERE id=%d;", id);

  rc = sqlite3_prepare_v2(db, request, -1,
			  &ppStmt, NULL);
  sqlite3_free(request);
  if(check_error("Error in preparing SELECT", rc, db))
    return CHK_FAIL;

  rc = sqlite3_step(ppStmt);
  if(check_error("Error in SELECT", rc, db))
    return CHK_FAIL;

  if (rc == SQLITE_ROW)
    {
      uid = (char *) sqlite3_column_text(ppStmt, 0);
      //printf("uid: %s\n", uid);
      aes = (char *) sqlite3_column_text(ppStmt, 1);
      //printf("aes: %s\n", aes);
      if ((strlen(uid) != 2*YUBIKEY_UID_SIZE) &&
	  (strlen(aes) != 2*YUBIKEY_KEY_SIZE))
	{
	  sqlite3_finalize(ppStmt);
	  printf("Error in database: wrong uid/aes size\n");
	  return CHK_FAIL;
	}
      strcpy(entry->uid, uid);
      strcpy(entry->aes, aes);

      entry->counter = sqlite3_column_int(ppStmt, 2);
      entry->session = sqlite3_column_int(ppStmt, 3);
      entry->id = id;
      //printf("counter: %d - session: %d\n", entry->counter, entry->session);
      sqlite3_finalize(ppStmt);

      yubikey_hex_decode ((char *) entry->aes_bin,
			  entry->aes, YUBIKEY_KEY_SIZE);
      return CHK_OK;
    }
  else
    {
      sqlite3_finalize(ppStmt);
      return CHK_FAIL;
    }
}

int update_db(sqlite3 *db, int id, yubikey_token_st *tok)
{
  char *request;
  sqlite3_stmt *ppStmt;
  int rc;

  request = sqlite3_mprintf("UPDATE tokens SET counter=%d, session=%d WHERE id=%d;", yubikey_counter(tok->ctr), tok->use, id);

  rc = sqlite3_prepare_v2(db, request, -1,
			  &ppStmt, NULL);
  sqlite3_free(request);
  if(check_error("Error in preparing UPDATE", rc, db))
    return CHK_FAIL;

  rc = sqlite3_step(ppStmt);
  sqlite3_finalize(ppStmt);
  if(check_error("Error in UPDATE", rc, db))
    return CHK_FAIL;
  //printf("counter: %d - session: %d\n", yubikey_counter(tok->ctr), tok->use);
  return CHK_OK;
}
