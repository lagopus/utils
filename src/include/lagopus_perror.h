/* %COPYRIGHT% */

#include "lagopus_logger.h"

#ifdef perror
#undef perror
#endif /* perror */
#define perror(str)	lagopus_msg_error("%s: %s\n", str, strerror(errno))
