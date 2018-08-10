/* %COPYRIGHT% */

#ifndef __QMUXER_INTERNAL_H__
#define __QMUXER_INTERNAL_H__





#define NEED_WAIT_READABLE(type) \
  (IS_BIT_SET(((int)(type)), ((int)LAGOPUS_QMUXER_POLL_READABLE)))

#define NEED_WAIT_WRITABLE(type) \
  (IS_BIT_SET(((int)(type)), ((int)LAGOPUS_QMUXER_POLL_WRITABLE)))

#define IS_VALID_POLL_TYPE(t)                                   \
  (((int)t > (int)LAGOPUS_QMUXER_POLL_UNKNOWN &&                \
    (int)t <= (int)LAGOPUS_QMUXER_POLL_BOTH) ? true : false)





void
qmuxer_notify(lagopus_qmuxer_t qmx);


lagopus_result_t
cbuffer_setup_for_qmuxer(lagopus_cbuffer_t cb,
                         lagopus_qmuxer_t qmx,
                         ssize_t *szptr,
                         ssize_t *remptr,
                         lagopus_qmuxer_poll_event_t type,
                         bool is_pre);





#endif /* ! __QMUXER_INTERNAL_H__ */
