#include "departl.h"

#include "include/plog.h"

#include <stdlib.h>

dptl_result dptlAddToSchedule(dptl_schedule *schedule, dptl_plane *plane) {
  dptl_plane *pCurr;
  int iSearch_direction = 1; // -1 go from end, 1 go from start
  int iDiff_start, iDiff_end;

  plog("dptlAddToSchedule <- schedule(%p) plane(*%p)", schedule, plane);

  if (schedule == NULL) {
    perr("dptlAddToSchedule: schedule is NULL");
    return DPTLSYSERR;
  }
  else pCurr = *schedule;
  
  if (pCurr == NULL) {
    *schedule = plane;
    (*schedule)->pNext_plane = plane;
    (*schedule)->pPrev_plane = plane;
    return DPTLOK;
  }

  while (pCurr->pNext_plane != *schedule) {
    if (pCurr->uDeparture_time > plane->uDeparture_time) {
      pCurr = pCurr->pPrev_plane;
      break;
    }
    else {
      pCurr = pCurr->pNext_plane;
    }
  }

  plane->pNext_plane = pCurr->pNext_plane;
  plane->pPrev_plane = pCurr;

  if (pCurr->pNext_plane == *schedule)
    (*schedule)->pPrev_plane = plane;
  pCurr->pNext_plane = plane;

  // Move schedule pointer to the earliest plane if needed
  if (pCurr->uDeparture_time > plane->uDeparture_time) {
    *schedule = plane;
  }

  return DPTLOK;
}

dptl_plane *dptlPeekScheduleByOffset (dptl_schedule *schedule, int offset) {
  int iCurrent_index = 0;
  dptl_plane *pCurr_plane;
  plog("dptlPeekScheduleByOffset <- schedule(%p) offset(%i)", schedule, offset);

  if (schedule == NULL) {
    perr("dptlPeekScheduleByOffset: schedule is NULL");
    return NULL;
  }

  pCurr_plane = *schedule;

  if (offset < 0) {
    while (iCurrent_index > offset) {
      if (pCurr_plane->pPrev_plane == *schedule) return NULL; // Plane not found
      pCurr_plane = pCurr_plane->pPrev_plane;
      iCurrent_index--;
    }
  }
  else if (offset > 0) {
    while (iCurrent_index < offset) {
      if (pCurr_plane->pNext_plane == *schedule) return NULL; // Plane not found
      pCurr_plane = pCurr_plane->pNext_plane;
      iCurrent_index++;
    }
  }

  return pCurr_plane;
}

dptl_plane *dptlPeekScheduleByTime (dptl_schedule *schedule, unsigned time) {
  dptl_plane *pCurr_plane;
  plog("dptlPeekScheduleByTime <- schedule(%p) time(%u)", schedule, time);

  if (schedule == NULL) {
    perr("dptlPeekScheduleByTime: schedule is NULL");
    return NULL;
  }

  pCurr_plane = *schedule;
  while (pCurr_plane->pPrev_plane == *schedule) {
    if (pCurr_plane->uDeparture_time == time) return pCurr_plane; // Plane not found
    pCurr_plane = pCurr_plane->pNext_plane;
  }

  return NULL;
}

dptl_result dptlDeleteFromSchedule (dptl_schedule *schedule, int offset) {
  dptl_plane *pCurr;
  plog("dptlDeleteFromSchedule <- schedule(%p) offset(%i)", schedule, offset);

  pCurr = dptlPeekScheduleByOffset(schedule, offset);
  plog("dptlDeleteFromSchedule: Plane to delete %p", pCurr);

  if (pCurr == NULL) return DPTLNOTFOUND;

  if (pCurr == *schedule) {
    *schedule = pCurr->pNext_plane;
  }

  pCurr->pPrev_plane->pNext_plane = pCurr->pNext_plane;
  pCurr->pNext_plane->pPrev_plane = pCurr->pPrev_plane;

  if (pCurr == *schedule) { // If *schedule still is pCurr, then it was the only plane in the schedule
    *schedule = NULL;
  }

  plog("dptlDeleteFromSchedule: Freeing passenger list at %p", pCurr->dsPassengers);
  while (pCurr->dsPassengers != NULL) {
    dptlRemoveSeat(&pCurr->dsPassengers, pCurr->dsPassengers->uSeat);
  }

  if (pCurr->czDestination != NULL) free(pCurr->czDestination);
  if (pCurr->czFlight_id != NULL) free(pCurr->czFlight_id);
  free(pCurr);

  return DPTLOK;
}
