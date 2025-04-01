#include <mysql/field_types.h>
#include <stdio.h>
#include <stdlib.h>
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
#include <time.h>
#include "invoice.c"
#include "statistics.c"
#include "quotation.c"

MYSQL *conn;

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
  while (selectedOpt != 6) {
    selectedOpt = showMenu(opts);
    switch (selectedOpt) {
      case 0:
        registerProductFamily(conn);
        break;
      case 1:
        registerProduct(conn);
        break;
      case 2:
        deleteProduct(conn);
        break;
      case 3:
        LoadInventory(conn);
        break;
      case 4:
        queryInvoice(conn);
        break;
      case 5:
        statistics(conn);
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
        catalogQuery(conn);
        break;
      case 1:
        makeQuotation(conn);
        break;
      case 2:
        editQuotation(conn);
        break;
      case 3:
        makeInvoice(conn);
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

  // Mostrar pantalla de bienvenida
  showWelcomeScreen("BIENVENIDOS A PULTETEC", "Sistema de Punto de Venta");

  // Conectar a la base de datos
  conn = mysql_init(NULL);
  if (!conn) {
    printw("Error al inicializar la conexión a MySQL.\n");
    endwin();
    return 1;
  }

  int isRicardo = 0;
  File *hostnameFile = readFile("/etc/hostname");
  if (hostnameFile->len==9 && memcmp(hostnameFile->content, "Ideapad3", hostnameFile->len - 1) == 0) {
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
