#ifndef STRING
#define STRING
#include <stdlib.h>
#include <string.h>  // This is the built-in string.h, only needed for memcpy.

typedef struct String {
  char *text;
  int len;
} String;

String *newString(char *originalString) {
  int len = 0;
  while (originalString[len] != '\0') len++;
  if (len == 0) {
    return NULL;
  }
  if (len == 0) {
    return NULL;
  }
  char *text = (char*)malloc(sizeof(char) * len);
  if (text == NULL) {
    return NULL;
  }
  String *str = (String*)malloc(sizeof(String));
  if (str == NULL) {
    free(text);
    return NULL;
  }
  memcpy(text, originalString, len);
  str->text = text;
  str->len = len;
  return str;
}

void deleteString(String *ptr) {
  free(ptr->text);
  free(ptr);
}

#endif
