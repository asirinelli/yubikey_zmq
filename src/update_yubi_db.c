#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include "sql_yubi.h"

#define DEFAULT_DB "/var/lib/yubikey-server/yubikeys.db"

void usage(char *prg)
{
  printf("%s -a [-D database] secrets_file\n\tAdd new keys from a file.\n\n",
	 prg);
  printf("%s -r [-D database] users...\n\tReset counters for given users.\n\n",
	 prg);
  printf("%s -d [-D database] users/id...\n\tDelete users.\n\n",
	 prg);
  printf("%s -u [-D database] user token\n\tUpdate user counter with a given token.\n\n",
	 prg);
  printf("%s -l [-D database] \n\tList users in database.\n\n",
	 prg);
  printf("%s -L [-D database] \n\tList users in database including secrets.\n\n",
	 prg);
}

int list_users(sqlite3 *db, int print_aes)
{
  const unsigned char *user, *uid, *aes;
  int counter, session;
  char *request;
  int rc, id;
  sqlite3_stmt *ppStmt;

  request = sqlite3_mprintf("SELECT id, user, uid, aes, counter, session FROM tokens;");

  rc = sqlite3_prepare_v2(db, request, -1,
			  &ppStmt, NULL);
  sqlite3_free(request);
  if(check_error("DB Error in preparing SELECT", rc, db))
    return EXIT_FAILURE;

  while(sqlite3_step(ppStmt) == SQLITE_ROW)
    {
      id = sqlite3_column_int(ppStmt, 0);
      user = sqlite3_column_text(ppStmt, 1);
      uid = sqlite3_column_text(ppStmt, 2);
      aes = sqlite3_column_text(ppStmt, 3);
      counter = sqlite3_column_int(ppStmt, 4);
      session = sqlite3_column_int(ppStmt, 5);
      if (print_aes)
	printf("%d: %s - %s/%s (%d/%d)\n", id, user, uid, aes, counter, session);
      else
	printf("%d: %s (%d)\n", id, user, counter);
    }

  sqlite3_finalize(ppStmt);

  return EXIT_SUCCESS;
}

int already_in_db(sqlite3 *db, char *user, char *uid, char *aes)
{
  int rc;
  sqlite3_stmt *ppStmt;
  char *request;

  request = sqlite3_mprintf("SELECT *  FROM tokens where user=%Q AND uid=%Q AND aes=%Q;", user, uid, aes);

  rc = sqlite3_prepare_v2(db, request, -1,
			  &ppStmt, NULL);
  sqlite3_free(request);
  if(check_error("DB Error in preparing SELECT", rc, db))
    return 0;

  rc = sqlite3_step(ppStmt);
  sqlite3_finalize(ppStmt);
  if(check_error("Error in INSERT", rc, db))
    return 0;
  if (rc == SQLITE_ROW)
    return 1;

  return 0;
}

int add_users(sqlite3 *db, char *filename)
{
  FILE *f;
  char line[96];
  char *user, *uid, *aes, *request;
  int rc;
  sqlite3_stmt *ppStmt;

  f = fopen(filename, "r");
  if (f == NULL)
    {
      printf("Error opening file: %s\n", filename);
      return EXIT_FAILURE;
    }
  while(fgets(line, 96, f))
    {
      user = strtok(line, " ");
      if (user == NULL)
	continue;
      uid = strtok(NULL, " ");
      if (uid == NULL)
	{
	  printf("Incomplete line\n");
	  continue;
	}
      aes = strtok(NULL, " ");
      if (aes == NULL)
	{
	  printf("Incomplete line\n");
	  continue;
	}
      while((aes[strlen(aes)-1] == '\n') ||
	    (aes[strlen(aes)-1] == '\r'))
	aes[strlen(aes) - 1] = '\0';

      if (already_in_db(db, user, uid, aes))
	continue;
      request = sqlite3_mprintf("INSERT INTO tokens (user, uid, aes, counter, session) VALUES (%Q, %Q, %Q, 0, 0);", user, uid, aes);

      rc = sqlite3_prepare_v2(db, request, -1,
			      &ppStmt, NULL);
      sqlite3_free(request);
      if(check_error("DB Error in preparing INSERT", rc, db))
	return EXIT_FAILURE;

      rc = sqlite3_step(ppStmt);
      sqlite3_finalize(ppStmt);
      if(check_error("Error in INSERT", rc, db))
	return EXIT_FAILURE;

      printf("User '%s' added.\n", user);

    }
  return EXIT_SUCCESS;
}

int reset_users(sqlite3 *db, char **users, int nb_users)
{
  int ii, rc, id;
  char *request, *endptr;
  sqlite3_stmt *ppStmt;

  for (ii=0; ii < nb_users; ii++)
    {
      id = (int) strtol(users[ii], &endptr, 10);
      if (users[ii] == endptr)
	request = sqlite3_mprintf("UPDATE tokens SET counter=0, session=0 WHERE user=%Q;", users[ii]);
      else
	request = sqlite3_mprintf("UPDATE tokens SET counter=0, session=0 WHERE id=%d;", id);

      rc = sqlite3_prepare_v2(db, request, -1,
			      &ppStmt, NULL);
      sqlite3_free(request);
      if(check_error("DB Error in preparing UPDATE", rc, db))
	return EXIT_FAILURE;

      rc = sqlite3_step(ppStmt);
      sqlite3_finalize(ppStmt);
      if(check_error("Error in UPDATE", rc, db))
	return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

int delete_users(sqlite3 *db, char **users, int nb_users)
{
  int ii, rc, id;
  char *request, *endptr;
  sqlite3_stmt *ppStmt;

  for (ii=0; ii < nb_users; ii++)
    {
      id = (int) strtol(users[ii], &endptr, 10);
      if (users[ii] == endptr)
	request = sqlite3_mprintf("DELETE FROM tokens WHERE user=%Q;", users[ii]);
      else
	request = sqlite3_mprintf("DELETE FROM tokens WHERE id=%d;", id);

      rc = sqlite3_prepare_v2(db, request, -1,
			      &ppStmt, NULL);
      sqlite3_free(request);
      if(check_error("DB Error in preparing DELETE", rc, db))
	return EXIT_FAILURE;

      rc = sqlite3_step(ppStmt);
      sqlite3_finalize(ppStmt);
      if(check_error("Error in DELETE", rc, db))
	return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

int update_user(sqlite3 *db, char *user, char *token)
{
  int rc;
  if (strlen(token) != 32)
    {
      printf("Invalid token lenght.\n");
      return EXIT_FAILURE;
    }

  rc = check_user_token(user, token, db);
  switch(rc)
    {
    case CHK_OK:
      printf("User '%s' updated.\n", user);
      return EXIT_SUCCESS;
      break;
    case CHK_UNKNOWN:
      printf("WARNING: User '%s' unknown.\n", user);
      return EXIT_FAILURE;
      break;
    default:
      printf("BAD token.\n");
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  int ch, ret;
  int reset, add, delete, update, list, list_aes;
  char *database_filename;
  sqlite3 *db;

  reset = 0;
  add = 0;
  delete = 0;
  update = 0;
  list = 0;
  list_aes = 0;

  database_filename = DEFAULT_DB;
  while ((ch = getopt(argc, argv, "radulLD:h")) != -1)
    {
      switch(ch)
	{
	case 'r':
	  reset = 1;
	  break;
	case 'a':
	  add = 1;
	  break;
	case 'd':
	  delete = 1;
	  break;
	case 'u':
	  update = 1;
	  break;
	case 'L':
	  list_aes = 1;
	case 'l':
	  list = 1;
	  break;
	case 'D':
	  database_filename = optarg;
	  break;
	case 'h':
	  usage(argv[0]);
	  return EXIT_SUCCESS;
	case '?':
	  usage(argv[0]);
	  return EXIT_FAILURE;
	}
    }
  if (add + delete + reset + update + list != 1)
    {
      printf("One mode must be selected.\n");
      usage(argv[0]);
      return EXIT_FAILURE;
    }

  if (add && (optind != (argc - 1)))
    {
      printf("Token file missing\n");
      usage(argv[0]);
      return EXIT_FAILURE;
    }

  if ((reset || delete) && (optind == argc))
    {
      printf("No user supplied.\n");
      usage(argv[0]);
      return EXIT_FAILURE;
    }

  if (update && (optind != (argc - 2)))
    {
      printf("Update mode requires an user and a token\n");
      usage(argv[0]);
      return EXIT_FAILURE;
    }

  if (list && (optind != argc))
    {
      printf("Not waiting for any parameters in list mode.\n");
      usage(argv[0]);
      return EXIT_FAILURE;
    }

  ret = sqlite3_open(database_filename, &db);
  if (check_error("Can't open database", ret, db))
    exit(EXIT_FAILURE);

  if (add)
    ret = add_users(db, argv[optind]);
  else if (reset)
    ret = reset_users(db, &argv[optind], argc - optind);
  else if (delete)
    ret = delete_users(db, &argv[optind], argc - optind);
  else if (update)
    ret = update_user(db, argv[optind], argv[optind + 1]);
  else if (list)
    ret = list_users(db, list_aes);
  else
    ret = EXIT_FAILURE;

  sqlite3_close(db);
  return ret;
}
