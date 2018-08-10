/* %COPYRIGHT% */

#ifndef __LAGOPUS_LOGGER_H__
#define __LAGOPUS_LOGGER_H__





/**
 * @file	lagopus_logger.h
 */





typedef enum {
  LAGOPUS_LOG_LEVEL_UNKNOWN = 0,
  LAGOPUS_LOG_LEVEL_DEBUG,
  LAGOPUS_LOG_LEVEL_INFO,
  LAGOPUS_LOG_LEVEL_NOTICE,
  LAGOPUS_LOG_LEVEL_WARNING,
  LAGOPUS_LOG_LEVEL_ERROR,
  LAGOPUS_LOG_LEVEL_FATAL
} lagopus_log_level_t;


typedef enum {
  LAGOPUS_LOG_EMIT_TO_UNKNOWN = 0,
  LAGOPUS_LOG_EMIT_TO_FILE,
  LAGOPUS_LOG_EMIT_TO_SYSLOG
} lagopus_log_destination_t;





__BEGIN_DECLS


/**
 * Initialize the logger.
 *
 *	@param[in]	dst	Where to log;
 *	\b LAGOPUS_LOG_EMIT_TO_UNKNOWN: stderr,
 *	\b LAGOPUS_LOG_EMIT_TO_FILE: Any regular file,
 *	\b LAGOPUS_LOG_EMIT_TO_SYSLOG: syslog
 *	@param[in]	arg	For \b LAGOPUS_LOG_EMIT_TO_FILE: a file name,
 *	for \b LAGOPUS_LOG_EMIT_TO_SYSLOG: An identifier for syslog.
 *	@param[in]	multi_process	If the \b dst is
 *	\b LAGOPUS_LOG_EMIT_TO_FILE, use \b true if the application shares
 *	the log file between child processes.
 *	@param[in]	emit_date	Use \b true if date is needed in each
 *	line header.
 *	@param[in]	debug_level	A debug level.
 *
 *	@retval	LAGOPUS_RESULT_OK		Succeeded.
 *	@retval	LAGOPUS_RESULT_ANY_FAILURES	Failed.
 */
lagopus_result_t
lagopus_log_initialize(lagopus_log_destination_t dst,
                       const char *arg,
                       bool multi_process,
                       bool emit_date,
                       uint16_t debug_level);


/**
 * Re-initialize the logger.
 *
 *	@details Calling this function implies 1) close opened log
 *	file. 2) re-open the log file, convenient for the log rotation.
 *
 *	@retval	LAGOPUS_RESULT_OK		Succeeded.
 *	@retval	LAGOPUS_RESULT_ANY_FAILURES	Failed.
 */
lagopus_result_t	lagopus_log_reinitialize(void);


/**
 * Finalize the logger.
 */
void	lagopus_log_finalize(void);


/**
 * Set the debug level.
 *
 *	@param[in]	lvl	A debug level.
 */
void	lagopus_log_set_debug_level(uint16_t lvl);


/**
 * Get the debug level.
 *
 *	@returns	The debug level.
 */
uint16_t	lagopus_log_get_debug_level(void);


/**
 * Check where the log is emitted to.
 *
 *	@param[out]	A pointer to the argument that was passed via the \b lagopus_log_get_destination(). (NULL allowed.)
 *
 *	@returns The log destination.
 *
 *	@details If the \b arg is specified as non-NULL pointer, the
 *	returned \b *arg must not be free()'d nor modified.
 */
lagopus_log_destination_t
lagopus_log_get_destination(const char **arg);


/**
 * The main logging workhorse: not intended for direct use.
 */
void	lagopus_log_emit(lagopus_log_level_t log_level,
                       uint64_t debug_level,
                       const char *file,
                       int line,
                       const char *func,
                       const char *fmt, ...)
__attr_format_printf__(6, 7);


__END_DECLS





#ifdef __GNUC__
#define __PROC__	__PRETTY_FUNCTION__
#else
#define	__PROC__	__func__
#endif /* __GNUC__ */


/**
 * Emit a debug message to the log.
 *
 *	@param[in]	level	A debug level (int).
 */
#define lagopus_msg_debug(level, ...) \
  lagopus_log_emit(LAGOPUS_LOG_LEVEL_DEBUG, (uint64_t)(level), \
                   __FILE__, __LINE__, __PROC__, __VA_ARGS__)


/**
 * Emit an informative message to the log.
 */
#define lagopus_msg_info(...) \
  lagopus_log_emit(LAGOPUS_LOG_LEVEL_INFO, 0LL, __FILE__, __LINE__, \
                   __PROC__, __VA_ARGS__)


/**
 * Emit a notice message to the log.
 */
#define lagopus_msg_notice(...) \
  lagopus_log_emit(LAGOPUS_LOG_LEVEL_NOTICE, 0LL, __FILE__, __LINE__, \
                   __PROC__, __VA_ARGS__)


/**
 * Emit a warning message to the log.
 */
#define lagopus_msg_warning(...) \
  lagopus_log_emit(LAGOPUS_LOG_LEVEL_WARNING, 0LL, __FILE__, __LINE__, \
                   __PROC__, __VA_ARGS__)


/**
 * Emit an error message to the log.
 */
#define lagopus_msg_error(...) \
  lagopus_log_emit(LAGOPUS_LOG_LEVEL_ERROR, 0LL, __FILE__, __LINE__, \
                   __PROC__, __VA_ARGS__)


/**
 * Emit a fatal message to the log.
 */
#define lagopus_msg_fatal(...) \
  lagopus_log_emit(LAGOPUS_LOG_LEVEL_FATAL, 0LL, __FILE__, __LINE__, \
                   __PROC__, __VA_ARGS__)


/**
 * Emit an arbitarary message to the log.
 */
#define lagopus_msg(...) \
  lagopus_log_emit(LAGOPUS_LOG_LEVEL_UNKNOWN, 0LL, __FILE__, __LINE__, \
                   __PROC__, __VA_ARGS__)


/**
 * The minimum level debug emitter.
 */
#define lagopus_dprint(...) \
  lagopus_msg_debug(1LL, __VA_ARGS__)


/**
 * Emit a readable error message for the errornous result.
 *
 *	@param[in]	s	A result code (lagopus_result_t)
 */
#define lagopus_perror(s) \
  do {                                                                  \
    (s == LAGOPUS_RESULT_POSIX_API_ERROR) ?                             \
    lagopus_msg_error("LAGOPUS_RESULT_POSIX_API_ERROR: %s.\n",      \
                      strerror(errno)) :                            \
    lagopus_msg_error("%s.\n", lagopus_error_get_string((s)));      \
  } while (0)


/**
 * Emit an error message and exit.
 *
 *	@param[in]	ecode	An exit code (int)
 */
#define lagopus_exit_error(ecode, ...) {        \
    lagopus_msg_error(__VA_ARGS__);             \
    exit(ecode);                                \
  }


/**
 * Emit a fatal message and abort.
 */
#define lagopus_exit_fatal(...) {                   \
    lagopus_msg_fatal(__VA_ARGS__);                 \
    abort();                                        \
  }





#endif /* ! __LAGOPUS_LOGGER_H__ */
