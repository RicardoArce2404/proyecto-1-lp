#include <stdio.h>
#define _XOPEN_SOURCE 700
#include "array.c"
#include "filehandling.c"
#include "inventory.c"
#include "login.c"
#include "string.c"
#include "ui.c" // Ui! Ui!
#include <locale.h>
#include <mysql/mysql.h>
#include <ncurses.h>
#include <string.h>

MYSQL *conn;

// TO DO.
void catalogQuery() {
}

void makeQuotation() {
  PtrArray *headings = newPtrArray();
  ptrArrayAppend(newString("#"), headings);
  ptrArrayAppend(newString("Nombre"), headings);
  ptrArrayAppend(newString("Descripción"), headings);
  ptrArrayAppend(newString("Precio"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(3, widths);
  intArrayAppend(20, widths);
  intArrayAppend(40, widths);
  intArrayAppend(20, widths);

  PtrArray *row1 = newPtrArray();
  ptrArrayAppend(newString("1"), row1);
  ptrArrayAppend(newString("1234"), row1);
  ptrArrayAppend(newString("abcde"), row1);
  ptrArrayAppend(newString("abcde"), row1);

  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  ptrArrayAppend(row1, rows);

  String *title = newString("titulo");
  int initialRow = 0;
  int keyPressed = 0;
  do {
    int numVisibleRows = showScrollableList(title, headings, rows, widths, initialRow);
    keyPressed = getch();
    switch (keyPressed) {
    case KEY_UP:
      if (initialRow > 0)
        initialRow--;
      break;
    case KEY_DOWN:
      if (initialRow + numVisibleRows < rows->len)
        initialRow++;
      break;
    case '+': {
      String *title = newString("Agregar producto (ID)");
      String *input = showInput(title, 2, 0);
      while (input == NULL || !isNumber(input)) {
        deleteString(input);
        input = showInput(title, 2, 1);
      }
      deleteString(title);
      /*int id = toInt(input);*/
      deleteString(input);
      // TO DO: Query DB to get product info using product's ID.
      PtrArray *newRow = newPtrArray();
      ptrArrayAppend(newString("1"), newRow);
      ptrArrayAppend(newString("1234"), newRow);
      ptrArrayAppend(newString("abcde"), newRow);
      ptrArrayAppend(newString("abcde"), newRow);
      ptrArrayAppend(newRow, rows);
      break;
    }
    case '-': {
      if (rows->len == 0) {
        break;
      }
      String *title = newString("Eliminar producto (Fila)");
      String *input = showInput(title, 2, 0);
      while (input == NULL || !isNumber(input)) {
        deleteString(input);
        input = showInput(title, 2, 1);
      }
      deleteString(title);
      int row = toInt(input) - 1;
      deleteString(input);
      if (0 <= row && row <= rows->len - 1) {
        deleteStringArray(rows->data[row]);
        ptrArrayRemove(row, rows);
      }
      break;
    }
    }
  } while (keyPressed != '\n');
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
  ptrArrayAppend(newString("Eliminar producto"), opts);
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
        adminOpts();
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

  for (int i = 0; i < opts->len; i++)
    deleteString(opts->data[i]);
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

  for (int i = 0; i < opts->len; i++)
    deleteString(opts->data[i]);
  deletePtrArray(opts);
}

void initialOpts() {
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
          adminOpts(); // Mostrar opciones administrativas si el login es exitoso
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

  for (int i = 0; i < opts->len; i++)
    deleteString(opts->data[i]);
  deletePtrArray(opts);
}

int main() {
  setlocale(LC_CTYPE, "");
  initscr();
  use_default_colors();
  start_color();
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
    if (!mysql_real_connect(conn, "localhost", "root", "", "puntodeventa", 3306,
                            NULL, 0)) {
      printw("Error al conectar a la base de datos: %s\n", mysql_error(conn));
      getch();
      endwin();
      return 1;
    }
  } else {
    if (!mysql_real_connect(conn, "localhost", "root", "Jdmfg2920**",
                            "puntodeventa", 3306, NULL, 0)) {
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
