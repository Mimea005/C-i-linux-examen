#pragma once
#ifndef _PLOG_H_
#define _PLOG_H_

/*
 *  ------------------------
 *  Includes
 *  ------------------------
 */
#include <stdio.h>

/*
 *  ------------------------
 *  Defines
 *  ------------------------
 */
#define _PLOG_CFG "./plog.cfg" // Default out directory
#define _PLOG_DIR "./log/"

/*
 * ------------------------
 * Global scope
 * ------------------------
 */


/*
 * ------------------------
 *  Types and structs
 * ------------------------
 */
typedef enum {
  Ok,
  Err
} Result;

typedef enum {
  LOG,
  WARN,
  ERROR,
  TRACE
} _PlogLevel;

Result plog_init();

void plogwrite(_PlogLevel level, int line, char* file, char *fmt, ...);

/*
 * ------------------------
 *  convinent macros
 * ------------------------
 */
#define ptrace(...) plogwrite(TRACE, __LINE__, __FILE__, __VA_ARGS__)
#define plog(...) plogwrite(LOG, __LINE__, __FILE__, __VA_ARGS__)
#define pwarn(...) plogwrite(WARN, __LINE__, __FILE__, __VA_ARGS__)
#define perr(...) plogwrite(ERROR, __LINE__, __FILE__, __VA_ARGS__)
#endif // !_PLOG_H_
