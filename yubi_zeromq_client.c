#include <stdio.h>
#include <string.h>
#include "zhelpers.h"
#include "yubi_zeromq_client.h"

int ask_server(const char *user, const char *token, const char *server)
{
  void *context, *requester;
  char *request, *response;
  int out;

  context = zmq_init(1);
  requester = zmq_socket(context, ZMQ_REQ);
  zmq_connect(requester, server);

  request = malloc((strlen(user) + strlen(token) + 2) * sizeof(char));
  sprintf(request, "%s %s", user, token);
  s_send(requester, request);
  free(request);

  response = s_recv(requester);
  zmq_close(requester);
  zmq_term(context);
  if (strcmp(response, TOKEN_OK) == 0)
    out = CHK_OK;
  else if (strcmp(response, BAD_TOKEN) == 0)
    out = CHK_FAIL;
  else if (strcmp(response, UNKNOWN_USER) == 0)
    out = CHK_UNKNOWN;
  else
    {
      printf("Unexpected error\n");
      out = CHK_FAIL;
    }

  free(response);
  return out;
}
