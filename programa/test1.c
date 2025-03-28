#include <stdio.h>
#include "filehandling.c"
#include "array.c"
#include "string.c"

int main() {
  String *str = newString("./files/familias.txt");
  PtrArray *file = readCsv(str);
  deleteString(str);

  for (int i = 0; i < file->len; i++) {
    PtrArray *line = file->data[i];
    for (int j = 0; j < line->len; j++) {
      String *str = line->data[j];
      printf("%.*s\n", str->len, str->text);
      String *strc = newStringN(str->text, str->len);
      deleteString(strc);
    }
    deleteStringArray(line);
  }
  deletePtrArray(file);

  return 0;
}
