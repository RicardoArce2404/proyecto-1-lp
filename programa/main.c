#include <ncurses.h>
#include <locale.h>
#include <mysql/mysql.h>
#include "string.c"
#include "array.c"
#include "ui.c"  // Ui!

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

// Registrar familia de productos
void registerProductFamily() {
    char descripcion[100];
    printw("Ingrese la descripción de la familia: ");
    echo(); // Habilitar la visualización de la entrada
    scanw("%s", descripcion);
    noecho(); // Deshabilitar la visualización de la entrada

    char query[256];
    snprintf(query, sizeof(query), "INSERT INTO Familia (descripcion) VALUES ('%s')", descripcion);

    if (mysql_query(conn, query)) {
        printw("Error al registrar la familia: %s\n", mysql_error(conn));
    } else {
        printw("Familia registrada con éxito.\n");
    }
    refresh();
    getch();
}

// Registrar producto
void registerProduct() {
    char descripcion[100];
    int stock, id_familia;
    float costo, precio;

    printw("Ingrese la descripción del producto: ");
    echo();
    scanw("%s", descripcion);
    noecho();

    printw("Ingrese el stock: ");
    scanw("%d", &stock);

    printw("Ingrese el costo: ");
    scanw("%f", &costo);

    printw("Ingrese el precio: ");
    scanw("%f", &precio);

    printw("Ingrese el ID de la familia: ");
    scanw("%d", &id_familia);

    char query[256];
    snprintf(query, sizeof(query), 
             "INSERT INTO Producto (descripcion, stock, costo, precio, id_familia) "
             "VALUES ('%s', %d, %f, %f, %d)", 
             descripcion, stock, costo, precio, id_familia);

    if (mysql_query(conn, query)) {
        printw("Error al registrar el producto: %s\n", mysql_error(conn));
    } else {
        printw("Producto registrado con éxito.\n");
    }
    refresh();
    getch();
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

  // Conectar a la base de datos
  conn = mysql_init(NULL);
  if (!conn) {
      printw("Error al inicializar la conexión a MySQL.\n");
      endwin();
  }

  if (!mysql_real_connect(conn, "localhost", "root", "Jdmfg2920**", "puntodeventa", 3306, NULL, 0)) {
      printw("Error al conectar a la base de datos: %s\n", mysql_error(conn));
      endwin();
  }

  generalOpts(); // Llamar al menú principal una sola vez

  // Cerrar la conexión a la base de datos
  mysql_close(conn);

  refresh();
  endwin();
  return 0;
}