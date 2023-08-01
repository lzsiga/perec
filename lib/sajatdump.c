/* sajatdump.c */

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <sys/types.h>
  #include <sys/resource.h>
  #include <sys/wait.h>
  #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tilog.h"

#include "sajatdump.h"

#if defined(_WIN32)
void sajatdump1 (int flags)
{
    fprintf (stderr, "sajatdump: <<<Ha ez nem Windows lenne, most coredump-ot írnék>>>\n");
    if (flags&1) exit (11);
}
#else
static void sajatdump_core (void)
{
    char *p= NULL;
    struct rlimit rlimv, *rlim= &rlimv;

    memset (rlim, 0, sizeof (*rlim));
    getrlimit (RLIMIT_CORE, rlim);
    if (rlim->rlim_cur==0) {
	TiLLogF (2, 0, "sajatdump: rlimit_core soft=%lld hard=%lld"
	    , (long long)rlim->rlim_cur
	    , (long long)rlim->rlim_max
	    );
	if (rlim->rlim_max>0) {
	    rlim->rlim_cur= rlim->rlim_max;
	    setrlimit (RLIMIT_CORE, rlim);
	}
    }

/* valamelyik csak megállítja már! */
    signal (SIGSEGV, SIG_DFL);
    *p= 42;
    abort ();

/* franc, nem sikerült */
    exit (1);
}

void sajatdump1 (int flags)
{
    pid_t child_pid;
    int child_status;

    if ((flags&1)==1) {
	TiLLogF (2, 0, "sajatdump: Exiting with coredump");
	sajatdump_core ();
    }

    TiLLogF (2, 0, "sajatdump: Trying to write coredump file");
    child_pid= fork ();
    if (!child_pid) { /* Én vagyok a child */
	sajatdump_core ();

    } else {	  /* Én vagyok a parent */
	waitpid (child_pid, &child_status, 0);
	TiLLogF (2, 0, "sajatdump: Coredump file is written (perhaps)");
    }
}
#endif
