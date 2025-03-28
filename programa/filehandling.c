#ifndef FILEHANDLING
#define FILEHANDLING
#include "array.c"
#include "string.c"
#include <stdio.h>
#include <stdlib.h>

typedef struct File {
  char *content;
  int len;
} File;

void freeFile(File *ptr) {
  if (ptr == NULL) {
    return;
  }
  free(ptr->content);
  free(ptr);
}

// This function reserves 16 bytes initially to read the file and adds
// characters 1 by 1 to the buffer. If the buffer runs out of space, the buffer
// gets reallocated with twice the space and continue reading characters. When
// the reading is completed, the buffer gets reallocated with as much capacity
// as bytes were read (just to avoid wasting memory).
File *readFile(char *filename) {
  FILE *file = fopen(filename, "rb");
  int max = 1073741824; // Number of bytes in a gigabyte.
  int capacity = 16;    // The initial capacity is 16 bytes.
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
  char *buffer = malloc((filename->len + 1) * sizeof(char));
  memcpy(buffer, filename->text, filename->len);
  buffer[filename->len] = '\0';
  FILE *file = fopen(buffer, "rb");
  free(buffer);
  int max = 1073741824; // Number of bytes in a gigabyte.
  int capacity = 16;    // The initial capacity is 16 bytes.
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

// This function reads a file with comma-separated values and returns an array
// of lines where each line is an array of values.
//
// | This,is,an    |     =>     [ ["This", "is", "an"], ["example", "file."] ]
// | example,file. |
PtrArray *readCsv(String *filename) {
  File *file = readFileStr(filename);
  char *content = file->content;
  PtrArray *result = newPtrArray();
  PtrArray *line = newPtrArray();

  int s = 0; // Value start.
  int e = 0; // Value end.
  while (e < file->len) {
    if (content[e] == ',' && s != e) { // The s != e skips empty values.
      ptrArrayAppend(newStringN(content + s, e - s), line);
      s = e + 1;
      e = s;
    } else if (content[e] == '\n') {
      ptrArrayAppend(newStringN(content + s, e - s), line);
      ptrArrayAppend(line, result);
      line = newPtrArray();
      s = e + 1;
      e = s;
    }
    e++;
  }
  deletePtrArray(line); // Last line isn't used.
  freeFile(file);
  return result;
}



#endif
