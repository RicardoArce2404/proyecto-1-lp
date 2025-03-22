#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <locale.h>
#include <mysql/mysql.h>
#include <string.h>
#include "string.c"
#include "array.c"
#include "ui.c"  // Ui! Ui!
#include "login.c" 
#include "inventory.c"
#include "filehandling.c"

MYSQL *conn;

// TO DO.
void catalogQuery() {
}

void makeQuotation() {
  PtrArray *headings = newPtrArray();
  ptrArrayAppend(newString("#"), headings);
  ptrArrayAppend(newString("Encabezado1"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(5, widths);
  intArrayAppend(20, widths);

  PtrArray *row1 = newPtrArray();
  ptrArrayAppend(newString("1"), row1);
  ptrArrayAppend(newString("celda1"), row1);
  PtrArray *row2 = newPtrArray();
  ptrArrayAppend(newString("2"), row2);
  ptrArrayAppend(newString("celda2"), row2);
  PtrArray *row3 = newPtrArray();
  ptrArrayAppend(newString("3"), row3);
  ptrArrayAppend(newString("celda3"), row3);
  PtrArray *row4 = newPtrArray();
  ptrArrayAppend(newString("4"), row4);
  ptrArrayAppend(newString("celda4"), row4);
  PtrArray *row5 = newPtrArray();
  ptrArrayAppend(newString("5"), row5);
  ptrArrayAppend(newString("celda5"), row5);
  PtrArray *row6 = newPtrArray();
  ptrArrayAppend(newString("6"), row6);
  ptrArrayAppend(newString("celda6"), row6);
  PtrArray *row7 = newPtrArray();
  ptrArrayAppend(newString("7"), row7);
  ptrArrayAppend(newString("celda7"), row7);

  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  ptrArrayAppend(row1, rows);
  ptrArrayAppend(row2, rows);
  ptrArrayAppend(row3, rows);
  ptrArrayAppend(row4, rows);
  ptrArrayAppend(row5, rows);
  ptrArrayAppend(row6, rows);
  ptrArrayAppend(row7, rows);

  String *title = newString("titulo");
  showScrollableList(title, headings, rows, widths);
  deleteString(title);

  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
  }
  deletePtrArray(rows);
}

// TO DO.
void editQuotation() {
}

// TO DO.
void makeInvoice() {
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
        registerProductFamily(conn);
        break;
      case 1:
        registerProduct(conn);
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
  ptrArrayAppend(newString("Consulta de catálogo"), opts);
  ptrArrayAppend(newString("Cotizar"), opts);
  ptrArrayAppend(newString("Modificar cotización"), opts);
  ptrArrayAppend(newString("Facturar"), opts);
  ptrArrayAppend(newString("Regresar"), opts);

  int selectedOpt = 0;
  while (selectedOpt != 4) {
    selectedOpt = showMenu(opts);
    switch (selectedOpt) {
      case 0:
        catalogQuery();
        break;
      case 1:
        makeQuotation();
        break;
      case 2:
        editQuotation();
        break;
      case 3:
        makeInvoice();
        break;
    }
  }

  for (int i = 0; i < opts->len; i++) deleteString(opts->data[i]);
  deletePtrArray(opts);
}

void initialOpts(){
  PtrArray *opts = newPtrArray();
  ptrArrayAppend(newString("Iniciar sesión"), opts);
  ptrArrayAppend(newString("Opciones generales"), opts);
  ptrArrayAppend(newString("Salir"), opts);

  int selectedOpt = 0;
  while (selectedOpt != 2) {
    selectedOpt = showMenu(opts);
    switch (selectedOpt) {
      case 0:
        if (adminLogin(conn)) {
          adminOpts();  // Mostrar opciones administrativas si el login es exitoso
        } else {
          printw("Presione cualquier tecla para continuar...\n");
          getch();
        }
        break;
      case 1:
        generalOpts();
        break;
      case 2:
        printw("Saliendo...\n");
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

  // Conectar a la base de datos
  conn = mysql_init(NULL);
  if (!conn) {
      printw("Error al inicializar la conexión a MySQL.\n");
      endwin();
      return 1;
  }

  int isRicardo = 0;
  File *hostnameFile = readFile("/etc/hostname");
  if (memcmp(hostnameFile->content, "Ideapad3", hostnameFile->len - 1) == 0) {
    isRicardo = 1;
  }
  freeFile(hostnameFile);

  if (isRicardo) {
    if (!mysql_real_connect(conn, "localhost", "root", "", "puntodeventa", 3306, NULL, 0)) {
      printw("Error al conectar a la base de datos: %s\n", mysql_error(conn));
      getch();
      endwin();
      return 1;
    }
  } else {
    if (!mysql_real_connect(conn, "localhost", "root", "Jdmfg2920**", "puntodeventa", 3306, NULL, 0)) {
      printw("Error al conectar a la base de datos: %s\n", mysql_error(conn));
      getch();
      endwin();
      return 1;
    }
  }


  initialOpts();
  // Cerrar la conexión a la base de datos
  mysql_close(conn);

  refresh();
  endwin();
  return 0;
}
