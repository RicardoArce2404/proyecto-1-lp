#include <ncurses.h>
#include <locale.h>
#include "string.c"
#include "array.c"
#include "ui.c"  // Ui!

int main() {

  PtrArray *options = newPtrArray();
  ptrArrayAppend(newString("Opciones administrativas"), options);
  ptrArrayAppend(newString("Consulta de catálogo"), options);
  ptrArrayAppend(newString("Cotizar"), options);
  ptrArrayAppend(newString("Modificar cotización"), options);
  ptrArrayAppend(newString("Facturar"), options);
  ptrArrayAppend(newString("Salir"), options);

  setlocale(LC_CTYPE, "");
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  showMenu(options);

  refresh();
  getch();
  endwin();

  for (int i = 0; i < options->len; i++) deleteString(options->data[i]);
  deleteArray(options);
  return 0;
}
