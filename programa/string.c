#include <stdlib.h>
#include <string.h>

#ifndef STRING
#define STRING

typedef struct String {
  char *text;
  int len;
} String;

String *newString(char *originalString) {
  String *str = (String*)malloc(sizeof(String));
  if (str == NULL) {
    return NULL;
  }
  int len = 0;
  while (originalString[len] != '\0') {
    len++;
  }
  char *text = (char*)malloc(sizeof(char) * len);
  if (text == NULL) {
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
