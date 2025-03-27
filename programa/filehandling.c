#ifndef FILEHANDLING
#define FILEHANDLING
#include <stdio.h>
#include <stdlib.h>

typedef struct File {
  char *content;
  int len;
} File;

// This function reserves 16 bytes initially to read the file and adds
// characters 1 by 1 to the buffer. If the buffer runs out of space, the buffer
// gets reallocated with twice the space and continue reading characters. When
// the reading is completed, the buffer gets reallocated with as much capacity
// as bytes were read (just to avoid wasting memory).
File *readFile(char *filename) {
  FILE *file = fopen(filename, "rb");
  int max = 1073741824; // Number of bytes in a gigabyte.
  int capacity = 16;   // The initial capacity is 16 bytes.
  char *content = (char *)malloc(capacity);
  int numBytesRead = 0;

  while (numBytesRead <= max) {
    content[numBytesRead] = fgetc(file);
    if (content[numBytesRead] == EOF)
      break;
    numBytesRead++;
    if (numBytesRead == capacity) {
      capacity *= 2;
      content = (char *)realloc(content, capacity);
    }
  }
  fclose(file);
  if (numBytesRead > max) {
    return NULL;
  }
  // The buffer gets reallocated to use only the required space.
  if (numBytesRead > 0)
    content = (char *)realloc(content, numBytesRead);
  File *result = (File *)malloc(sizeof(File));
  result->content = content;
  result->len = numBytesRead;
  return result;
}

File *readFileStr(String *filename) {
  char *buffer= malloc((filename->len+1)*sizeof(char));
  memcpy(buffer,filename->text, filename->len);
  buffer[filename->len]='\0';
  FILE *file = fopen(buffer, "rb");
  int max = 1073741824; // Number of bytes in a gigabyte.
  int capacity = 16;   // The initial capacity is 16 bytes.
  char *content = (char *)malloc(capacity);
  int numBytesRead = 0;

  while (numBytesRead <= max) {
    content[numBytesRead] = fgetc(file);
    if (content[numBytesRead] == EOF)
      break;
    numBytesRead++;
    if (numBytesRead == capacity) {
      capacity *= 2;
      content = (char *)realloc(content, capacity);
    }
  }
  fclose(file);
  if (numBytesRead > max) {
    return NULL;
  }
  // The buffer gets reallocated to use only the required space.
  if (numBytesRead > 0)
    content = (char *)realloc(content, numBytesRead);
  File *result = (File *)malloc(sizeof(File));
  result->content = content;
  result->len = numBytesRead;
  return result;
}

void freeFile(File *ptr) {
  free(ptr->content);
  free(ptr);
}



#endif
