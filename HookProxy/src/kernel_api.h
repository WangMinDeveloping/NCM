// =========================================
// kernel_api.h.
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
	typedef unsigned long int size_t;
#define KERN_INFO "<6>"   
#define NULL 0UL  
	extern void printk(const char *fmt, ...);
	extern void *my_alloc(size_t size);
	extern void my_free(void *p);
#ifdef __cplusplus
}
#endif
