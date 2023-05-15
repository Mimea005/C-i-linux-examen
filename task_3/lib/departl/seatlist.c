#include "departl.h"

#include "include/plog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

dptl_result dptlAddToLedger(dptl_ledger *ledger, dptl_seat *seat) {
  dptl_seat *pPrev, *pCurr;

  plog("dptlAddToLedger <- ledger(&%p) seat(&%p)", ledger, seat);
  
  if (ledger == NULL || seat == NULL) {
    return DPTLSYSERR;
  }

  pPrev = pCurr = *ledger;
  if (pCurr == NULL) {
    *ledger = seat;
    return DPTLOK;
  }
  while (pCurr != NULL) {

    if (pCurr->uSeat == seat->uSeat) {
      return DPTLOCCUPIED;
    }
    else if (pCurr->uSeat > seat->uSeat) {
      if (pCurr == *ledger) *ledger = seat;
      else pPrev->pNext_seat = seat;
      seat->pNext_seat = pCurr;
      break;
    }
    else if (pCurr->pNext_seat == NULL) {
      pCurr->pNext_seat = seat;
      break;
    }
    
    pPrev = pCurr;
    pCurr = pCurr->pNext_seat;
  }

  return DPTLOK;
}

dptl_seat *dptlPeekPassenger(dptl_ledger *ledger, const char* const passenger_name) {
  dptl_seat *pCurr;
  int iName_cmp;
  ptrace("dptlPeekPassenger <- ledger(&%p) passenger_name(&%s)", ledger, passenger_name);

  if (ledger == NULL) return NULL;
  pCurr = *ledger;

  while (pCurr != NULL) {
    iName_cmp = strcmp(passenger_name, pCurr->szPassenger_name);

    if (iName_cmp == 0) {
      return pCurr;
    }
    pCurr = pCurr->pNext_seat;
  }
  
  pwarn("dptlPeekPassenger: \"%s\" not found in ledger &%p");
  return NULL;
}

dptl_seat *dptlPeekSeat(dptl_ledger *ledger, const unsigned int seat) {
  dptl_seat *pCurr;
  plog("dptlPeekSeat <- ledger(&%p) seat(%u)", ledger, seat);

  if (ledger == NULL) return NULL;
  pCurr = *ledger;

  while (pCurr != NULL) {
    if (pCurr->uSeat == seat) return pCurr;
    else if (pCurr->uSeat > seat) break;
    pCurr = pCurr->pNext_seat;
  }

  pwarn("dptlPeekPassenger: \"%s\" not found in ledger &%p");
  return NULL;
}

dptl_result dptlRemoveSeat(dptl_ledger *ledger, const unsigned int seat) {
  dptl_seat *pPrev = NULL, *pCurr;
  dptl_result result = DPTLNOTFOUND;

  plog("dptlRemoveSeat <- ledger(&%p) seat(%u)", ledger, seat);

  if (ledger == NULL) {
    perror("dptlRemoveSeat: ledger is NULL");
    return DPTLSYSERR;
  }

  pCurr = *ledger;

  // This is a repeat of the peek function, but the previous node is needed
  // to keep continuity of the linked list
  while (pCurr != NULL) {
    if (pCurr->uSeat == seat) {
      if (pCurr == *ledger) *ledger = pCurr->pNext_seat;
      else pPrev->pNext_seat = pCurr->pNext_seat;
      result = DPTLOK;
      break;
    }
    else if (pCurr->uSeat < seat) {
      pwarn("dptlRemoveSeat: Seat not found");
      result = DPTLNOTFOUND;
      break;
    }
    pPrev = pCurr;
    pCurr = pCurr->pNext_seat;
  }

  if (result == DPTLOK) {
    free(pCurr->szPassenger_name);
    free(pCurr);
  }

  return result;
}

dptl_result dptlRemovePassenger(dptl_ledger *ledger, const char* const name) {
  dptl_seat *pPrev = NULL, *pCurr;
  dptl_result result = DPTLNOTFOUND;
  int iName_cmp;

  plog("dptlRemovePassenger <- ledger(&%p) name(%s)", ledger, name);

  if (ledger == NULL) return DPTLSYSERR;
  pCurr = *ledger;

  while (pCurr != NULL) {
    iName_cmp = strcmp(pCurr->szPassenger_name, name);

    if (pCurr->uSeat == iName_cmp) {
      if (pCurr == *ledger) *ledger = pCurr->pNext_seat;
      else pPrev->pNext_seat = pCurr->pNext_seat;
      result = DPTLOK;
      break;
    }
    pPrev = pCurr;
    pCurr = pCurr->pNext_seat;
  }

  if (result == DPTLOK) {
    free(pCurr->szPassenger_name);
    free(pCurr);
  }
  else {
    pwarn("dptlRemovePassenger: Passenger not found");
  }
  return result;
}

dptl_result dptlMovePassenger(dptl_ledger *ledger, const unsigned int old_seat, const unsigned int new_seat) {
  dptl_seat *pOld_seat, *pOld_prev, **pInsert_point, *pPrev, *pCurr;
  dptl_result result = DPTLOK;
  pOld_seat = pOld_prev = pPrev = pCurr = NULL;
  pInsert_point = NULL;

  plog("dptlMovePassenger <- ledger(&%p) old_seat(%u) new_seat(%u)", ledger, old_seat, new_seat);

  if (ledger == NULL) {
    perr("dptlMovePassenger: ledger is NULL");
    return DPTLSYSERR;
  }
  pCurr = *ledger;

  if (old_seat == 0 || new_seat == 0) {
    pwarn("dptlMovePassenger: %s is 0", old_seat == 0 ? "old_seat" : "new_seat");
    return DPTLNOTFOUND; // Seat does not exist
  }


  while (pCurr != NULL) {

    if (pOld_seat == NULL) {
      if (pCurr->uSeat == old_seat) {
        ptrace("dptlMovePassenger: Found old seat at%p", pCurr);
        pOld_seat = pCurr;
        pOld_prev = pPrev;
      }
      else if (pCurr->uSeat > old_seat) {
        pwarn("dptlMovePassenger: old_seat not found");
        result = DPTLNOTFOUND;
        break;
      }
    }

    if (pInsert_point == NULL) {
      if (pCurr->uSeat > new_seat) {
        if (pPrev == NULL) {
          pInsert_point = ledger;
          ptrace("dptlMovePassenger: Inserting at head");
        }
        else {
          pInsert_point = &pPrev->pNext_seat;
          ptrace("dptlMovePassenger: Inserting after seat %u at %p", pPrev->uSeat, *pInsert_point);
        }
      } 
      else if (pCurr->uSeat == new_seat) {
        pwarn("dptlMovePassenger: New seat (%u) is occupied", new_seat);
        result = DPTLOCCUPIED;
        break;
      }
    }

    // to skip the search for the new seat later
    if (pOld_seat != NULL && pInsert_point != NULL) {
      ptrace("dptlMovePassenger: Found all needed, stopping search early");
      // Found all needed, no need to go through the rest of the list
      break;
    }
    pPrev = pCurr;
    pCurr = pCurr->pNext_seat;
  }

  if (pOld_seat == NULL) {
    pwarn("dptlMovePassenger: old_seat (%u) not found", old_seat);
    result = DPTLNOTFOUND;
  }
  
  if (result == DPTLOK) {

    if (pInsert_point == NULL) {
      ptrace("dptlMovePassenger: Insert point at end of list");
      pInsert_point = &pPrev;
    }

    if (*pInsert_point != pOld_seat) {
      ptrace("Extracting seat to change");
      // Extract seat
      if (pOld_prev != NULL) {
        pOld_prev->pNext_seat = pCurr->pNext_seat;
      }
      else {
        ptrace("dptlMovePassenger: New ledger head is %p", (*ledger)->pNext_seat);
        *ledger = (*ledger)->pNext_seat;
      }

      ptrace("Inserting in new location at %p", *pInsert_point);
      pOld_seat->pNext_seat = (*pInsert_point)->pNext_seat;
      (*pInsert_point)->pNext_seat = pOld_seat;
    }
    else {
      ptrace("dptlMovePassenger: Insert point is the same as the old seat");
    }

    pOld_seat->uSeat = new_seat;
  }
  return result;
}
