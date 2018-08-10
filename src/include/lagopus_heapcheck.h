/* %COPYRIGHT% */

#ifndef __LAGOPUS_HEAPCHECK_H__
#define __LAGOPUS_HEAPCHECK_H__





__BEGIN_DECLS





void
lagopus_heapcheck_module_initialize(void);

bool
lagopus_heapcheck_is_in_heap(const void *addr);

#if 0
bool
lagopus_heapcheck_is_mallocd(const void *addr);
#endif





__END_DECLS





#endif /* __LAGOPUS_HEAPCHECK_H__ */

