/* %COPYRIGHT% */

#ifndef __LAGOPUS_TYPES_H__
#define __LAGOPUS_TYPES_H__





#ifdef __cplusplus
#define restrict /**/
#endif /* __cplusplus */





/**
 *	@file	lagopus_types.h
 */





/**
 * @details The result type.
 */
typedef int64_t lagopus_result_t;


/**
 * @details The flat nano second expression of the time, mainly
 * acquired by \b clock_gettime(). For arithmetic operations, the sign
 * extension is needed.
 */
typedef	int64_t	lagopus_chrono_t;





#endif /* ! __LAGOPUS_TYPES_H__ */
