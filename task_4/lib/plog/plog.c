#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "plog.h"

static int PlogInit = 0;

// sets logdir based on config file
int parseConfig(char *cfgpath, char logdir[256]);

Result plog_init(char* cfgpath) {
  Result result = Ok;
  char logdir[256] = {0};
  char *logpath, *logfile;
  int logfilesize, cfgparse;
  time_t epctime = time(NULL);
  struct tm *lctime = localtime(&epctime); // WARNING: localtime is not threadsafe
  FILE *cfgfile;

  if (cfgpath == NULL) cfgpath = _PLOG_CFG;

  cfgparse = parseConfig(cfgpath, logdir);
  if (cfgparse == 0) {

    //  mallocs logpath + sizeof("year:mon:day") + log.log\0
    logfilesize = sizeof(char) * ((4 + 2 + 2) + 8);
    logfile = malloc(logfilesize);
    if (logfile == NULL) return Err; // most likely an out of memory issue
    memset(logfile, 0, logfilesize);

    sprintf(logfile, "%s/log%04i-%02i-%02i.log", logdir, lctime->tm_year + 1900, lctime->tm_mon, lctime->tm_mday); // + 1900 since tm_year is years since 1900

    if (freopen(logfile, "a", stderr) == NULL) {
      result = Err;
    }
    free(logfile);
  }
  else //if (cfgparse != -1 && cfgparse != ENOENT)
    result = Err; // File exists but failed to open

  if (result == Ok) {
    fprintf(stderr, "\n\n-------- %02i:%02i:%02i -------\n", lctime->tm_hour, lctime->tm_min, lctime->tm_sec);
    PlogInit = 1;
  }

  return result;
}

void plogwrite(_PlogLevel level, int line, char *file, char *fmt, ...) {
  va_list args;

  if (PlogInit == 0) return;

  switch (level) {
    case LOG:
      fprintf(stderr, "[%s:%i LOG] ", file, line);
      break;
    case WARN:
      fprintf(stderr, "[%s:%i WARN] ", file, line);
      break;
    case ERROR:
      fprintf(stderr, "[%s:%i ERROR] ", file, line);
      break;
    case TRACE:
      fprintf(stderr, "[%s:%i TRACE] ", file, line);
      break;
    default:
      fprintf(stderr, "[%s:%i] ", file, line);
      break;
  }

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
  fflush(stderr);
}

int parseConfig(char *cfgpath, char logdir[256]) {
  FILE *cfgfile;
  int logdirlast;
  int result = 0;
  
  errno = 0;
  if (cfgpath == NULL) return result = -1;

  cfgfile = fopen(cfgpath, "r");
  if (cfgfile == NULL) return result = errno;

  if (fgets(logdir, 256, cfgfile) == NULL) result = errno;

  logdirlast = strlen(logdir)-1;
  while (isspace(logdir[logdirlast])) {
    logdir[logdirlast] = '\0';
    logdirlast--;
  }

  fclose(cfgfile);
  return result;
}
