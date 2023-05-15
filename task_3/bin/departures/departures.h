#include "include/plog.h"
#ifndef _DEPARTURES_H_

#include "include/departl.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
  NOTSET,
  ADDFlight,
  PRINTFlight,
  GETFlight,
  PICKFlight,
  SRCHFlight,
  RMFlight,
  ADDPassenger,
  RMPassenger,
  MVPassenger,
  SRCHPassenger,
  QUIT,
} Action;

typedef struct _String {
  long lLen;
  char *pStr;
} String;

// Read a string from file until newline (included) or EOF
// expectedSize is used to minimize the number of reallocs
// Returns a String struct with the string and its length
String getText(FILE *stream, const int expectedSize);

/*
  schedule is the only parameter required for these functions.
  The other parameters are queried from the user if set to NULL.
*/
dptl_plane *uiAddFlight(
    dptl_schedule *schedule, 
    String *plane_id, 
    String *destination, 
    unsigned *departure_time
    );

dptl_plane *uiGetFlight(dptl_schedule *schedule, unsigned *departure_time);

dptl_plane *uiPickFlight(dptl_schedule *schedule, unsigned *departure_time);

Result uiRemoveFlight(
    dptl_schedule *schedule, 
    unsigned *flight_offset, 
    unsigned *departure_time
    );

dptl_seat *uiAddPassenger(
    dptl_schedule *schedule, 
    dptl_plane *plane, 
    String *name, 
    unsigned *age, 
    unsigned *seat
    );

Result uiRemovePassenger(
    dptl_schedule *schedule, 
    dptl_plane *plane,
    unsigned *seat
    );

Result uiMovePassenger(
    dptl_schedule *schedule,
    dptl_plane *plane,
    unsigned *old_seat, 
    unsigned *new_seat
    );

Result uiSearchPassenger(dptl_schedule *schedule, String *name);

Result uiPrintSeats(dptl_plane *plane);

/*
  all parameters are required.
*/
Result uiPrintSchedule(dptl_schedule *schedule);
Result uiEditFlight(dptl_schedule *schedule, dptl_plane *plane);

#endif // !_DEPARTURES_H_
