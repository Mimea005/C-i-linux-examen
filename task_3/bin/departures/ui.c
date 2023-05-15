#include "departures.h"

#include "include/departl.h"
#include "include/plog.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Result uiPrintSeats(dptl_plane *plane) {
  unsigned uSeat = 1;
  dptl_seat *pSeat;
  ptrace("uiPrintSeats <- plane(%p)", plane);

  if (plane == NULL) {
    return Err;
  }
  pSeat = plane->dsPassengers;

  for (uSeat=1; uSeat <= plane->uSeats; uSeat++) {
    printf("Seat %u: ", uSeat);

    if (pSeat != NULL && uSeat == pSeat->uSeat) {
      printf("%s\n", pSeat->szPassenger_name);
      pSeat = pSeat->pNext_seat;
    }
    else {
      printf("empty\n");
    }
  }

  return Ok;
}

Result uiPrintSchedule(dptl_schedule *schedule) {
  dptl_plane *pPlane;
  ptrace("uiPrintSchedule <- schedule(%p)", schedule);

  int index = 1;
  if (*schedule == NULL) {
    ptrace("No schedule\n");
    return Err;
  }

  pPlane = *schedule;
  do {
    printf("%03i: Time: %u Id: %s Destination: %s\n", index++, pPlane->uDeparture_time, pPlane->czFlight_id, pPlane->czDestination);
    pPlane = pPlane->pNext_plane;
  } while (pPlane != *schedule);

  return Ok;
}

dptl_plane *uiAddFlight(
  dptl_schedule *schedule, 
  String *plane_id, 
  String *destination, 
  unsigned int *departure_time
) {
  dptl_plane *pPlane = malloc(sizeof(dptl_plane));
  String sUsr_input = {0};
  char *pEnd = NULL;
  ptrace("uiAddFlight <- schedule(%p), plane_id(%p), destination(%p), departure_time(%p)", schedule, plane_id, destination, departure_time);

  if (schedule == NULL) {
    ptrace("uiAddFlight: schedule is NULL\n");
    return NULL;
  }

  if (pPlane == NULL) {
    perr("uiAddFlight: Failed to allocate memory for plane\n");
    return NULL;
  }
  memset(pPlane, 0, sizeof(dptl_plane));
  if (plane_id != NULL) pPlane->czFlight_id = plane_id->pStr;
  else {
    do {
      printf("Enter flight id: ");
      sUsr_input = getText(stdin, 3);
      if (sUsr_input.pStr == NULL) {
        perr("Failed to read flight id (%i)\n", errno);
        free(pPlane);
        return NULL;
      }
      sUsr_input.pStr[sUsr_input.lLen-2] = '\0';
      
      if (sUsr_input.pStr[0] != '\0') {
        pPlane->czFlight_id = sUsr_input.pStr;
      }
      else {
        printf("Flight id cannot be empty\n");
        free(sUsr_input.pStr);
      }

    } while (pPlane->czFlight_id == NULL);
  }

  if (destination != NULL) pPlane->czDestination = destination->pStr;
  else {
    do {
      printf("Enter destination: ");
      sUsr_input = getText(stdin, 32);
      if (sUsr_input.pStr == NULL) {
        ptrace("Failed to read destination (%i)\n", errno);
        free(pPlane->czDestination);
        free(pPlane);
        return NULL;
      }
      sUsr_input.pStr[sUsr_input.lLen-2] = '\0';

      if (sUsr_input.pStr[0] != '\0')
        pPlane->czDestination = sUsr_input.pStr;
      else {
        printf("Destination cannot be empty\n");
        free(sUsr_input.pStr);
      }
    } while (pPlane->czDestination == NULL);
  }

  if (departure_time != NULL) pPlane->uDeparture_time = *departure_time;
  else {
    do {
      printf("Enter departure time: ");
      sUsr_input = getText(stdin, 3);
      if (sUsr_input.pStr == NULL) {
        perr("Failed to read departure time (%i)\n", errno);
        free(pPlane->czDestination);
        free(pPlane->czFlight_id);
        free(pPlane);
        return NULL;
      }

      errno = 0;
      pPlane->uDeparture_time = strtoul(sUsr_input.pStr, &pEnd, 10);
      if (errno != 0) {
        perr("Failed to convert departure_time input to integer (%i)\n", errno);
        free(pPlane->czDestination);
        free(pPlane->czFlight_id);
        free(pPlane);
        free(sUsr_input.pStr);
        return NULL;
      }
      if (pEnd == sUsr_input.pStr) {
        ptrace("Got nothing converting input to departure time\n");
        free(sUsr_input.pStr);
        continue;
      }
      else {
        free(sUsr_input.pStr);
        break;
      }
    } while (true);
  }

  printf("Enter number of seats: ");
  do {
    sUsr_input = getText(stdin, 3);
    if (sUsr_input.pStr == NULL) {
      perr("Failed to read number of seats (%i)\n", errno);
      free(pPlane->czDestination);
      free(pPlane->czFlight_id);
      free(pPlane);
      return NULL;
    }

    errno = 0;
    pPlane->uSeats = strtoul(sUsr_input.pStr, &pEnd, 10);
    if (errno != 0) {
      perr("Failed to convert number of seats input to integer (%i)\n", errno);
      free(pPlane->czDestination);
      free(pPlane->czFlight_id);
      free(pPlane);
      free(sUsr_input.pStr);
      return NULL;
    }
    if (pEnd == sUsr_input.pStr) {
      ptrace("Got nothing converting input to number of seats\n");
      free(sUsr_input.pStr);
      continue;
    }
    else {
      free(sUsr_input.pStr);
      break;
    }
  } while (true);

  dptlAddToSchedule(schedule, pPlane);
  return pPlane;
}

Result uiEditFlight(dptl_schedule *schedule, dptl_plane* plane) {
  String sUsr_input = {0};
  bool bBack = false;
  ptrace("uiPrintFlight <- plane(%p)", plane);

  if (schedule == NULL) {
    ptrace("uiPrintFlight: schedule is NULL\n");
    return Err;
  }

  if (*schedule == NULL) {
    printf("No flights in schedule\n");
    return Ok;
  }

  if (plane == NULL) {
    plane = uiPickFlight(schedule, NULL);
    if (plane == NULL) {
      perr("uiPrintFlight: Failed to get flight\n");
      return Err;
    }
  }

  do {
    printf("\tFlight %s\n", plane->czFlight_id);
    printf("\tDestination: %s\n", plane->czDestination);
    printf("\tDeparture time: %u\n", plane->uDeparture_time);
    printf("\tSeats: %u\n", plane->uSeats);
    printf("\tPassengers:\n");
    uiPrintSeats(plane);

    printf("actions: (a)dd, (m)ove, (r)emove, (b)ack: ");

    sUsr_input = getText(stdin, 3);
    if (sUsr_input.pStr == NULL) {
      perr("Failed to read action (%i)\n", errno);
      return Err;
    }

    switch (sUsr_input.pStr[0]) {
      case 'a':
        uiAddPassenger(schedule, plane, NULL, NULL, NULL);
        break;
      case 'm':
        uiMovePassenger(schedule, plane, NULL, NULL);
        break;
      case 'r':
        uiRemovePassenger(schedule, plane, NULL);
        break;
      case 'b':
        bBack = true;
        break;
    }

    free(sUsr_input.pStr);
  } while (bBack == false);

  return Ok;
}

dptl_seat *uiAddPassenger(dptl_schedule *schedule, dptl_plane *plane, String *name, unsigned int *age, unsigned int *seat) {
  dptl_seat *pSeat;
  dptl_plane *pPlane;
  String sUsr_input = {0};
  char *pEnd = NULL;
  ptrace("uiAddPassenger <- schedule(%p), plane(%p), name(%p), age(%p), seat(%p)", schedule, plane, name, age, seat);

  if (schedule == NULL) {
    perr("uiAddPassenger: schedule is NULL\n");
    return NULL;
  }

  pSeat = malloc(sizeof(dptl_seat));
  if (pSeat == NULL) {
    perr("uiAddPassenger: Failed to allocate memory for seat\n");
    return NULL;
  }
  memset(pSeat, 0, sizeof(dptl_seat));

  if (name == NULL) {
    printf("Enter name: ");
    sUsr_input = getText(stdin, 32);
    if (sUsr_input.pStr == NULL) {
      perr("Failed to read name (%i)", errno);
      free(pSeat);
      return NULL;
    }
    sUsr_input.pStr[sUsr_input.lLen-2] = '\0';
    pSeat->szPassenger_name = sUsr_input.pStr;
    sUsr_input.pStr = NULL;
  }
  else pSeat->szPassenger_name = name->pStr;

  if (age == NULL) {
    printf("Enter age: ");
    do {
      sUsr_input = getText(stdin, 4);
      if (sUsr_input.pStr == NULL) {
        perr("Failed to read age (%i)\n", errno);
        free(pSeat->szPassenger_name);
        free(pSeat);
        return NULL;
      }

      errno = 0;
      pSeat->uPassenger_age = strtoul(sUsr_input.pStr, &pEnd, 10);
      if (errno != 0) {
        perr("Failed to convert age input to integer (%i)\n", errno);
        free(pSeat->szPassenger_name);
        free(pSeat);
        free(sUsr_input.pStr);
        return NULL;
      }
      else if (pEnd == sUsr_input.pStr) {
        ptrace("Got nothing converting input to age\n");
        pSeat->uPassenger_age = 0;
      }
      free(sUsr_input.pStr);
      sUsr_input.pStr = NULL;
    } while (pSeat->uPassenger_age == 0);
  }

  if (plane != NULL) pPlane = plane;
  else if (*schedule == NULL) { 
    pPlane = uiAddFlight(schedule, NULL, NULL, NULL);
    if (pPlane == NULL) {
      perr("uiAddPassenger: Failed to create flight\n");
      free(pSeat->szPassenger_name);
      free(pSeat);
      return NULL;
    }
  }
  else {
    printf("Create new flight? (y/n): ");
    sUsr_input = getText(stdin, 3);
    if (sUsr_input.pStr == NULL) {
      perr("Failed to read input (%i)\n", errno);
      free(pSeat->szPassenger_name);
      free(pSeat);
      return NULL;
    }
    if (sUsr_input.pStr[0] == 'y') {
      pPlane = uiAddFlight(schedule, NULL, NULL, NULL);
    }
    else {
      pPlane = uiGetFlight(schedule, NULL);
    }
    free(sUsr_input.pStr);
    sUsr_input.pStr = NULL;
  }

  uiPrintSeats(pPlane);
  printf("Choose seat: ");
  do {
    sUsr_input = getText(stdin, 4);
    if (sUsr_input.pStr == NULL) {
      perr("Failed to read seat (%i)\n", errno);
      free(pSeat->szPassenger_name);
      free(pSeat);
      return NULL;
    }

    errno = 0;
    pSeat->uSeat = strtoul(sUsr_input.pStr, &pEnd, 10);
    if (errno != 0) {
      perr("Failed to convert seat input to integer (%i)\n", errno);
      free(pSeat->szPassenger_name);
      free(pSeat);
      free(sUsr_input.pStr);
      return NULL;
    }
    else if (pEnd == sUsr_input.pStr) {
      ptrace("Got nothing converting input to seat\n");
    }
    else if (dptlAddToLedger(&pPlane->dsPassengers, pSeat) == DPTLOK) {
      free(sUsr_input.pStr);
      sUsr_input.pStr = NULL;
      break;

    }
  } while (true);

  return pSeat;
}

Result uiMovePassenger(dptl_schedule *schedule, dptl_plane *plane, unsigned int *old_seat, unsigned int *new_seat) {
  String sUsr_input = {0};
  char *pEnd = NULL;
  unsigned uOld_seat, uNew_seat;
  ptrace("uiMovePassenger <- schedule(%p), plane(%p), old_seat(%p), new_seat(%p)", schedule, plane, old_seat, new_seat);

  if (schedule == NULL) {
    perr("uiMovePassenger: schedule is NULL\n");
    return Err;
  }

  if (plane == NULL) {
    plane = uiGetFlight(schedule, NULL);
    if (plane == NULL) {
      perr("uiMovePassenger: Failed to get plane\n");
      return Err;
    }
  }

  if (old_seat != NULL) {
    uOld_seat = *old_seat;
  }
  else {
    ptrace("uiMovePassenger: Querying for seat to move");

    printf("Enter old seat: ");
    }
    do {
      sUsr_input = getText(stdin, 4);
      if (sUsr_input.pStr == NULL) {
        perr("Failed to read old seat (%i)\n", errno);
        free(old_seat);
        return Err;
      }

      errno = 0;
      uOld_seat = strtoul(sUsr_input.pStr, NULL, 10);
      ptrace("uiMovePassenger: old_seat = %i\n", uOld_seat);
      if (errno != 0) {
        perr("Failed to convert old seat input to integer (%i)", errno);
        free(old_seat);
        free(sUsr_input.pStr);
        return Err;
      }
      else if (pEnd == sUsr_input.pStr) {
        ptrace("Got nothing converting input to old seat\n");
      }
      else {
      }
      free(sUsr_input.pStr);
    } while (dptlPeekSeat(&plane->dsPassengers, uOld_seat) == NULL);

  if (new_seat != NULL) {
    uNew_seat = *new_seat;
  }
  else {
    printf("Enter new seat: ");
    do {
      sUsr_input = getText(stdin, 4);
      if (sUsr_input.pStr == NULL) {
        perr("Failed to read old seat (%i)\n", errno);
        free(new_seat);
        return Err;
      }

      errno = 0;
      uNew_seat = strtoul(sUsr_input.pStr, NULL, 10);
      if (errno != 0) {
        perr("Failed to convert old seat input to integer (%i)\n", errno);
        free(new_seat);
        free(sUsr_input.pStr);
        return Err;
      }
      else if (uNew_seat > plane->uSeats || uNew_seat < 1) {
        printf("Seat %i does not exist\n", uNew_seat);
      }
      else {
        free(sUsr_input.pStr);
        break;
      }

      free(sUsr_input.pStr);
    } while (true);
  }

    return dptlMovePassenger(&plane->dsPassengers, uOld_seat, uNew_seat) == DPTLOK;
}

dptl_plane *uiGetFlight(dptl_schedule *schedule, unsigned *departure_time) {
  String sUsr_input = {0};
  char *pEnd = NULL;
  dptl_plane *pPlane = NULL;
  unsigned uDeparture_time;

  if (departure_time != NULL) pPlane = uiPickFlight(schedule, departure_time);
  else {
    printf("(s)earch by time or (p)ick from all flight: ");
    do {
      sUsr_input = getText(stdin, 3);
      if (sUsr_input.pStr == NULL) {
        perr("Failed to read input (%i)\n", errno);
        return NULL;
      }

      if (sUsr_input.pStr[0] == 's') {
        free(sUsr_input.pStr);

        printf("Enter departure time: ");
        sUsr_input = getText(stdin, 5);
        if (sUsr_input.pStr == NULL) {
          perr("Failed to read departure time (%i)\n", errno);
          return NULL;
        }
        uDeparture_time = strtoul(sUsr_input.pStr, &pEnd, 10);
        if (pEnd == sUsr_input.pStr) {
          ptrace("Got nothing converting input to departure time\n");
        }
        else if (errno != 0) {
          perr("Failed to convert departure time input to integer (%i)\n", errno);
          return NULL;
        }
        else {
          pPlane = uiPickFlight(schedule, &uDeparture_time);
        }
        free(sUsr_input.pStr);
      }
      else if (sUsr_input.pStr[0] == 'p') {
        free(sUsr_input.pStr);
        pPlane = uiPickFlight(schedule, NULL);
      } 
    } while (pPlane == NULL);

  }

  return pPlane;
}

dptl_plane *uiPickFlight(dptl_schedule *schedule, unsigned *departure_time) {
  dptl_plane *pPlane = NULL, *pCurr;
  int iOffset, iSkipped;
  String sUsr_input = {0};
  char *pEnd = NULL;
  
  if (schedule == NULL) {
    perr("uiPickFlight: schedule is NULL\n");
    return NULL;
  }

  if (*schedule == NULL) {
    plog("uiPickFlight: schedule is empty\n");
    return NULL;
  }

  do {
    // Print only flights with corresponding departure time 
    pCurr = *schedule;
    iOffset = iSkipped = 0;
    do {
      if (pCurr->uDeparture_time == *departure_time) {
        iOffset++;
        printf("%i: %s -> %s\n", iOffset, pCurr->czFlight_id, pCurr->czDestination);
      }
      else iSkipped++;
    } while (pCurr != *schedule);

    printf("Enter index: ");
    sUsr_input = getText(stdin, 5);
    if (sUsr_input.pStr == NULL) {
      perr("Failed to read input\n");
      return NULL;
    }
    errno = 0;
    strtol(sUsr_input.pStr, &pEnd, 10);
    if (errno != 0) {
      perr("Failed to parse input\n");
    }
    else if (pEnd != sUsr_input.pStr) {
      pPlane = dptlPeekScheduleByOffset(schedule, strtol(sUsr_input.pStr, NULL, 10)-1 + iSkipped);
    }

    free(sUsr_input.pStr);
    sUsr_input.pStr = NULL;
  } while(pPlane == NULL);
  return pPlane;
}

Result uiSearchPassenger(dptl_schedule *schedule, String *name) {
  String sUsrInput = {0};
  dptl_plane *plane = NULL;
  dptl_seat *passenger = NULL;
  unsigned int index = 0;

  if (schedule == NULL) {
    perr("uiSearchPassenger: schedule is NULL\n");
    return Err;
  }

  if (*schedule == NULL) {
    printf("No flights in schedule\n");
    return Err;
  }

  if (name == NULL) {
    printf("Enter name: ");
    sUsrInput = getText(stdin, 50);
    if (sUsrInput.pStr == NULL) {
      perr("Failed to read input (%i)\n", errno);
      return Err;
    }
    sUsrInput.pStr[sUsrInput.lLen-2] = '\0';
    name = &sUsrInput;
  }

  plane = *schedule;
  do {
    index++;
    if (plane->dsPassengers != NULL) {
      passenger = plane->dsPassengers;
      do {
        if (strcmp(passenger->szPassenger_name, name->pStr) == 0) {
          printf("\t%i. Time: %u Flight id: %s Destination: %s\n", index, plane->uDeparture_time, plane->czFlight_id, plane->czDestination);
        }
        passenger = passenger->pNext_seat;
      } while (passenger != NULL);
      plane = plane->pNext_plane;
    }
  } while (plane != *schedule);

  return Ok;
}

Result uiRemoveFlight(dptl_schedule *schedule, unsigned int *flight_offset, unsigned int *departure_time) {
  String sUsr_input = {0};
  unsigned uOffset;
  ptrace("uiRemoveFlight <- schedule(%p) flight_offset(%p) departure_time(%p)\n", schedule, flight_offset, departure_time);

  if (schedule == NULL) {
    perr("uiRemoveFlight: schedule is NULL\n");
    return Err;
  }
  
  if (*schedule == NULL) {
    printf("Schedule is empty\n");
    pwarn("Schedule is empty\n");
    return Ok;
  }

  if (flight_offset != NULL) uOffset = *flight_offset;
  else  {
    uiPrintSchedule(schedule);
    printf("Enter flight index: ");

    sUsr_input = getText(stdin, 5);
    if (sUsr_input.pStr == NULL) {
      perr("Failed to read input (%i)\n", errno);
      return Err;
    }

    errno = 0;
    uOffset = strtoul(sUsr_input.pStr, NULL, 10)-1;
    if (errno != 0) {
      perr("Failed to convert input to integer (%i)\n", errno);
      return Err;
    }
    free(sUsr_input.pStr);
  }

  return  dptlDeleteFromSchedule(schedule, uOffset) == DPTLOK? Ok: Err;
}

Result uiRemovePassenger(dptl_schedule *schedule, dptl_plane *plane, unsigned int *seat) {
  String sUsr_input = {0};
  char *pEnd = NULL;
  unsigned uSeat;
  ptrace("uiRemovePassenger <- schedule(%p) plane(%p) seat(%p)\n", schedule, plane, seat);

  if (schedule == NULL) {
    perr("uiRemovePassenger: schedule is NULL\n");
    return Err;
  }

  if (*schedule == NULL) {
    printf("Schedule is empty\n");
    pwarn("Schedule is empty\n");
  }

  if (plane == NULL) {
    ptrace("uiRemovePassenger: Querying for plane");
    plane = uiGetFlight(schedule, NULL);
  }

  if (seat != NULL) {
    uSeat = *seat;
  }
  else {
    ptrace("uiRemovePassenger: Querying for seat");

    uiPrintSeats(plane);
    do {
      printf("Enter seat number: ");

      sUsr_input = getText(stdin, 5);
      if (sUsr_input.pStr == NULL) {
        perr("Failed to read input (%i)\n", errno);
        return Err;
      }
      errno = 0;
      uSeat = strtoul(sUsr_input.pStr, &pEnd, 10);
      ptrace("uiRemovePassenger: \"%s\" = %u", sUsr_input.pStr, uSeat);
      if (errno != 0) {
        perr("Failed to convert input to integer (%i)\n", errno);
        free(sUsr_input.pStr);
        return Err;
      }
      else if (pEnd == sUsr_input.pStr) {
        perr("Got nothing");
      }

      free(sUsr_input.pStr);
      sUsr_input.pStr = NULL;
    } while (uSeat == 0 || uSeat > plane->uSeats);
  }

  return dptlRemoveSeat(&plane->dsPassengers, uSeat) == DPTLOK? Ok: Err;
}

