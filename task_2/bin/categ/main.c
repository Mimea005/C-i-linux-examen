#include "categ.h"
#include "prewritten.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


const char* czHelp_msg = "Usage: categ INFILE [OUTFILE]\n\
\tReads numbers from a txt INFILE and writes it to [OUTFILE] in binary format\n\
\ttogether with some metadata for each number.\
";

int main(int argc, char *argv[])
{
  char czIn_file[FILENAME_MAX +1] = {0};
  char* pIn_file_type; // Used to split the filename from the file ending to generate the output filename
  char czOut_file[FILENAME_MAX +1] = {0};
  long lIn_file_len, lOut_file_len;
  FILE *fIn_File, *fOut_file;

  int iInput_number, iInput_matched;
  long lCurr_line = 0;
  struct TASK2_NUMBER_METADATA Current_number;
  
  int iTotal_numbers;
  long lBytes_read, lElements_written, lTotal_bytes_read, lTotal_bytes_written;
  lTotal_bytes_read = lTotal_bytes_written = iTotal_numbers = 0;

  // 1. Parse args
  if (argc < 2) {
    printf("Missing INFILE\n%s\n", czHelp_msg);
    return EXIT_FAILURE;
  }
  lIn_file_len = strlen(argv[1]);
  if (lIn_file_len > FILENAME_MAX) {
    printf("INFILE length is more than FILENAME_MAX (%li/%i)\n", lIn_file_len, FILENAME_MAX);
    return EXIT_FAILURE;
  }
  memcpy(czIn_file, argv[1], lIn_file_len);
  
  if (argc < 3) {

    pIn_file_type = strstr(czIn_file, ".txt"); // separate the filename from the file ending
    if (pIn_file_type == NULL && argc < 3) {
      printf("%s is not a text file, rename the file to [name].txt\n", czIn_file);
      return EXIT_FAILURE;
    }

    // generate an output filename
    memcpy(czOut_file, czIn_file, pIn_file_type - czIn_file);
    memcpy(czOut_file + (pIn_file_type - czIn_file), ".bin", 4);
  }
  else if ((lOut_file_len = strlen(argv[2])) > FILENAME_MAX) {
    printf("OUTFILE length is more than FILENAME_MAX (%li/%i)\n", lOut_file_len, FILENAME_MAX);
    return EXIT_FAILURE;
  }
  else {
    memcpy(czOut_file, argv[2], lOut_file_len);
  }


  // 2. Open INFILE & create/truncate OUTFILE
  fIn_File = fopen(czIn_file, "r");
  if (fIn_File == NULL) {
    printf("Failed to open INFILE \"%s\" (errno: %i)", czIn_file, errno);
    return EXIT_FAILURE;
  } 

  // The man page for fopen(3) mentions that 'b' in the mode string is ignored in UNIX systems
  // but may be required if a system handles text and binary files differently
  fOut_file = fopen(czOut_file, "wb"); 
  if (fIn_File == NULL) {
    printf("Failed to open OUTFILE \"%s\" (errno: %i)", czIn_file, errno);
    return EXIT_FAILURE;
  }

  // 3. Read->parse->write data

  while (feof(fIn_File) == 0) { // While not EOF
    iInput_matched = fscanf(fIn_File, "%i", &iInput_number);
    if (iInput_matched == EOF) break;
    else if (iInput_matched < 1) {
      printf("Number on line %li not valid, skipping\n", lCurr_line);
    }
    else {
      printf("[%li] %i\n", lCurr_line, iInput_number);

      Current_number = metaNumber(iInput_number);
      Current_number.iIndex = iTotal_numbers++;

      lElements_written = fwrite(&Current_number, sizeof(struct TASK2_NUMBER_METADATA), 1, fOut_file);
      if (lElements_written == 0) {
        printf("Failed to write down %i to \"%s\" (%i)\n", Current_number.iNumber, czOut_file, errno);
        return EXIT_FAILURE;
      }
      lTotal_bytes_written += (lElements_written * sizeof(struct TASK2_NUMBER_METADATA));
    }

    lCurr_line++;
  }
  
  fclose(fIn_File);
  fclose(fOut_file);

  // 4. print numbers (and bytes) read and bytes written
  
  printf("Read %i numbers from: %s\nWrote %lib to: %s\n", iTotal_numbers, czIn_file, lTotal_bytes_written, czOut_file);
  return EXIT_SUCCESS;
}

struct TASK2_NUMBER_METADATA metaNumber(int n) {
  struct TASK2_NUMBER_METADATA result;

  result.iNumber = n;
  result.bIsFibonnacii = isFibonacci(n);
  result.bIsPrimeNumber = isPrime(n);
  result.bIsSquareNumber = isSquareNumber(n);
  result.bIsCubeNumber = isCubeNumber(n);
  result.bIsPerfectNumber = isPerfectNumber(n);
  result.bIsAbundantNumber = isAbundantNumber(n);
  result.bIsOddNumber = isOdd(n);

  return result;
}
