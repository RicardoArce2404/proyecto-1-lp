#ifndef STRING
#define STRING
#include <stdlib.h>
#include <string.h> // This is the built-in string.h, only needed for memcpy.

typedef struct String {
  char *text;
  int len;
} String;

String *newString(char *originalString) {
  int len = 0;
  while (originalString[len] != '\0') {
    len++;
  }
  if (len == 0) {
    return NULL;
  }
  if (len == 0) {
    return NULL;
  }
  char *text = (char *)malloc(sizeof(char) * len);
  if (text == NULL) {
    return NULL;
  }
  String *str = (String *)malloc(sizeof(String));
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
  if (ptr == NULL) {
    return;
  }
  free(ptr->text);
  free(ptr);
}

// The len field in a String struct contains the number of bytes that the string
// needs to be stored. Accentuated characters (á, é, etc) need 2 bytes to be
// stored. In UI function the number of characters is needed, this function
// returns that.
int getVisualLen(String *str) {
  int len = 0;
  for (int i = 0; i < str->len; i++) {
    if (str->text[i] != -61) {
      len++;
    }
  }
  return len;
}

int isNumber(String *str) {
  for (int i = 0; i < str->len; i++) {
    int letterValue = (int)(str->text[i]);
    // In ASCII, numeric characters are between 48 and 57.
    if (letterValue < 48 || letterValue > 57) {
      return 0;
    }
  }
  return 1;
}

int toInt(String *str) {
  char *buffer = malloc(str->len + 1);
  memcpy(buffer, str->text, str->len);
  buffer[str->len] = '\0';
  int value = atoi(buffer);
  free(buffer);
  return value;
}

#endif
