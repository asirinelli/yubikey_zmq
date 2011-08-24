#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ykpers.h>
#include <ykcore.h>
#include <yubikey.h>

void check_error(void)
{
  if (yk_errno)
    {
      if (yk_errno == YK_EUSBERR)
	fprintf(stderr, "USB Error: %s\n", yk_usb_strerror());
      else
	fprintf(stderr, "Yubikey Error: %s (%d)\n", yk_strerror(yk_errno), yk_errno);
      yk_release();
      exit(1);
    }
  if (ykp_errno)
    {
      fprintf(stderr, "Personalization Error: %s (%d)\n", ykp_strerror(yk_errno), ykp_errno);
      yk_release();
      exit(1);
    }
}     

/* static int writer(const char *buf, size_t count, void *stream) */
/* { */
/*         return (int)fwrite(buf, 1, count, (FILE *)stream); */
/* } */

int get_random_bin(unsigned char *out, int len)
{
  FILE *random;
  random = fopen("/dev/random", "r");
  if (random)
    {
      while(len > 0)
	{
	  fread(out+len-1, 1, sizeof(unsigned char), random);
	  len--;
	}
      return 0;
    }
  else
    return -1;
} 

void from_bin_to_string(unsigned char *in, char *out, int len)
{
  int ii;

  for (ii=0; ii<len; ii++)
    snprintf(out + 2*ii, 3, "%02x", in[ii]);
  out[2*len] = 0;
}


int main(int argc, char **argv)
{
  YK_KEY *key=NULL;
  YK_STATUS *status=NULL;
  YKP_CONFIG *config_p=NULL;
  int rc, ii;
  unsigned char bin[16];
  char aes[33];
  char uid[13];

  yk_init();
  check_error();

  key = yk_open_first_key();
  check_error();
  printf("key: %p\n", key);

  status = ykds_alloc();
  check_error();
  printf("status: %p\n", status);

  rc = yk_get_status(key, status);
  //printf("rc: %d\n", rc);
  check_error();

  config_p = ykp_create_config();
  //printf("config_p: %p\n", config_p);
  check_error();

  rc = ykp_configure_for(config_p, 1, status);
  //printf("rc: %d\n", rc);
  check_error();

  for (ii=0; ii<16; ii++)
    bin[ii] = 0;

  get_random_bin(bin, 6);
  from_bin_to_string(bin, uid, 6);
  //printf("uid: %s\n", uid);

  rc = ykp_set_uid(config_p, bin, 6);
  //printf("rc: %d\n", rc);
  check_error();

  get_random_bin(bin, 16);
  from_bin_to_string(bin, aes, 16);
  //printf("aes: %s\n", aes);
  
  ykp_AES_key_from_hex(config_p, aes);
  //printf("rc: %d\n", rc);
  check_error();

  //ykp_write_config(config_p, writer, stdout);

  rc = yk_write_config(key, ykp_core_config(config_p),
  		       ykp_config_num(config_p),
  		       NULL);
  //printf("rc: %d\n", rc);
  check_error();

  yk_release();

  if (argc == 2)
    printf("%s %s %s\n", argv[1], uid, aes);
  else
    printf("%s %s\n", uid, aes);

  return EXIT_SUCCESS;
}
