#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdarg.h>

// macro for debug
#ifdef DEBUG
#define DPRINT(fmt, ...) debug_printf(fmt, __FILE__, __LINE__, __VA_ARGS__)
#else
#define DPRINT(fmt, ...) 
#endif

// ============================================================================
// prototype functions
// ============================================================================
extern void debug_printf(FILE *fp, char *file, int line, char *fmt, ...);

#endif  // __DEBUG_H__