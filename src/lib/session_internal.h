/* %COPYRIGHT% */

#ifndef __SESSION_INTERNAL_H__
#define __SESSION_INTERNAL_H__

#define SESSION_BUFSIZ 4096

struct session {
  /* socket descriptor */
  int sock;
  int family;
  int type;
  int protocol;
  session_type_t session_type;
  short events; /* for session_event_poll */
  short revents; /* for session_event_poll */
  struct session_buf {
    char *rp;
    char *ep;
    char buf[SESSION_BUFSIZ];
  } rbuf;
  /* function pointers */
  lagopus_result_t (*connect)(lagopus_session_t s, const char *host,
                              const char *port);
  lagopus_result_t (*accept)(lagopus_session_t s1, lagopus_session_t *s2);
  ssize_t (*read)(lagopus_session_t, void *, size_t);
  ssize_t (*write)(lagopus_session_t, void *, size_t);
  void (*close)(lagopus_session_t);
  void (*destroy)(lagopus_session_t);
  lagopus_result_t (*connect_check)(lagopus_session_t);
  /* protocol depended context object */
  void *ctx;
};

#endif /* __SESSION_INTERNAL_H__ */

