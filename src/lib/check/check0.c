/* %COPYRIGHT% */

#include "lagopus_apis.h"


static inline const char *
myname(const char *argv0) {
  const char *p = (const char *)strrchr(argv0, '/');
  if (p != NULL) {
    p++;
  } else {
    p = argv0;
  }
  return p;
}


int
main(int argc, const char *const argv[]) {
  const char *nm = myname(argv[0]);
  (void)argc;

  lagopus_msg("this should emitted to stderr.\n");

  /*
   * log to stderr.
   */
  if (IS_LAGOPUS_RESULT_OK(
        lagopus_log_initialize(LAGOPUS_LOG_EMIT_TO_UNKNOWN, NULL,
                               false, true, 1)) == false) {
    lagopus_msg_fatal("what's wrong??\n");
    /* not reached. */
  }
  lagopus_dprint("debug to stderr.\n");

  /*
   * log to file.
   */
  if (IS_LAGOPUS_RESULT_OK(
        lagopus_log_initialize(LAGOPUS_LOG_EMIT_TO_FILE, "./testlog.txt",
                               false, true, 10)) == false) {
    lagopus_msg_fatal("what's wrong??\n");
    /* not reached. */
  }
  lagopus_dprint("debug to file.\n");
  lagopus_msg_debug(5, "debug to file, again.\n");

  if (IS_LAGOPUS_RESULT_OK(
        lagopus_log_initialize(LAGOPUS_LOG_EMIT_TO_SYSLOG, nm,
                               false, false, 10)) == false) {
    lagopus_msg_fatal("what's wrong??\n");
    /* not reached. */
  }
  lagopus_msg_debug(5, "debug to syslog.\n");

  /*
   * log to stderr, again.
   */
  if (IS_LAGOPUS_RESULT_OK(
        lagopus_log_initialize(LAGOPUS_LOG_EMIT_TO_UNKNOWN, NULL,
                               false, true, 1)) == false) {
    lagopus_msg_fatal("what's wrong??\n");
    /* not reached. */
  }
  lagopus_dprint("will exit 1 ...\n");
  lagopus_exit_error(1, "exit 1 on purpose.\n");

  return 0;
}
