#include <ncursesw/curses.h>
#include <locale.h>
#include "string.c"
#include "array.c"
#include "ui.c"  // Ui!



// TO DO.
void catalogQuery() {
}

// TO DO.
void makeQuotation() {
}

// TO DO.
void editQuotation() {
}

// TO DO.
void makeInvoice() {
}

// TO DO.
void registerProductFamily() {
  String *filename = showInput(UiTextInput, 3);
  move(20, 1);
  printw("%.*s", filename->len, filename->text);
  deleteString(filename);
  // TO DO: read file and add families
}

// TO DO.
void registerProduct() {
}

// TO DO.
void LoadInventory() {
}

// TO DO.
void queryInvoice() {
}

// TO DO.
void statistics() {
}

void adminOpts() {
  PtrArray *opts = newPtrArray();
  ptrArrayAppend(newString("Registrar familia de productos"), opts);
  ptrArrayAppend(newString("Registrar producto"), opts);
  ptrArrayAppend(newString("Cargar inventario"), opts);
  ptrArrayAppend(newString("Consultar factura"), opts);
  ptrArrayAppend(newString("Estadísticas"), opts);
  ptrArrayAppend(newString("Regresar"), opts);

  int selectedOpt = 0;
  while (selectedOpt != 5) {
    selectedOpt = showMenu(opts);
    switch (selectedOpt) {
      case 0:
        registerProductFamily();
        break;
      case 1:
        registerProduct();
        break;
      case 2:
        LoadInventory();
        break;
      case 3:
        queryInvoice();
        break;
      case 4:
        statistics();
        break;
    }
  }

  for (int i = 0; i < opts->len; i++) deleteString(opts->data[i]);
  deletePtrArray(opts);

}

void generalOpts() {
  PtrArray *opts = newPtrArray();
  ptrArrayAppend(newString("Opciones administrativas"), opts);
  ptrArrayAppend(newString("Consulta de catálogo"), opts);
  ptrArrayAppend(newString("Cotizar"), opts);
  ptrArrayAppend(newString("Modificar cotización"), opts);
  ptrArrayAppend(newString("Facturar"), opts);
  ptrArrayAppend(newString("Salir"), opts);

  int selectedOpt = 0;
  while (selectedOpt != 5) {
    selectedOpt = showMenu(opts);
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

  for (int i = 0; i < opts->len; i++) deleteString(opts->data[i]);
  deletePtrArray(opts);
}

int main() {
  setlocale(LC_CTYPE, "");
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  generalOpts();

  refresh();
  endwin();
  return 0;
}
