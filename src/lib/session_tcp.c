/* %COPYRIGHT% */

#include "lagopus_apis.h"
#include "lagopus_session.h"
#include "session_internal.h"

lagopus_result_t session_tcp_init(lagopus_session_t );

static ssize_t
read_tcp(lagopus_session_t s, void *buf, size_t n) {
  return read(s->sock, buf, n);
}

static ssize_t
write_tcp(lagopus_session_t s, void *buf, size_t n) {
  return write(s->sock, buf, n);
}

lagopus_result_t
session_tcp_init(lagopus_session_t s) {
  s->read = read_tcp;
  s->write = write_tcp;

  return LAGOPUS_RESULT_OK;
}
