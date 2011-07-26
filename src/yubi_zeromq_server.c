#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <sqlite3.h>
#include "sql_yubi.h"
#include "zhelpers.h"

#define DEFAULT_DB "/var/lib/yubikey-server/yubikeys.db"
#define DEFAULT_PID "/var/run/yubikey-server.pid"

#define TOKEN_OK "OK"
#define BAD_TOKEN "FAIL"
#define UNKNOWN_USER "UNKNOWN"

#define DAEMON 1

int get_user_token(char *req, char **user, char **token)
{
  *user = strtok(req, " ");
  if (*user == NULL)
    return 255;
  *token = strtok(NULL, " ");
  if (*token == NULL)
    return 255;
  return 0;
}

int main(int argc, char *argv[])
{
  sqlite3 *db;
  void *context;
  void *responder;
  char *recv_str, *user, *token;
  char *database_filename, *pid_filename;
  int ii, rc, ch, nb_bind=0;
  pid_t pid, sid;
  FILE *pid_file;

  database_filename = DEFAULT_DB;
  pid_filename = DEFAULT_PID;

  while ((ch = getopt(argc, argv, "d:p:")) != -1)
    {
      switch(ch)
	{
	case 'd':
	  database_filename = optarg;
	  break;
	case 'p':
	  pid_filename = optarg;
	  break;
	}
    }

  rc = sqlite3_open(database_filename, &db);
  if (check_error("Can't open database", rc, db))
    exit(EXIT_FAILURE);

  if (optind == argc)
    {
      fprintf(stderr, "Please specify where to bind.\n");
      exit(EXIT_FAILURE);
    }

#if DAEMON
  pid = fork();

  if (pid < 0)
    exit(EXIT_FAILURE);
  else if (pid > 0)
    exit(EXIT_SUCCESS);

  umask(0);

  sid = setsid();

  if (sid < 0)
    exit(EXIT_FAILURE);
  pid_file = fopen(pid_filename, "w");
  fprintf(pid_file, "%d", sid);
  fclose(pid_file);

  if ((chdir("/")) < 0)
    exit(EXIT_FAILURE);
#endif

  context = zmq_init(1);
  responder = zmq_socket(context, ZMQ_REP);
  for(ii=optind; ii<argc; ii++)
    {
      if (zmq_bind(responder, argv[ii]) !=0)
	syslog(LOG_WARNING, "Not able to bind on '%s'.", argv[ii]);
      else
	nb_bind++;
    }

  if (nb_bind == 0)
    {
      syslog(LOG_ERR, "No binding available. Quitting...");
      exit(EXIT_FAILURE);
    }
  syslog(LOG_NOTICE, " started.");

  while(1)
    {
      recv_str = s_recv(responder);
      syslog(LOG_NOTICE, "Request recieved: %s", recv_str);
      if (get_user_token(recv_str, &user, &token) != 0)
        s_send(responder, BAD_TOKEN);
      else
        {
	  rc = check_user_token(user, token, db);
	  switch(rc)
	    {
	    case CHK_OK:
	      s_send(responder, TOKEN_OK);
	      syslog(LOG_NOTICE,  "User: %s - token: %s - %s",
		     user, token, TOKEN_OK);
	      break;
	    case CHK_FAIL:
	      s_send(responder, BAD_TOKEN);
	      syslog(LOG_NOTICE,  "User: %s - token: %s - %s",
		     user, token, BAD_TOKEN);
	      break;
	    case CHK_UNKNOWN:
	    default:
	      s_send(responder, UNKNOWN_USER);
	      syslog(LOG_NOTICE,  "User: %s - token: %s - %s",
		     user, token, UNKNOWN_USER);
	    }
        }
      free(recv_str);
    }
  sqlite3_close(db);
  zmq_close(responder);
  zmq_term(context);
  syslog(LOG_NOTICE, " terminated.");
  exit(EXIT_SUCCESS);
}

      
