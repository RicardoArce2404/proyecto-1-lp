#include <ncurses.h>
#include <locale.h>
#include "string.c"
#include "array.c"
#include "ui.c"  // Ui!

int generalOpts() {
  PtrArray *opts = newPtrArray();
  ptrArrayAppend(newString("Opciones administrativas"), opts);
  ptrArrayAppend(newString("Consulta de catálogo"), opts);
  ptrArrayAppend(newString("Cotizar"), opts);
  ptrArrayAppend(newString("Modificar cotización"), opts);
  ptrArrayAppend(newString("Facturar"), opts);
  ptrArrayAppend(newString("Salir"), opts);

  int selectedOpt = showMenu(opts);
  for (int i = 0; i < opts->len; i++) deleteString(opts->data[i]);
  deletePtrArray(opts);
  return selectedOpt;
}

// TO DO.
int adminOpts() {
  return 0;
}

// TO DO.
int catalogQuery() {
  return 0;
}

// TO DO.
int makeQuotation() {
  return 0;
}

// TO DO.
int editQuotation() {
  return 0;
}

// TO DO.
int makeInvoice() {
  return 0;
}


int main() {
  setlocale(LC_CTYPE, "");
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  int selectedOpt = -1;
  while (selectedOpt != 5) {
    selectedOpt = generalOpts();
    switch (selectedOpt) {
      case 0:
        adminOpts();
        break;
      case 1:
        catalogQuery();
        break;
      case 2:
        makeQuotation();
        break;
      case 3:
        editQuotation();
        break;
      case 4:
        makeInvoice();
        break;
    }
  }
  

  refresh();
  endwin();
  return 0;
}
