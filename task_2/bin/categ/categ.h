/*
 * Writes the list of numbers from an input file together
 * with some metadata for each number
*/
#ifndef _CATEG_H_

#define __STRICT_ANSI__
#include "stdbool.h"
#include "stdio.h"

struct TASK2_NUMBER_METADATA {
  int iIndex; // The index of the number, first = 1
  int iNumber; // The number, as read from the input file
  bool bIsFibonnacii;
  bool bIsPrimeNumber;
  bool bIsSquareNumber;
  bool bIsCubeNumber;
  bool bIsPerfectNumber;
  bool bIsAbundantNumber;
  bool bIsOddNumber;
};

struct TASK2_NUMBER_METADATA metaNumber(int n);

#endif // !_CATEG_H_
