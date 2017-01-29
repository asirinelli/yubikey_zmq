#include <stdio.h>
#include <string.h>
#include "yubi_zeromq_client.h"
#include "yubi_zeromq_server.h"
#include <zmq.h>

#define BUFFER_LEN 255

int ask_server(const char *user, const char *token, const char *server)
{
  void *context, *requester;
  char buffer[BUFFER_LEN];
  int out, nbytes;

  context = zmq_ctx_new();
  requester = zmq_socket(context, ZMQ_REQ);
  zmq_connect(requester, server);

  snprintf(buffer, BUFFER_LEN, "%s %s", user, token);
  buffer[BUFFER_LEN - 1] = 0;
  nbytes = strnlen(buffer, BUFFER_LEN);
  zmq_send(requester, buffer, nbytes, 0);

  nbytes = zmq_recv(requester, buffer, BUFFER_LEN, 0);
  nbytes = (nbytes >= BUFFER_LEN) ? BUFFER_LEN-1 : nbytes;
  buffer[nbytes] = 0;
  /* printf("Debug(%d): %s\n", nbytes, buffer); */

  zmq_close(requester);
  zmq_term(context);
  if (strcmp(buffer, TOKEN_OK) == 0)
    out = CHK_OK;
  else if (strcmp(buffer, BAD_TOKEN) == 0)
    out = CHK_FAIL;
  else if (strcmp(buffer, UNKNOWN_USER) == 0)
    out = CHK_UNKNOWN;
  else
    {
      printf("Unexpected error\n");
      out = CHK_FAIL;
    }

  return out;
}
