/* %COPYRIGHT% */

#ifndef __LAGOPUS_CALLOUT_TASK_FUNCS_H__
#define __LAGOPUS_CALLOUT_TASK_FUNCS_H__





/**
 *	@file	lagopus_callout_task_funcs.h
 */





#ifndef CALLOUT_TASK_T_DECLARED
typedef struct lagopus_callout_task_record 	*lagopus_callout_task_t;
#define CALLOUT_TASK_T_DECLARED
#endif /* ! CALLOUT_TASK_T_DECLARED */





/**
 * The signature of callout task functions.
 *
 *	@param[in]	arg	An argument.
 *
 *	@retval LAGOPUS_RESULT_OK	Succeeded.
 *	@retval !=LAGOPUS_RESULT_OK	Failed.
 *
 *	@returns	LAGOPUS_RESULT_OK	Succeeded.
 *	@retuens	<0			Failures.
 *
 * @details If the function returns a value other than \b
 * LAGOPUS_RESULT_OK and if the task is a periodic task, the task is
 * supposed to be cancelled and not scheduled for the next execution,
 * and the \b arg is destroyed if the freeup function is specified.
 */
typedef lagopus_result_t	(*lagopus_callout_task_proc_t)(void *arg);


/**
 * The signature of task argument freeup functions.
 *
 *	@param[in]	arg	An argument.
 */
typedef void	(*lagopus_callout_task_arg_freeup_proc_t)(void *arg);





#endif /* ! __LAGOPUS_CALLOUT_TASK_FUNCS_H__ */
