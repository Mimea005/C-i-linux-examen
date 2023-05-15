#include "departures.h"

#include "include/departl.h"
#include "include/plog.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
  bool bRun = true;
  dptl_schedule schedule = NULL;

  // Variables to store user created or picked objects
  String sUsr_input = {0};

  if (plog_init(NULL) == Err) {
    if (errno != ENOENT)
      printf("Failed to init logger (%i)\n", errno);
  }

  do {
    if (sUsr_input.pStr != NULL) free(sUsr_input.pStr);

    printf("\
What would you like to do?\n\
\t1. Add a flight\n\
\t2. Add a passenger\n\
\t3. Display flight info\n\
\t4. Search for flights with passenger\n\
\t5. Remove a flight\n\
\t6. Exit\n");

    sUsr_input = getText(stdin, 3);
    if (sUsr_input.pStr == NULL) {
      printf("Failed to read input\n");
      return EXIT_FAILURE;
    }
    switch (strtol(sUsr_input.pStr, NULL, 10)) {
      case 1:
        uiAddFlight(&schedule, NULL, NULL, NULL);
        break;
      case 2:
        uiAddPassenger(&schedule, NULL, NULL, NULL, NULL);
        break;
      case 3:
        uiEditFlight(&schedule, NULL);
        break;
      case 4:
        uiSearchPassenger(&schedule, NULL);
        break;
      case 5:
        uiRemoveFlight(&schedule, NULL, NULL);
        break;
      case 6:
        bRun = false;
        break;
    }

    ptrace("GetAction: User input was: %s", sUsr_input.pStr);

    free(sUsr_input.pStr);
    sUsr_input.pStr = NULL;
  } while (bRun);

  while (schedule != NULL) {
    dptlDeleteFromSchedule(&schedule, 0);
  }

  return 0;
}

String getText(FILE *stream, const int expectedSize) {
  int iSizeMultiples = 1;
  String str;
  str.lLen = 1; // start at 1 because lLen includes the null terminator
  str.pStr = malloc(expectedSize * iSizeMultiples);

  if (str.pStr == NULL) {
    str.lLen = -1;
    perr("Failed to allocate memory for string %i", errno);
    return str;
  }

  memset(str.pStr, 0, expectedSize * iSizeMultiples);

  while(fgets(str.pStr + str.lLen-1, expectedSize * iSizeMultiples, stream) != NULL) {
    str.lLen += strlen(str.pStr + str.lLen-1);
    if (str.lLen < (expectedSize * iSizeMultiples) || str.pStr[str.lLen-2] == '\n') { // check if it reached eof before str was full
      return str;
    }
    else {
      iSizeMultiples++;
      str.pStr = realloc(str.pStr, expectedSize * iSizeMultiples);
      if (str.pStr == NULL) {
        perr("Failed to reallocate memory for string %i", errno);
        free(str.pStr);
        str.lLen = -1;
        return str;
      }
    }
  }

  return str;
}

