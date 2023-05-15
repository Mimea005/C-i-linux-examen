#pragma once
#ifndef _DPARTL_H_
#include <stdbool.h>

typedef struct _dptl_seat {
  unsigned int uSeat;
  char *szPassenger_name;
  unsigned int uPassenger_age;
  struct _dptl_seat *pNext_seat;
} dptl_seat;
typedef dptl_seat* dptl_ledger;

typedef enum {
  DPTLOK, // Everything good
  DPTLSYSERR, // Error spesified with errno
  DPTLOCCUPIED, // Seat specified is occupied
  DPTLNOTFOUND, // Seat specified is not a node
} dptl_result;

// Add a seat to the ledger sorted by seat the uSeat field
// If the seat is already occupied, return DPTLOCCUPIED
// If ledger or seat are NULL, return DPTLSYSERR
dptl_result dptlAddToLedger (dptl_ledger *ledger, dptl_seat *seat);

// Retrieve a seet from the ledger by passenger_name
// If the seat is not found, return NULL
dptl_seat* dptlPeekPassenger(dptl_ledger *ledger, const char* const passenger_name);

// Retrieve a seet from the ledger by seat number
// If the seat is not found, return NULL
dptl_seat* dptlPeekSeat(dptl_ledger *ledger, const unsigned int seat);

// Remove a seat from the ledger by seat_number
// This can return DPTLNOTFOUND or DPTLOK
dptl_result dptlRemoveSeat (dptl_ledger *ledger, const unsigned int seat_number);

// Remove a seat from the ledger by passenger_name
dptl_result dptlRemovePassenger (dptl_ledger *ledger, const char* const passenger_name);

// Move a passenger from one seat to another
// This can return DPTLNOTFOUND, DPTLOCCUPIED, or DPTLOK
dptl_result dptlMovePassenger (dptl_ledger *ledger, const unsigned int old_seat, const unsigned new_seat);


typedef struct _dptl_plane {
  struct _dptl_plane *pPrev_plane;
  struct _dptl_plane *pNext_plane;
  char *czFlight_id;
  char *czDestination;
  unsigned int uSeats;
  unsigned int uDeparture_time;
  dptl_seat *dsPassengers;
} dptl_plane;

typedef dptl_plane* dptl_schedule;

// Add a plane to the schedule sorted by departure time
dptl_result dptlAddToSchedule (dptl_schedule *schedule, dptl_plane *plane);

// Retrieve a plane from the schedule by the offset relative to the first plane
dptl_plane *dptlPeekScheduleByOffset (dptl_schedule *schedule, int offset);

// Retrieve a plane from the schedule by the offset relative to the first plane
dptl_result dptlDeleteFromSchedule (dptl_schedule *schedule, int offset);

#endif // !_DPARTL_H_
