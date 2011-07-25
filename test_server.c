#include <stdio.h>
#include "yubi_zeromq_client.h"

int main(int argc, char *argv[])
{
  if (argc != 4)
    return 255;

  printf("%d\n", ask_server(argv[2], argv[3], argv[1]));
  return 0;
}
