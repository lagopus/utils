/* %COPYRIGHT% */

#ifndef __QMUXER_TYPES_H__
#define __QMUXER_TYPES_H__





typedef struct lagopus_qmuxer_poll_record {
  lagopus_bbq_t m_bbq;
  lagopus_qmuxer_poll_event_t m_type;
  ssize_t m_q_size;
  ssize_t m_q_rem_capacity;
} lagopus_qmuxer_poll_record;


typedef struct lagopus_qmuxer_record {
  lagopus_mutex_t m_lock;
  lagopus_cond_t m_cond;
} lagopus_qmuxer_record;





#endif /* __QMUXER_TYPES_H__ */
