#ifndef STRING
#define STRING
#include <stdlib.h>
#include <string.h> // This is the built-in string.h, only needed for memcpy.

typedef struct String {
  char *text;
  int len;
} String;

// Creates a String struct using the null-terminated string originalString.
String *newString(char *originalString) {
  int len = 0;
  while (originalString[len] != '\0') {
    len++;
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

// Creates a String struct using n characters from string originalString.
String *newStringN(char *originalString, int n) {
  if (!originalString || n <= 0) return NULL;
    
    char *text = (char *)malloc(n + 1); // +1 para el null terminator
    if (!text) return NULL;
    
    memcpy(text, originalString, n);
    text[n] = '\0'; // Asegurar null termination
    
    String *str = (String *)malloc(sizeof(String));
    if (!str) {
        free(text);
        return NULL;
    }
    
    str->text = text;
    str->len = n;
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
  if (!str || !str->text) return 0;
  
  int hasDecimal = 0;
  for (int i = 0; i < str->len; i++) {
      // Permitir signo negativo solo al inicio
      if (i == 0 && str->text[i] == '-') {
          continue;
      }
      // Permitir un solo punto decimal
      if (str->text[i] == '.') {
          if (hasDecimal) return 0; // Más de un punto
          hasDecimal = 1;
          continue;
      }
      if (str->text[i] < '0' || str->text[i] > '9') {
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
