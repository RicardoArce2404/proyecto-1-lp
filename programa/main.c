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
#include <time.h>

MYSQL *conn;

void catalogQuery() {
  PtrArray *headings = newPtrArray();
  ptrArrayAppend(newString("ID"), headings);
  ptrArrayAppend(newString("Nombre"), headings);
  ptrArrayAppend(newString("Familia"), headings);
  ptrArrayAppend(newString("Precio"), headings);
  ptrArrayAppend(newString("Stock"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(6, widths);
  intArrayAppend(20, widths);
  intArrayAppend(20, widths);
  intArrayAppend(8, widths);
  intArrayAppend(6, widths);

  /*PtrArray *families = newPtrArray();  // Stores families.*/

  /*if (mysql_query(conn, "SELECT * FROM Familia")) {*/
  /*  printw("Error al consultar familias: %s\n", mysql_error(conn));*/
  /*  refresh();*/
  /*  getch();*/
  /*  return;*/
  /*}*/
  /**/
  /*MYSQL_RES *familyResult = mysql_store_result(conn);*/
  /*  if (!familyResult) {*/
  /*      printw("Error al obtener resultados de familias\n");*/
  /*      refresh();*/
  /*      getch();*/
  /*      return;*/
  /*  }*/
  /**/
  /*MYSQL_ROW familyRow;*/
  /*  while ((familyRow = mysql_fetch_row(familyResult))) {*/
  /*      int *idFam = familyRow[0];*/
  /*      String *familyStr = newString(familyRow[1]); // Descripción de familia*/
  /*      ptrArrayAppend(familyStr, families); //----------------------------------------------------------------Aqui quede, falta el struct de familia y agregar productos*/
  /*  }*/
  /*  mysql_free_result(familyResult);*/

  PtrArray *row1 = newPtrArray();
  ptrArrayAppend(newString("1"), row1);
  ptrArrayAppend(newString("Atún Suli"), row1);
  ptrArrayAppend(newString("Enlatados"), row1);
  ptrArrayAppend(newString("1000"), row1);
  ptrArrayAppend(newString("20"), row1);

  PtrArray *row2 = newPtrArray();
  ptrArrayAppend(newString("2"), row2);
  ptrArrayAppend(newString("Arroz Suli"), row2);
  ptrArrayAppend(newString("Granos"), row2);
  ptrArrayAppend(newString("1000"), row2);
  ptrArrayAppend(newString("20"), row2);

  PtrArray *rows = newPtrArray(); // Stores products.
  ptrArrayAppend(row1, rows);
  ptrArrayAppend(row2, rows);
  PtrArray *filteredRows = newPtrArray();
  for (int i = 0; i < rows->len; i++) {
    ptrArrayAppend(rows->data[i], filteredRows);
  }

  String *helpBar1 = newString("Puede usar las flechas para subir y bajar");
  String *helpBar2 = newString("Filtrar por familia: F  |  Regresar: <Enter>");
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  String *title = newString("Catálogo");
  int initialRow = 0;
  int keyPressed = 0;
  do {
    int numVisibleRows = showScrollableList(title, headings, filteredRows, widths, initialRow);
    move(tHeight - 2, 1);
    printCentered(helpBar1, tWidth);
    move(tHeight - 1, 1);
    printCentered(helpBar2, tWidth);

    keyPressed = getch();
    refresh();
    switch (keyPressed) {
    case KEY_UP:
      if (initialRow > 0)
        initialRow--;
      break;
    case KEY_DOWN:
      if (initialRow + numVisibleRows < rows->len)
        initialRow++;
      break;
    case 'f': {
      String *title = newString("Filtrar por familia (ID)");
      String *input = showInput(title, 2, 0);
      while (input == NULL) {
        deleteString(input);
        input = showInput(title, 2, 1);
      }
      deleteString(title);

      clearPtrArray(filteredRows);
      for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        String *family = row->data[2];
        if (family->len == input->len && memcmp(family->text, input->text, input->len) == 0) {
          ptrArrayAppend(row, filteredRows);
        }
      }
      deleteString(input);
      break;
    }
    }
  } while (keyPressed != '\n');
  deleteString(title);
  deleteString(helpBar1);
  deleteString(helpBar2);

  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
  }
  deletePtrArray(rows);
  deletePtrArray(filteredRows);
}

int showCatalog() {
  PtrArray *headings = newPtrArray();
  ptrArrayAppend(newString("ID"), headings);
  ptrArrayAppend(newString("Nombre"), headings);
  ptrArrayAppend(newString("Familia"), headings);
  ptrArrayAppend(newString("Precio"), headings);
  ptrArrayAppend(newString("Stock"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(6, widths);
  intArrayAppend(20, widths);
  intArrayAppend(20, widths);
  intArrayAppend(8, widths);
  intArrayAppend(5, widths);

  PtrArray *row1 = newPtrArray();
  ptrArrayAppend(newString("1"), row1);
  ptrArrayAppend(newString("Atún Suli"), row1);
  ptrArrayAppend(newString("Enlatados"), row1);
  ptrArrayAppend(newString("1000"), row1);
  ptrArrayAppend(newString("20"), row1);

  PtrArray *row2 = newPtrArray();
  ptrArrayAppend(newString("2"), row2);
  ptrArrayAppend(newString("Arroz Suli"), row2);
  ptrArrayAppend(newString("Granos"), row2);
  ptrArrayAppend(newString("1000"), row2);
  ptrArrayAppend(newString("20"), row2);

  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  ptrArrayAppend(row1, rows);
  ptrArrayAppend(row2, rows);
  PtrArray *filteredRows = newPtrArray();
  for (int i = 0; i < rows->len; i++) {
    ptrArrayAppend(rows->data[i], filteredRows);
  }

  String *helpBar1 = newString("Puede usar las flechas para subir y bajar");
  String *helpBar2 = newString("Filtrar por familia: F  |  Elegir por ID: <Enter>");
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  String *title = newString("Catálogo");
  int initialRow = 0;
  int keyPressed = 0;
  do {
    int numVisibleRows = showScrollableList(title, headings, filteredRows, widths, initialRow);
    move(tHeight - 2, 1);
    printCentered(helpBar1, tWidth);
    move(tHeight - 1, 1);
    printCentered(helpBar2, tWidth);

    keyPressed = getch();
    refresh();
    switch (keyPressed) {
    case KEY_UP:
      if (initialRow > 0)
        initialRow--;
      break;
    case KEY_DOWN:
      if (initialRow + numVisibleRows < rows->len)
        initialRow++;
      break;
    case 'f': {
      String *title = newString("Filtrar por familia (Nombre)");
      String *input = showInput(title, 2, 0);
      while (input == NULL) {
        deleteString(input);
        input = showInput(title, 2, 1);
      }
      deleteString(title);

      clearPtrArray(filteredRows);
      for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        String *family = row->data[2];
        if (family->len == input->len && memcmp(family->text, input->text, input->len) == 0) {
          ptrArrayAppend(row, filteredRows);
        }
      }
      deleteString(input);
      break;
    }
    }
  } while (keyPressed != '\n');
  deleteString(title);
  deleteString(helpBar1);
  deleteString(helpBar2);

  title = newString("Ingrese el ID a seleccionar");
  String *input = showInput(title, 2, 0);
  while (input == NULL && !isNumber(input)) {
    deleteString(input);
    input = showInput(title, 2, 1);
  }
  deleteString(title);
  int id = toInt(input);
  deleteString(input);
  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
  }
  deletePtrArray(rows);
  deletePtrArray(filteredRows);
  return id;
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

  String *helpBar1 = newString("Puede usar las flechas para subir y bajar");
  String *helpBar2 = newString("Agregar producto: +  |  Eliminar producto: -  |  Guardar: <Enter>");
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);

  String *title = newString("Crear cotización");
  int initialRow = 0;
  int keyPressed = 0;
  do {
    int numVisibleRows = showScrollableList(title, headings, rows, widths, initialRow);
    move(tHeight - 2, 1);
    printCentered(helpBar1, tWidth);
    move(tHeight - 1, 1);
    printCentered(helpBar2, tWidth);

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
      int id = showCatalog();
      id++;
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
  deleteString(helpBar1);
  deleteString(helpBar2);

  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
  }
  deletePtrArray(rows);
}

void editQuotation() {
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  clear();
  String *title = newString("Ingrese el número de cotización");
  String *input = showInput(title, tHeight / 2, 0);
  while (input == NULL || !isNumber(input)) {
    deleteString(input);
    input = showInput(title, tHeight / 2, 1);
  }
  deleteString(title);
  /*int quotationId = toInt(input);*/

  if (0) { // If quotation doesn't exist.
    String *title = newString("Cotización no encontrada.");
    String *msg = newString("Error: No existe ninguna cotización con el número introducido.");
    showAlert(title, msg, tHeight / 2, 1);
    deleteString(title);
    deleteString(msg);
    return;
  } else if (0) { // If quotation exists but is already invoiced.
    String *title = newString("Cotización ya facturada.");
    String *msg = newString("Error: La cotización asociada al número introducido ya fue facturada.");
    showAlert(title, msg, tHeight / 2, 1);
    deleteString(title);
    deleteString(msg);
    return;
  }

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

  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  String *helpBar1 = newString("Puede usar las flechas para subir y bajar");
  String *helpBar2 = newString("Agregar producto: +  |  Eliminar producto: -  |  Guardar: <Enter>");

  title = newString("Crear cotización");
  int initialRow = 0;
  int keyPressed = 0;
  do {
    int numVisibleRows = showScrollableList(title, headings, rows, widths, initialRow);
    move(tHeight - 2, 1);
    printCentered(helpBar1, tWidth);
    move(tHeight - 1, 1);
    printCentered(helpBar2, tWidth);

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
      int id = showCatalog();
      for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        if (toInt(row->data[0]) == id) {
          // Logic to add new amount to existing amount.
          break;
        }
      }
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
  deleteString(helpBar1);
  deleteString(helpBar2);

  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
  }
  deletePtrArray(rows);
}

void makeInvoice() {
  //int tWidth = 0;
  int tHeight = 0;
  //getmaxyx(stdscr, tHeight, tWidth);
  clear();
  String *title = newString("Ingrese el número de cotización");
  String *input = showInput(title, tHeight / 2, 0);
  while (input == NULL || !isNumber(input)) {
    deleteString(input);
    input = showInput(title, tHeight / 2, 1);
  }
  deleteString(title);
  /*int quotationId = toInt(input);*/
  deleteString(input);
  if (0) { // If quotation doesn't exist.
    String *title = newString("Cotización no encontrada.");
    String *msg = newString("Error: No existe ninguna cotización con el número introducido.");
    showAlert(title, msg, tHeight / 2, 1);
    deleteString(title);
    deleteString(msg);
    return;
  } else if (0) { // If quotation exists but is already invoiced.
    String *title = newString("Cotización ya facturada.");
    String *msg = newString("Error: La cotización asociada al número introducido ya fue facturada.");
    showAlert(title, msg, tHeight / 2, 1);
    deleteString(title);
    deleteString(msg);
    return;
  }

  title = newString("Ingrese el nombre del cliente");
  String *client = showInput(title, tHeight / 2, 0);
  while (client == NULL) {
    deleteString(client);
    client = showInput(title, tHeight / 2, 1);
  }
  deleteString(title);

  String *invoiceId = newString("1");
  String *localName = newString("PulpeTEC");
  String *legalId = newString("3-101-123456");
  String *phoneNum = newString("1234-5678");
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);
  char buffer[1024] = {0};
  snprintf(buffer, 1024, "%i-%i-%i", lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
  String *date = newString(buffer);

  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  String *helpBar1 = newString("Puede usar las flechas para subir y bajar");
  String *helpBar2 = newString("Cancelar: C  |  Aceptar: A");

  PtrArray *headings = newPtrArray();
  ptrArrayAppend(newString("#"), headings);
  ptrArrayAppend(newString("Nombre"), headings);
  ptrArrayAppend(newString("Cantidad"), headings);
  ptrArrayAppend(newString("Precio"), headings);
  ptrArrayAppend(newString("Total"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(3, widths);
  intArrayAppend(20, widths);
  intArrayAppend(10, widths);
  intArrayAppend(10, widths);
  intArrayAppend(10, widths);

  // The invoice header occupies 13 rows, plus 2 for top and bottom borders,
  // plus 4 for total price summary.
  int height = rows->len + 19;
  int width = 0;
  for (int i = 0; i < widths->len; i++) {
    width += widths->data[i];
  }
  // This is to consider the width of vertical column separators and left
  // and right borders.
  width += headings->len + 1;
  int ulCornerRow = (tHeight - height) / 2;
  int ulCornerCol = (tWidth - width) / 2;
  Cell ulCorner = {ulCornerRow, ulCornerCol};

  title = newString("Facturar");
  String *subtotalStr = newString("Subtotal │");
  String *taxesStr = newString("Impuesto de venta 13% │");
  String *totalStr = newString("Total │");
  /*int initialRow = 0;*/
  int keyPressed = 0;
  do {
    clear();
    printRectangle(ulCorner, width, height);
    mvprintw(ulCornerRow + 1, ulCornerCol + 1,
             "Consecutivo: %.*s", invoiceId->len, invoiceId->text);
    printLineD(ulCornerRow + 2, ulCornerCol + 1, width - 1, 1);
    mvprintw(ulCornerRow + 3, ulCornerCol + 1,
             "Nombre del local: %.*s", localName->len, localName->text);
    printLineD(ulCornerRow + 4, ulCornerCol + 1, width - 1, 1);
    mvprintw(ulCornerRow + 5, ulCornerCol + 1,
             "Cédula jurídica: %.*s", legalId->len, legalId->text);
    printLineD(ulCornerRow + 6, ulCornerCol + 1, width - 1, 1);
    mvprintw(ulCornerRow + 7, ulCornerCol + 1,
             "Teléfono: %.*s", phoneNum->len, phoneNum->text);
    printLineD(ulCornerRow + 8, ulCornerCol + 1, width - 1, 1);
    mvprintw(ulCornerRow + 9, ulCornerCol + 1,
             "Fecha: %.*s", date->len, date->text);
    printLineD(ulCornerRow + 10, ulCornerCol + 1, width - 1, 1);
    mvprintw(ulCornerRow + 11, ulCornerCol + 1,
             "Cliente: %.*s", client->len, client->text);
    printLineD(ulCornerRow + 12, ulCornerCol + 1, width - 1, 1);
    move(ulCornerRow + 13, ulCornerCol + 1);
    printRow(headings, widths);
    printLineD(ulCornerRow + 14, ulCornerCol + 1, width - 1, 1);

    int detailsRow = ulCornerRow + 15;
    for (int i = 0; i < rows->len; i++) {
      PtrArray *row = rows->data[i];
      move(detailsRow + i, ulCornerCol + 1);
      printRow(row, widths);
    }
    int summaryRow = detailsRow + rows->len;
    printLineD(summaryRow, ulCornerCol + 1, width - 1, 1);

    int subtotal = 0;
    int taxes = 0;
    int total = 0;
    // The + 1 is to take into account the column separator.
    int c = ulCornerCol + width - widths->data[4] + 1;
    int subtotalCol = c - subtotalStr->len;
    int taxesCol = c - taxesStr->len;
    int totalCol = c - totalStr->len;
    mvprintw(summaryRow + 1, subtotalCol, "%.*s %i",
             subtotalStr->len, subtotalStr->text, subtotal);
    mvprintw(summaryRow + 2, taxesCol, "%.*s %i",
             taxesStr->len, taxesStr->text, taxes);
    mvprintw(summaryRow + 3, totalCol, "%.*s %i",
             totalStr->len, totalStr->text, total);
    keyPressed = getch();
  } while (keyPressed != '\n');

  deleteString(title);
  deleteString(helpBar1);
  deleteString(helpBar2);
  deleteString(subtotalStr);
  deleteString(taxesStr);
  deleteString(totalStr);

  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
  }
  deletePtrArray(rows);
  deleteString(invoiceId);
  deleteString(localName);
  deleteString(legalId);
  deleteString(phoneNum);
  deleteString(date);
  deleteString(client);
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
        deleteProduct(conn);
        break;
      case 3:
        LoadInventory(conn);
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
