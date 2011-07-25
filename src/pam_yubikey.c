/*
 * auth [user_unknown=ignore success=ok default=die] pam_yubikey.so
 *
 * This case to be implemented: PAM_AUTHINFO_UNAVAIL;
 */

#define DEBUG_YUBIKEY 0
/* partly stolen from pam_pwdfile.c */
#include <libintl.h>
#define _(msgid) dgettext("Linux-PAM", msgid)
#define N_(msgid) msgid

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "yubi_zeromq_client.h"

#include <security/pam_appl.h>

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_PASSWORD
#include <security/pam_modules.h>

#define SERVER_LINK "tcp://localhost:5555"

#define D(x) do {							\
    printf ("debug: %s:%d (%s): ", __FILE__, __LINE__, __FUNCTION__);	\
    printf x;								\
    printf ("\n");							\
  } while (0)

#define DBG(x) if (DEBUG_YUBIKEY) { D(x); }

static void
setcred_free (pam_handle_t *pamh, void *ptr, int err)
{
  if (ptr)
    free (ptr);
}

PAM_EXTERN int
pam_sm_authenticate (pam_handle_t * pamh,
		     int flags, int argc, const char **argv)
{
  int retval, *ret_data=NULL;
  int try_first_pass=0, use_first_pass=0;
  const char *user = NULL;
  const char *password = NULL;
  struct pam_conv *conv;
  struct pam_message *pmsg[1], msg[1];
  struct pam_response *resp;
  int nargs = 1, i;

  ret_data = malloc(sizeof(int));

  for (i=0; i<argc; i++)
    {
      if (strcmp(argv[i], "try_first_pass") == 0)
	try_first_pass = 1;
      else if (strcmp(argv[i], "use_first_pass") == 0)
	use_first_pass = 1;
    }

  retval = pam_get_user (pamh, &user, NULL);
  if (retval != PAM_SUCCESS)
    {
      DBG (("get user returned error: %s", pam_strerror (pamh, retval)));
      goto done;
    }
  DBG (("get user returned: %s", user));

  if (ask_server(user, "test", SERVER_LINK) == CHK_UNKNOWN)
    {
      DBG(("User [%s] unknown: not going further", user));
      retval = PAM_USER_UNKNOWN;
      goto done;
    }

  if (try_first_pass || use_first_pass)
    {
      retval = pam_get_item (pamh, PAM_AUTHTOK, (const void **) &password);
      if (retval != PAM_SUCCESS)
	{
	  DBG (("get password returned error: %s",
	      pam_strerror (pamh, retval)));
	  goto done;
	}
      DBG (("get password returned: %s", password));
    }

  if (use_first_pass && password == NULL)
    {
      DBG (("use_first_pass set and no password, giving up"));
      retval = PAM_AUTH_ERR;
      goto done;
    }

  if (password)
    {
      retval = ask_server(user, password, SERVER_LINK);
      switch(retval)
	{
	case CHK_OK:
	  retval = PAM_SUCCESS;
	  goto done;
	case CHK_UNKNOWN:
	  retval = PAM_USER_UNKNOWN;
	  goto done;
	default:
	  password = NULL;
	}
    }

  retval = pam_get_item (pamh, PAM_CONV, (const void **) &conv);
  if (retval != PAM_SUCCESS)
    {
      DBG (("get conv returned error: %s", pam_strerror (pamh, retval)));
      goto done;
    }

  pmsg[0] = &msg[0];
  msg[0].msg = _("Password: ");
  msg[0].msg_style = PAM_PROMPT_ECHO_OFF;
  resp = NULL;

  retval = conv->conv (nargs, (const struct pam_message **) pmsg,
		       &resp, conv->appdata_ptr);

  if (retval != PAM_SUCCESS)
    {
      DBG (("conv returned error: %s", pam_strerror (pamh, retval)));
      goto done;
    }

  if (resp->resp == NULL)
    {
      DBG (("conv returned NULL passwd?"));
      goto done;
    }

  DBG (("conv returned: %s", resp->resp));

  password = resp->resp;

  retval = ask_server(user, password, SERVER_LINK);
  DBG (("ask_server returned: %d", retval));
  switch(retval)
    {
    case CHK_OK:
      retval = PAM_SUCCESS;
      break;
    case CHK_UNKNOWN:
      retval = PAM_USER_UNKNOWN;
      break;
    default:
      retval = PAM_AUTH_ERR;
    }

 done:
  *ret_data = retval;
  pam_set_data(pamh, "yubikey_setcred_return",
	       (void *) ret_data, setcred_free);

  DBG (("done. [%s]", pam_strerror (pamh, retval)));

  return retval;
}

PAM_EXTERN int
pam_sm_setcred (pam_handle_t * pamh, int flags, int argc, const char **argv)
{
  int retval;
  const void *pretval = NULL;

  DBG (("called."));

  retval = PAM_SUCCESS;
  if (pam_get_data(pamh, "yubikey_setcred_return", &pretval) == PAM_SUCCESS
      && pretval)
    {
      retval = *(const int *)pretval;
      pam_set_data(pamh, "yubikey_setcred_return", NULL, NULL);
    }

  DBG (("done. [%s]", pam_strerror (pamh, retval)));

  return retval;
}


#ifdef PAM_STATIC

struct pam_module _pam_yubikey_modstruct = {
  "pam_yubikey",
  pam_sm_authenticate,
  pam_sm_setcred,
  NULL,
  NULL,
  NULL,
  NULL
};

#endif
