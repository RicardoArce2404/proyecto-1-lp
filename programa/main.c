#include <ncurses.h>
#include <locale.h>
#include <mysql/mysql.h>
#include "string.c"
#include "array.c"
#include "ui.c"  // Ui! Ui!
#include "login.c" 
#include "inventory.c"

MYSQL *conn;

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

  for (int i = 0; i < opts->len; i++) deleteString(opts->data[i]);
  deletePtrArray(opts);
}

void generalOpts() {
  PtrArray *opts = newPtrArray();
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

void initialOpts(){
  PtrArray *opts = newPtrArray();
  ptrArrayAppend(newString("Iniciar sesión"), opts);
  ptrArrayAppend(newString("Opciones generales"), opts);
  ptrArrayAppend(newString("Salir"), opts);

  int selectedOpt = 0;
  while (selectedOpt != 5) {
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

  if (!mysql_real_connect(conn, "localhost", "root", "Jdmfg2920**", "puntodeventa", 3306, NULL, 0)) {
      printw("Error al conectar a la base de datos: %s\n", mysql_error(conn));
      endwin();
      return 1;
  }

  initialOpts();
  // Cerrar la conexión a la base de datos
  mysql_close(conn);

  refresh();
  endwin();
  return 0;
}