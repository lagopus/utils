/* %COPYRIGHT% */

#include "lagopus_apis.h"
#include "lagopus_thread_internal.h"

#include <math.h>


#


typedef struct {
  lagopus_thread_record m_thd;	/* must be on the head. */

  lagopus_mutex_t m_start_lock;

  lagopus_bbq_t m_q;
  ssize_t m_n_puts;
} test_thread_record;
typedef test_thread_record *test_thread_t;





static void
s_test_thread_finalize(const lagopus_thread_t *tptr, bool is_canceled,
                       void *arg) {
  (void)tptr;
  (void)arg;

  lagopus_msg_debug(1, "called. %s.\n",
                    (is_canceled == false) ? "finished" : "canceled");
}


static void
s_test_thread_destroy(const lagopus_thread_t *tptr, void *arg) {
  (void)tptr;
  (void)arg;

  lagopus_msg_debug(1, "called.\n");
}


static lagopus_result_t
s_test_thread_main(const lagopus_thread_t *tptr, void *arg) {
  lagopus_result_t ret = LAGOPUS_RESULT_ANY_FAILURES;
  lagopus_chrono_t *dates = NULL;

  (void)arg;

  if (tptr != NULL) {
    test_thread_t tt = (test_thread_t)*tptr;


    if (tt != NULL) {
      ssize_t i;
      lagopus_chrono_t now;
      dates = (lagopus_chrono_t *)
              malloc(sizeof(lagopus_chrono_t) * (size_t)tt->m_n_puts);
      if (dates == NULL) {
        goto done;
      }

      /*
       * Ready, steady,
       */
      (void)lagopus_mutex_lock(&(tt->m_start_lock));
      /*
       * go.
       */
      (void)lagopus_mutex_unlock(&(tt->m_start_lock));

      for (i = 0; i < tt->m_n_puts; i++) {
        WHAT_TIME_IS_IT_NOW_IN_NSEC(dates[i]);
      }
      if (lagopus_bbq_put_n(&(tt->m_q), (void **)dates,
                            (size_t)tt->m_n_puts,
                            lagopus_chrono_t, -1LL, NULL) !=
          (lagopus_result_t)tt->m_n_puts) {
        goto done;
      }

      now = -1LL;
      if (lagopus_bbq_put(&(tt->m_q), &now, lagopus_chrono_t, -1LL) !=
          LAGOPUS_RESULT_OK) {
        goto done;
      }
      ret = tt->m_n_puts;

      lagopus_msg_debug(1, "Done, ret = " PFSZS(020, d)
                        ", req = " PFSZS(020, u) ".\n",
                        (size_t)ret, (size_t)tt->m_n_puts);
    }
  }

done:
  free((void *)dates);

  return ret;
}


static inline bool
s_initialize(test_thread_t tt,
             lagopus_mutex_t start_lock,
             lagopus_bbq_t bbq,
             ssize_t n,
             const char *name) {
  bool ret = false;

  if (tt != NULL) {
    lagopus_result_t r;
    if ((r = lagopus_thread_create((lagopus_thread_t *)&tt,
                                   s_test_thread_main,
                                   s_test_thread_finalize,
                                   s_test_thread_destroy,
                                   name, NULL)) != LAGOPUS_RESULT_OK) {
      lagopus_perror(r);
      goto done;
    }
    tt->m_start_lock = start_lock;
    tt->m_q = bbq;
    tt->m_n_puts = n;
    ret = true;
  }
done:
  return ret;
}


static inline bool
test_thread_create(test_thread_t *ttptr,
                   lagopus_mutex_t start_lock,
                   lagopus_bbq_t bbq,
                   ssize_t n,
                   const char *name) {
  bool ret = false;

  if (ttptr != NULL) {
    test_thread_t tt;
    if (*ttptr == NULL) {
      tt = (test_thread_t)malloc(sizeof(*tt));
      if (tt == NULL) {
        goto done;
      }
    } else {
      tt = *ttptr;
    }
    ret = s_initialize(tt, start_lock, bbq, n, name);
    if (*ttptr == NULL) {
      *ttptr = tt;
    }
  }

done:
  return ret;
}





static lagopus_mutex_t start_lock = NULL;
static lagopus_mutex_t stop_lock = NULL;


static test_thread_record *tts = NULL;
static size_t n_created_thds = 0;
static lagopus_bbq_t *bbqs = NULL;
static size_t n_created_bbqs = 0;
static lagopus_qmuxer_poll_t *polls = NULL;
static size_t n_created_polls = 0;
static lagopus_qmuxer_t qmx = NULL;


static bool is_child = false;
static bool is_signaled = false;
static bool is_stopped = false;


static inline void
s_stop_all(void) {
  if (is_child == true) {

    if (stop_lock != NULL) {

      (void)lagopus_mutex_lock(&stop_lock);
      {
        if (is_stopped == false) {

          size_t i;

          is_stopped = true;

          /*
           * Shutdown the queues first. This should make all the
           * thread exited.
           */
          if (bbqs != NULL) {
            for (i = 0; i < n_created_bbqs; i++) {
              lagopus_bbq_shutdown(&(bbqs[i]), true);
            }
          }
        }
      }
      (void)lagopus_mutex_unlock(&stop_lock);

    }
  }
}


static inline void
s_destroy_all(void) {
  if (is_child == true) {

    s_stop_all();

    (void)lagopus_mutex_lock(&stop_lock);
    {
      size_t i;
      test_thread_t tt;

      /*
       * Wait the threads done and delete it.
       */
      if (tts != NULL) {
        for (i = 0; i < n_created_thds; i++) {
          tt = &(tts[i]);
          (void)lagopus_thread_wait((lagopus_thread_t *)&tt, -1LL);
          (void)lagopus_thread_destroy((lagopus_thread_t *)&tt);
        }
        free((void *)tts);
        tts = NULL;
      }

      if (polls != NULL) {
        for (i = 0; i < n_created_polls; i++) {
          lagopus_qmuxer_poll_destroy(&(polls[i]));
        }
        free((void *)polls);
        polls = NULL;
      }

      if (bbqs != NULL) {
        for (i = 0; i < n_created_bbqs; i++) {
          lagopus_bbq_destroy(&(bbqs[i]), true);
        }
        free((void *)bbqs);
        bbqs = NULL;
      }

      if (qmx != NULL) {
        lagopus_qmuxer_destroy(&qmx);
        qmx = NULL;
      }

      if (start_lock != NULL) {
        lagopus_mutex_destroy(&start_lock);
        start_lock = NULL;
      }
    }
    (void)lagopus_mutex_unlock(&stop_lock);

    if (stop_lock != NULL) {
      lagopus_mutex_destroy(&stop_lock);
      stop_lock = NULL;
    }
  }
}


static int
do_run(size_t nthds, ssize_t nputs) {
  int ret = 1;

  lagopus_result_t r;

  size_t i;
  size_t j;
  char thdname[16];

  lagopus_chrono_t *put_dates = NULL;

  size_t n_need_watch = 0;
  size_t n_valid_polls = 0;
  ssize_t qsz;

  test_thread_t tt;
  lagopus_bbq_t bbq;

  lagopus_chrono_t t_begin;
  lagopus_chrono_t t_end;

  lagopus_chrono_t p_begin;
  lagopus_chrono_t p_t;

  ssize_t n_gets = 0;
  lagopus_chrono_t p_min = LLONG_MAX;
  lagopus_chrono_t p_max = LLONG_MIN;
  double p_sum = 0.0;
  double p_sum2 = 0.0;

  double p_avg;
  double p_sd;

  lagopus_chrono_t t_total = 0;
  double t_avg;

  /*
   * This is only for choking clang/scan-build.
   */
  WHAT_TIME_IS_IT_NOW_IN_NSEC(t_begin);

  put_dates = (lagopus_chrono_t *)
              malloc(sizeof(lagopus_chrono_t) * (size_t)nputs);
  if (put_dates == NULL) {
    goto done;
  }

  if ((r = lagopus_mutex_create(&start_lock)) != LAGOPUS_RESULT_OK) {
    lagopus_perror(r);
    goto done;
  }
  if ((r = lagopus_mutex_create(&stop_lock)) != LAGOPUS_RESULT_OK) {
    lagopus_perror(r);
    goto done;
  }

  /*
   * Create the qmuxer.
   */
  if ((r = lagopus_qmuxer_create(&qmx)) != LAGOPUS_RESULT_OK) {
    lagopus_perror(r);
    goto done;
  }

  /*
   * Then create queues.
   */
  bbqs = (lagopus_bbq_t *)malloc(sizeof(lagopus_bbq_t) * nthds);
  if (bbqs == NULL) {
    goto done;
  }
  for (i = 0; i < nthds; i++) {
    if ((r = lagopus_bbq_create(&(bbqs[i]), lagopus_chrono_t,
                                1000LL * 1000LL,
                                NULL)) != LAGOPUS_RESULT_OK) {
      lagopus_perror(r);
      goto done;
    }
    n_created_bbqs++;
  }
  if (n_created_bbqs == 0) {
    goto done;
  }

  /*
   * Then create poll objects for the each queue.
   */
  polls = (lagopus_qmuxer_poll_t *)malloc(sizeof(lagopus_qmuxer_poll_t) *
                                          n_created_bbqs);
  if (polls == NULL) {
    goto done;
  }
  for (i = 0; i < n_created_bbqs; i++) {
    if ((r = lagopus_qmuxer_poll_create(&(polls[i]),
                                        bbqs[i],
                                        LAGOPUS_QMUXER_POLL_READABLE)) !=
        LAGOPUS_RESULT_OK) {
      lagopus_perror(r);
      goto done;
    }
    n_created_polls++;
  }
  if (n_created_polls == 0) {
    goto done;
  }

  /*
   * Then create threads for the each poll objects/queues.
   */
  tts = (test_thread_record *)malloc(sizeof(test_thread_record) *
                                     n_created_polls);
  if (tts == NULL) {
    goto done;
  }
  for (i = 0; i < n_created_polls; i++) {
    snprintf(thdname, sizeof(thdname), "putter " PFSZS(4, u), i);
    tt = &(tts[i]);
    if (test_thread_create(&tt, start_lock, bbqs[i], nputs,
                           (const char *)thdname) != true) {
      goto done;
    }
    n_created_thds++;
  }
  if (n_created_thds == 0) {
    goto done;
  }

  /*
   * Let the initiation begin.
   */

  /*
   * Ready, note that all the created threads do this lock.
   */
  (void)lagopus_mutex_lock(&start_lock);

  /*
   * Steady,
   */
  for (i = 0; i < n_created_thds; i++) {
    tt = &(tts[i]);
    if (lagopus_thread_start((lagopus_thread_t *)&tt, false) !=
        LAGOPUS_RESULT_OK) {
      (void)lagopus_mutex_unlock(&start_lock);
      goto done;
    }
  }

  fprintf(stdout, "Test for " PFSZ(u) " threads " PFSZ(u)
          " events/thdread start.\n", n_created_thds, (size_t)nputs);

  /*
   * Go.
   */
  (void)lagopus_mutex_unlock(&start_lock);

  WHAT_TIME_IS_IT_NOW_IN_NSEC(t_begin);

  while (true) {
    /*
     * Like the select(2)/poll(2), initialize poll objects before
     * checking events.
     */
    n_need_watch = 0;
    n_valid_polls = 0;
    for (i = 0; i < n_created_thds; i++) {
      /*
       * Check if the poll has a valid queue.
       */
      bbq = NULL;
      if ((r = lagopus_qmuxer_poll_get_queue(&(polls[i]), &bbq)) !=
          LAGOPUS_RESULT_OK) {
        lagopus_perror(r);
        break;
      }
      if (bbq != NULL) {
        n_valid_polls++;
      }

      /*
       * Reset the poll status.
       */
      if ((r = lagopus_qmuxer_poll_reset(&(polls[i]))) != LAGOPUS_RESULT_OK) {
        lagopus_perror(r);
        break;
      }
      n_need_watch++;
    }

    /*
     * If there are no valid queues, exit.
     */
    if (n_valid_polls == 0) {
      break;
    }

    /*
     * Wait for an event.
     *
     *	Note that we better set timeout, not waiting forever.
     */
    r = lagopus_qmuxer_poll(&qmx, (lagopus_qmuxer_poll_t * const)polls,
                            n_need_watch,
                            100LL * 1000LL * 1000LL);

    if (r > 0) {
      /*
       * Check which poll got an event. Actually, check all the queues
       * in this sample.
       */
      size_t n_actual_get = 0LL;

      for (i = 0; i < n_created_thds; i++) {

        if ((qsz = lagopus_bbq_size(&(bbqs[i]))) > 0) {

          lagopus_msg_debug(1, "Got " PFSZS(8, u) " events from the Q"
                            PFSZS(03, u) ".\n",
                            (size_t)qsz, (size_t)i);
          if ((r = lagopus_bbq_get_n(&(bbqs[i]), (void **)put_dates,
                                     (size_t)nputs, 1LL,
                                     lagopus_chrono_t,
                                     1000LL * 1000LL * 1000LL,
                                     &n_actual_get)) > 0) {
#if 1
            WHAT_TIME_IS_IT_NOW_IN_NSEC(p_begin);
#endif
            for (j = 0; j < n_actual_get; j++) {
              /*
               * In this sample, -1LL is kinda 'EOF'. Check if we got
               * the EOF.
               */
              if (put_dates[j] == -1LL) {
                /*
                 * The queue is kinda 'closed'. From now on we don't
                 * check this queue anymore. To specify this:
                 */
                lagopus_msg_debug(1, "Got an EOF from the Q" PFSZS(04, u)
                                  ".\n", i);
                goto nullify;
              }

#if 0
              WHAT_TIME_IS_IT_NOW_IN_NSEC(p_begin);
#endif

              p_t = p_begin - put_dates[j];

              if (p_t < p_min) {
                p_min = p_t;
              }
              if (p_t > p_max) {
                p_max = p_t;
              }
              p_sum += (double)p_t;
              p_sum2 += ((double)p_t * (double)p_t);
              n_gets++;
            }

          } else {
            /*
             * Something wrong for the queue. But we must not exit
             * here. Keep on checking other queues. In order to do
             * this, set NULL as the queue into the poll object for
             * the queue.
             */
            lagopus_perror(r);
          nullify:
            if ((r = lagopus_qmuxer_poll_set_queue(&(polls[i]), NULL)) ==
                LAGOPUS_RESULT_OK) {
              lagopus_msg_debug(1, "Q" PFSZS(04, u) " is not valid "
                                "anymore, ignore the queue.\n", i);
              break;
            } else {
              /*
               * There is nothing we can do now.
               */
              lagopus_perror(r);
              goto done;
            }
          }

        }

      }

    } else if (r == LAGOPUS_RESULT_TIMEDOUT) {
      lagopus_msg_debug(1, "Timedout. continue.\n");
      continue;
    } else {
      lagopus_perror(r);
      lagopus_msg_debug(1, "Break the loop due to error(s).\n");
      goto done;
    }
  }

  ret = 0;

done:

  WHAT_TIME_IS_IT_NOW_IN_NSEC(t_end);

  if (is_signaled == false) {
    fprintf(stdout, "Done.\n");
  } else {
    fprintf(stdout, "Stopped.\n");
  }

  fprintf(stdout, "Total # of the events:\t" PFSZS(22, u) "\n\n", n_gets);

  p_avg = p_sum / (double)n_gets;
  p_sd = (p_sum2 -
          2.0 * p_avg * p_sum +
          p_avg * p_avg * (double)n_gets) / (double)(n_gets - 1);
  p_sd = sqrt(p_sd);

  fprintf(stdout, "Queue stats:\n");
  fprintf(stdout, "wait time min =\t" PFSZS(22, d) " nsec.\n", p_min);
  fprintf(stdout, "wait time max =\t" PFSZS(22, d) " nsec.\n", p_max);
  fprintf(stdout, "wait time avg =\t%25.2f nsec.\n", p_avg);
  fprintf(stdout, "wait time sd =\t%25.2f.\n\n", p_sd);

  t_total = t_end - t_begin;
  t_avg = (double)t_total / (double)n_gets;

  fprintf(stdout, "Throughput:\n");
  fprintf(stdout, "total time:\t" PFSZS(22, d) " msec.\n",
          (size_t)(t_total / 1000LL / 1000LL));
  fprintf(stdout, "total avg:\t%25.2f nsec/event.\n", t_avg);

  s_destroy_all();

  free((void *)put_dates);

  return ret;
}





static void
s_sighandler(int sig) {
  lagopus_msg_debug(5, "got a signal %d\n", sig);
  if (sig == SIGINT) {
    lagopus_msg_debug(5, "WILL EXIT.\n");

    is_signaled = true;
    s_stop_all();

    lagopus_msg_debug(5, "STOPPED ALL.\n");
  }
}


int
main(int argc, const char *const argv[]) {
  size_t nthds = 8;
  ssize_t nputs = 1000LL * 1000LL * 10LL;
  pid_t c_pid;

  if (argc >= 2) {
    size_t n;
    if (lagopus_str_parse_int64(argv[1], (int64_t *)&n) ==
        LAGOPUS_RESULT_OK) {
      if (n > 0) {
        nthds = (size_t)n;
      }
    }
  }
  if (argc >= 3) {
    ssize_t n;
    if (lagopus_str_parse_int64(argv[2], (int64_t *)&n) ==
        LAGOPUS_RESULT_OK) {
      if (n > 0) {
        nputs = (ssize_t)n;
      }
    }
  }

  (void)lagopus_signal(SIGHUP, s_sighandler, NULL);
  (void)lagopus_signal(SIGINT, s_sighandler, NULL);

  c_pid = fork();
  if (c_pid == 0) {
    is_child = true;
    return do_run(nthds, nputs);
  } else if (c_pid > 0) {
    int st;
    (void)waitpid(c_pid, &st, 0);
    return 0;
  } else {
    perror("fork");
    return 1;
  }
}
