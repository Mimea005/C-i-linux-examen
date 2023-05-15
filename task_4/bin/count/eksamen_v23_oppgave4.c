#include <linux/limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "include/plog.h"

#define BUFFER_SIZE 4096
#define NUM_THREADS 2
#define BYTE_RANGE 127-32 // 36-126 are the printable characters in ASCII

struct Buffer {
  unsigned char buffer[BUFFER_SIZE];
  int bytes_in_buffer;
};

struct CommonArgs {
  sem_t buffer_ready;
  sem_t buffer_empty;
  struct Buffer buffers[2];
  int filled_buffer;
};

struct ThreadArgsA {
  struct CommonArgs *com;
  char* filename;
};

enum tracked_words {
  AND=0,
  AT, 
  IT, 
  MY,
  HAMLET,
  LAST // used to get the number of tracked words
};

const char* tracked_words[] = {
  "and",
  "at",
  "it",
  "my", 
  "Hamlet"
};

void* thread_A(void *args) {
  int bContinue = 1;
  int iSemValue;
  struct ThreadArgsA *arg = (struct ThreadArgsA*)args;
  struct Buffer *buffer;
  int iCurrentBuffer = 0;
  FILE* fp = fopen(arg->filename, "rb");

  if (!fp) {
    if (errno == ENOENT) {
      printf("File not found: %s\n", arg->filename);
    }
    perror("Failed to open file");
    exit(EXIT_FAILURE);
  }

  do {
    buffer = &arg->com->buffers[iCurrentBuffer]; // Which buffer to fill
    if (buffer->bytes_in_buffer > 0) { // Done filling up, waits for thread_B to swap buffers
      sem_wait(&arg->com->buffer_empty);

      ptrace("thread_A: Filled buffer is %i", iCurrentBuffer);
      arg->com->filled_buffer = iCurrentBuffer;
      iCurrentBuffer = !iCurrentBuffer;
      buffer = &arg->com->buffers[iCurrentBuffer];

      // signal that a buffer is ready
      sem_post(&arg->com->buffer_ready);
    }

    int read_bytes = fread(buffer->buffer + buffer->bytes_in_buffer, 1, BUFFER_SIZE - buffer->bytes_in_buffer, fp);
    buffer->bytes_in_buffer += read_bytes;

    // Check if we have reached the end of the file
    if (read_bytes == 0) {

      // signal that there is no more data to be read
      buffer->bytes_in_buffer = 0;
      sem_wait(&arg->com->buffer_empty);
      arg->com->filled_buffer = iCurrentBuffer;
      sem_post(&arg->com->buffer_ready);

      break;
    }
    plog("thread_A: buffer %i filled", iCurrentBuffer);
  } while (1);

  if (feof(fp) != 0) {
    plog("Finished reading file");
  } else {
    fclose(fp);
    perror("Failed to read file");
    exit(EXIT_FAILURE);
  }
  fclose(fp);
  pthread_exit(NULL);
}

void* thread_B(void *args) {
  int bContinue = 1;
  int count[BYTE_RANGE + LAST];
  struct Buffer *buffer;
  struct CommonArgs *arg = (struct CommonArgs*)args;
  memset(count, 0, sizeof(count));

  while (1) {
    // Wait until a buffer is ready to be processed
    sem_wait(&arg->buffer_ready);

    buffer = &arg->buffers[arg->filled_buffer];
    ptrace("thread_B: bytes_in_buffer[%i] = %d", arg->filled_buffer, buffer->bytes_in_buffer);

    // If the buffer is empty, we are done
    if (buffer->bytes_in_buffer == 0)
      break;

    for (int i = 0; i < buffer->bytes_in_buffer; i++) {
      if (buffer->buffer[i] < 32 && buffer->buffer[i] > 126) continue; // skip non-printable characters
      count[buffer->buffer[i]-32]++;

      // Check for tracked words
      if (buffer->bytes_in_buffer - i > 2) {
        if (strncasecmp(tracked_words[AT], (char*)&buffer->buffer[i], 2) == 0) count[BYTE_RANGE+ AT]++;
        if (strncasecmp(tracked_words[IT], (char*)&buffer->buffer[i], 2) == 0) count[BYTE_RANGE+ IT]++;
        if (strncasecmp(tracked_words[MY], (char*)&buffer->buffer[i], 2) == 0) count[BYTE_RANGE+ MY]++;
      }
      if (buffer->bytes_in_buffer - i > 3) {
        if (strncasecmp(tracked_words[AND], (char*)&buffer->buffer[i], 3) == 0) count[BYTE_RANGE+ AND]++;
      }
      if (buffer->bytes_in_buffer - i > 6) {
        if (strncasecmp(tracked_words[HAMLET], (char*)&buffer->buffer[i], 6) == 0) count[BYTE_RANGE+ HAMLET]++;
      }

    }
    plog("thread_B: buffer emptied");
    buffer->bytes_in_buffer = 0;

    // signal that buffer is empty
    sem_post(&arg->buffer_empty);
  }

  // Print the results
  for (int i = 0; i < BYTE_RANGE; i++)
    printf("%c (%i): %d\n", i+32, i+32, count[i]);

  printf("\nTracked words:\n");
  for (int i = 0; i < LAST; i++)
    printf("%s: %d\n", tracked_words[i], count[BYTE_RANGE+i]);
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  pthread_t threadA, threadB;
  struct CommonArgs shared_vars = {0};
  struct ThreadArgsA threadA_args = {0};

  plog_init(NULL); // initialize logging

  // Check for filename argument
  if (argc < 2) {
    printf("Usage: count <filename>\n");
    return 1;
  }

  // The byte buffer in shared_vars starts empty so semBuffer_ready is initialized to 0
  // and semBuffer_empty is initialized to 1
  sem_init(&shared_vars.buffer_ready, 0, 0);
  sem_init(&shared_vars.buffer_empty, 0, 1);
  threadA_args.com = &shared_vars;

  threadA_args.filename = argv[1];

  if (pthread_create(&threadA, NULL, thread_A, (void*)&threadA_args) != 0) {
    perror("Could not create thread A");
    exit(1);
  }

  if (pthread_create(&threadB, NULL, thread_B, (void*)&shared_vars) != 0) {
    perror("Could not create thread B");
    exit(1);
  }

  if (pthread_join(threadA, NULL) != 0) {
    perror("Could not join thread A");
    exit(1);
  }
  plog("Thread A joined");
  if (pthread_join(threadB, NULL) != 0) {
    perror("Could not join thread B");
    exit(1);
  }
  plog("Thread B joined");

  sem_destroy(&shared_vars.buffer_ready);
  sem_destroy(&shared_vars.buffer_empty);
  return 0;
}










