#ifndef QUOTATION_C
#define QUOTATION_C

#include <mysql/field_types.h>
#include <stdio.h>
#include <stdlib.h>
#define _XOPEN_SOURCE 700
#include "array.c"
#include "inventory.c"
#include "string.c"
#include "ui.c" // Ui! Ui!
#include <locale.h>
#include <mysql/mysql.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>

void catalogQuery(MYSQL *conn) {
  PtrArray *headings = newPtrArray();
  ptrArrayAppend(newString("ID"), headings);
  ptrArrayAppend(newString("Nombre"), headings);
  ptrArrayAppend(newString("Familia"), headings);
  ptrArrayAppend(newString("Precio"), headings);
  ptrArrayAppend(newString("Stock"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(10, widths);
  intArrayAppend(20, widths);
  intArrayAppend(20, widths);
  intArrayAppend(15, widths);
  intArrayAppend(6, widths);

  if (mysql_query(conn, "SELECT * FROM Familia")) {
    printw("Error al consultar familias: %s\n", mysql_error(conn));
    refresh();
    getch();
    return;
  }

  MYSQL_RES *familyResult = mysql_store_result(conn);
  if (!familyResult) {
    printw("Error al obtener resultados de familias\n");
    refresh();
    getch();
    return;
  }

  MYSQL_ROW row;
  PtrArray *families = newPtrArray();  // Stores families.
  while ((row = mysql_fetch_row(familyResult))) {
    String *id = newString(row[0]);
    String *desc = newString(row[1]);
    Family *family = malloc(sizeof(Family));
    if (!id || !desc || !family) {
      printw("Error al procesar los datos de familias\n");
      refresh();
      getch();
      return;
    }
    family->id = id;
    family->description = desc;
    ptrArrayAppend(family, families);
  }
  mysql_free_result(familyResult);

  if (mysql_query(conn, "SELECT * FROM Producto")) {
    printw("Error al consultar productos: %s\n", mysql_error(conn));
    refresh();
    getch();
    return;
  }

  MYSQL_RES *productResult = mysql_store_result(conn);
  if (!productResult) {
    printw("Error al obtener resultados de productos\n");
    refresh();
    getch();
    return;
  }

  PtrArray *products = newPtrArray();  // Stores products.
  while ((row = mysql_fetch_row(productResult))) {
    String *id = newString(row[0]);
    String *desc = newString(row[1]);
    String *fDesc = NULL;
    for (int i = 0; i < families->len; i++) {
      Family *f = families->data[i];
      String *fId = f->id;
      if (compareStringToBuffer(fId, row[5])) {
        fDesc = newStringN(f->description->text, f->description->len);
      }
    }
    Product *product = malloc(sizeof(Product));
    if (!id || !desc || !product) {
      printw("Error al procesar los datos de productos\n");
      refresh();
      getch();
      return;
    }
    product->id = id;
    product->description = desc;
    product->family_desc = fDesc;
    sscanf(row[3], "%f", &(product->cost));
    sscanf(row[4], "%f", &(product->price));
    sscanf(row[2], "%i", &(product->stock));
    ptrArrayAppend(product, products);

  }
  mysql_free_result(productResult);
 
  PtrArray *rows = newPtrArray(); // Stores products.
  for (int i = 0; i < products->len; i++) {
    Product *p = products->data[i];
    PtrArray *row = newPtrArray();
    ptrArrayAppend(newStringN(p->id->text, p->id->len), row);
    ptrArrayAppend(newStringN(p->description->text, p->description->len), row);
    ptrArrayAppend(newStringN(p->family_desc->text, p->family_desc->len), row);
    ptrArrayAppend(newStringD(p->cost), row);
    ptrArrayAppend(newStringI(p->stock), row);
    ptrArrayAppend(row, rows);
  }
  /*PtrArray *row1 = newPtrArray();*/
  /*ptrArrayAppend(newString("1"), row1);*/
  /*ptrArrayAppend(newString("Atún Suli"), row1);*/
  /*ptrArrayAppend(newString("Enlatados"), row1);*/
  /*ptrArrayAppend(newString("1000"), row1);*/
  /*ptrArrayAppend(newString("20"), row1);*/
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
  for (int i = 0; i < products->len; i++) {
    freeProduct(products->data[i]);
  }
  deletePtrArray(products);
  for (int i = 0; i < families->len; i++) {
    freeFamily(families->data[i]);
  }
  deletePtrArray(families);
  deletePtrArray(filteredRows);
}

String *showCatalog(MYSQL *conn) {
  PtrArray *headings = newPtrArray();
  ptrArrayAppend(newString("ID"), headings);
  ptrArrayAppend(newString("Nombre"), headings);
  ptrArrayAppend(newString("Familia"), headings);
  ptrArrayAppend(newString("Precio"), headings);
  ptrArrayAppend(newString("Stock"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(10, widths);
  intArrayAppend(20, widths);
  intArrayAppend(20, widths);
  intArrayAppend(15, widths);
  intArrayAppend(6, widths);

  if (mysql_query(conn, "SELECT * FROM Familia")) {
    printw("Error al consultar familias: %s\n", mysql_error(conn));
    refresh();
    getch();
    return NULL;
  }

  MYSQL_RES *familyResult = mysql_store_result(conn);
  if (!familyResult) {
    printw("Error al obtener resultados de familias\n");
    refresh();
    getch();
    return NULL;
  }

  MYSQL_ROW row;
  PtrArray *families = newPtrArray();  // Stores families.
  while ((row = mysql_fetch_row(familyResult))) {
    String *id = newString(row[0]);
    String *desc = newString(row[1]);
    Family *family = malloc(sizeof(Family));
    if (!id || !desc || !family) {
      printw("Error al procesar los datos de familias\n");
      refresh();
      getch();
      return NULL;
    }
    family->id = id;
    family->description = desc;
    ptrArrayAppend(family, families);
  }
  mysql_free_result(familyResult);

  if (mysql_query(conn, "SELECT * FROM Producto")) {
    printw("Error al consultar productos: %s\n", mysql_error(conn));
    refresh();
    getch();
    return NULL;
  }

  MYSQL_RES *productResult = mysql_store_result(conn);
  if (!productResult) {
    printw("Error al obtener resultados de productos\n");
    refresh();
    getch();
    return NULL;
  }

  PtrArray *products = newPtrArray();  // Stores products.
  while ((row = mysql_fetch_row(productResult))) {
    String *id = newString(row[0]);
    String *desc = newString(row[1]);
    String *fDesc = NULL;
    for (int i = 0; i < families->len; i++) {
      Family *f = families->data[i];
      String *fId = f->id;
      if (compareStringToBuffer(fId, row[5])) {
        fDesc = newStringN(f->description->text, f->description->len);
      }
    }
    Product *product = malloc(sizeof(Product));
    if (!id || !desc || !product) {
      printw("Error al procesar los datos de productos\n");
      refresh();
      getch();
      return NULL;
    }
    product->id = id;
    product->description = desc;
    product->family_desc = fDesc;
    sscanf(row[3], "%f", &(product->cost));
    sscanf(row[4], "%f", &(product->price));
    sscanf(row[2], "%i", &(product->stock));
    ptrArrayAppend(product, products);

  }
  mysql_free_result(productResult);
 
  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  for (int i = 0; i < products->len; i++) {
    Product *p = products->data[i];
    PtrArray *row = newPtrArray();
    ptrArrayAppend(newStringN(p->id->text, p->id->len), row);
    ptrArrayAppend(newStringN(p->description->text, p->description->len), row);
    ptrArrayAppend(newStringN(p->family_desc->text, p->family_desc->len), row);
    ptrArrayAppend(newStringD(p->cost), row);
    ptrArrayAppend(newStringI(p->stock), row);
    ptrArrayAppend(row, rows);
  }
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
  while (input == NULL) {
    deleteString(input);
    input = showInput(title, 2, 1);
  }
  deleteString(title);
  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
  }
  deletePtrArray(rows);
  for (int i = 0; i < products->len; i++) {
    freeProduct(products->data[i]);
  }
  deletePtrArray(products);
  for (int i = 0; i < families->len; i++) {
    freeFamily(families->data[i]);
  }
  deletePtrArray(families);
  deletePtrArray(filteredRows);
  return input;
}

void makeQuotation(MYSQL *conn) {
  PtrArray *headings = newPtrArray();
  ptrArrayAppend(newString("#"), headings);
  ptrArrayAppend(newString("Nombre"), headings);
  ptrArrayAppend(newString("Descripción"), headings);
  ptrArrayAppend(newString("Cantidad"), headings);
  ptrArrayAppend(newString("Precio"), headings);
  ptrArrayAppend(newString("Total"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(3, widths);
  intArrayAppend(20, widths);
  intArrayAppend(40, widths);
  intArrayAppend(10, widths);
  intArrayAppend(10, widths);
  intArrayAppend(15, widths);

  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  PtrArray *ids = newPtrArray(); // This stores only product ids.

  String *helpBar1 = newString("Puede usar las flechas para subir y bajar");
  String *helpBar2 = newString("Agregar producto: +  |  Eliminar producto: -  |  Guardar: <Enter>");
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  int height = 7;
  /*const int MAX_ROWS = tHeight - 15;*/
  const int MAX_ROWS = 5;
  if (rows->len > MAX_ROWS) {
    height += MAX_ROWS;
  } else {
    height += rows->len;
  }
  int width = 0;
  for (int i = 0; i < widths->len; i++) {
    width += widths->data[i];
  }
  width += headings->len + 1;

  int ulCornerRow = (tHeight - height) / 2;
  int ulCornerCol = (tWidth - width) / 2;
  Cell ulCorner = {ulCornerRow, ulCornerCol};

  String *subtotalStr = newString("Subtotal │");
  String *taxesStr = newString("Impuesto de venta 13% │");
  String *totalStr = newString("Total │");
  String *title = newString("Crear cotización");
  int initialRow = 0;
  int keyPressed = 0;
  int numVisibleRows = height - 7;
  do {
    height = 7;
    if (rows->len > MAX_ROWS) {
      height += MAX_ROWS;
    } else {
      height += rows->len;
    }
    numVisibleRows = height - 7;
    ulCornerRow = (tHeight - height) / 2;
    ulCornerCol = (tWidth - width) / 2;
    ulCorner = (Cell){ulCornerRow, ulCornerCol};

    clear();
    printRectangle(ulCorner, width, height);
    move(ulCornerRow + 1, ulCornerCol + 1);
    printRow(headings, widths);
    printLineD(ulCornerRow + 2, ulCornerCol + 1, width - 1, 1);

    int detailsRow = ulCornerRow + 3;
    for (int i = 0; i < numVisibleRows; i++) {
      PtrArray *row = rows->data[initialRow + i];
      String *num = newStringI(i + 1);
      deleteString(row->data[0]);
      row->data[0] = num;
      move(detailsRow + i, ulCornerCol + 1);
      printRow(row, widths);
    }
    int summaryRow = detailsRow + rows->len;
    printLineD(summaryRow, ulCornerCol + 1, width - 1, 1);

    double subtotal = 0;
    for (int i = 0; i < rows->len; i++) {
      PtrArray *row = rows->data[i];
      String *totalStr = row->data[5];
      subtotal += toDouble(totalStr);
    }
    double taxes = subtotal * 0.13;
    double total = subtotal + taxes;
    // The + 1 is to take into account the column separator.
    int c = ulCornerCol + width - widths->data[5] + 1;
    int subtotalCol = c - subtotalStr->len;
    int taxesCol = c - taxesStr->len;
    int totalCol = c - totalStr->len;
    mvprintw(summaryRow + 1, subtotalCol, "%.*s %.1f",
             subtotalStr->len, subtotalStr->text, subtotal);
    mvprintw(summaryRow + 2, taxesCol, "%.*s %.1f",
             taxesStr->len, taxesStr->text, taxes);
    mvprintw(summaryRow + 3, totalCol, "%.*s %.1f",
             totalStr->len, totalStr->text, total);
    move(tHeight - 3, ulCornerCol);
    printCentered(helpBar1, width);
    move(tHeight - 2, ulCornerCol);
    printCentered(helpBar2, width);
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
      String *id = showCatalog(conn);
      char query[256] = {0};
      sprintf(query,
              "SELECT * FROM Producto AS p JOIN Familia as f WHERE p.id_producto = \"%.*s\" && p.id_familia = f.id_familia",
              id->len,
              id->text);
      if (mysql_query(conn, query)) {
        printw("Error al consultar producto: %s\n", mysql_error(conn));
        refresh();
        getch();
        return;
      }
      MYSQL_RES *productResult = mysql_store_result(conn);
      if (!productResult) {
        printw("Error al obtener resultados de producto\n");
        refresh();
        getch();
        return;
      }
      int numRows = mysql_num_rows(productResult);
      if (numRows == 0) {
        mysql_free_result(productResult);
        deleteString(id);
        break;
      }
      clearBlock((Cell){0, 0}, width - 1, 5);
      String *title = newString("Ingrese la cantidad");
      String *amountStr = showInput(title, 2, 0);
      while (!amountStr || !isNumber(amountStr) || toInt(amountStr) < 1) {
        deleteString(amountStr);
        amountStr = showInput(title, 2, 1);
      }
      deleteString(title);
      int amount = toInt(amountStr);
      MYSQL_ROW dbRow = mysql_fetch_row(productResult);
      int stock = atoi(dbRow[2]);

      int isUsed = 0;
      int currAmount = 0;
      for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        if (compareStringToBuffer(row->data[1], dbRow[1])) {
          currAmount = toInt(row->data[3]);
          if (currAmount + amount > stock) {
            break;
          }
          double price = toDouble(row->data[4]);
          double total = (currAmount + amount) * price;
          deleteString(row->data[3]);
          row->data[3] = newStringI(currAmount + amount);
          deleteString(row->data[5]);
          row->data[5] = newStringD(total);
          isUsed = 1;
          break;
        }
      }
      if (currAmount + amount > stock) {
        String *msg = newString("Stock insuficiente");
        showAlert(NULL, msg, 3, 1);
        deleteString(msg);
        mysql_free_result(productResult);
        deleteString(amountStr);
        deleteString(id);
        break;
      }

      if (isUsed) {
        mysql_free_result(productResult);
        deleteString(amountStr);
        deleteString(id);
        break;
      }

      PtrArray *newRow = newPtrArray();
      ptrArrayAppend(newStringI(rows->len + 1), newRow); // Index.
      ptrArrayAppend(newString(dbRow[1]), newRow); // Name.
      ptrArrayAppend(newString(dbRow[7]), newRow); // Family.
      ptrArrayAppend(amountStr, newRow); // Amount.
      ptrArrayAppend(newString(dbRow[4]), newRow); // Price.
      ptrArrayAppend(newStringD(amount * atof(dbRow[4])), newRow); // Total.
      ptrArrayAppend(newRow, rows);
      mysql_free_result(productResult);
      ptrArrayAppend(id, ids);
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
        deleteString(ids->data[row]);
        ptrArrayRemove(row, ids);
      }

      break;
    }
    }
  } while (keyPressed != '\n');

  deleteString(title);
  deleteString(helpBar1);
  deleteString(helpBar2);
  deleteString(subtotalStr);
  deleteString(taxesStr);
  deleteString(totalStr);

  if (rows->len == 0) {
    String *s = newString("No se efectuaron cambios");
    showAlert(NULL, s, 3, 0);
    deleteString(s);
    deleteStringArray(headings);
    deleteIntArray(widths);
    for (int i = 0; i < rows->len; i++) {
      deleteStringArray(rows->data[i]);
      deleteString(ids->data[i]);
    }
    deletePtrArray(rows);
    deletePtrArray(ids);
    return;
  }

  MYSQL_STMT *stmt; // Esta wea parece hecha por chatgpt pero así sale en la documentación oficial xd.

  stmt = mysql_stmt_init(conn);
  if (stmt == NULL) {
    printw("Error inicializando stmt.");
    getch();
    exit(1);
  }

  char query[] = "CALL RegistrarCotizacion(@quotationId, @result)";
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    printw("%s", mysql_stmt_error(stmt));
    getch();
    exit(1);
  }

  if (mysql_stmt_execute(stmt)) {
    printw("%s", mysql_stmt_error(stmt));
    getch();
    exit(1);
  }

  if (mysql_query(conn, "SELECT @quotationId, @result")) {
    printw("%s", mysql_error(conn));
    getch();
    exit(1);
  }

  MYSQL_RES *res = mysql_store_result(conn);
  if (res == NULL) {
    printw("%s", mysql_error(conn));
    getch();
    exit(1);
  }

  MYSQL_ROW row  = mysql_fetch_row(res);
  int quotationId = atoi(row[0]);
  //int result = atoi(row[1]);
  mysql_free_result(res);
  mysql_stmt_close(stmt);

  MYSQL_STMT *stmtDet;
  MYSQL_BIND bindDet[4];
  //int resultDet = 0;

  stmtDet = mysql_stmt_init(conn);
  if (stmtDet == NULL) {
    printw("Error inicializando stmt.");
    getch();
    exit(1);
  }

  for (int i = 0; i < rows->len; i++) {
    char query[] = "CALL AgregarDetalleCotizacion(?, ?, ?, @resultDet)";
    if (mysql_stmt_prepare(stmtDet, query, strlen(query))) {
      printw("%s", mysql_stmt_error(stmtDet));
      getch();
      exit(1);
    }

    memset(bindDet, 0, sizeof(bindDet));

    bindDet[0].buffer_type = MYSQL_TYPE_LONG;
    bindDet[0].buffer = (char*)&quotationId;
    bindDet[0].is_null = 0;
    bindDet[0].length = 0;

    String *id = ids->data[i];
    bindDet[1].buffer_type = MYSQL_TYPE_STRING;
    bindDet[1].buffer = id->text;
    bindDet[1].buffer_length = id->len;
    bindDet[1].is_null = 0;
    bindDet[1].length = 0;

    PtrArray *row = rows->data[i];
    String *amountStr = row->data[3];
    int amount = toInt(amountStr);
    bindDet[2].buffer_type = MYSQL_TYPE_LONG;
    bindDet[2].buffer = (char *)&amount;
    bindDet[2].is_null = 0;
    bindDet[2].length = 0;

    if (mysql_stmt_bind_param(stmtDet, bindDet)) {
      printw("%s", mysql_stmt_error(stmtDet));
      getch();
      exit(1);
    }

    if (mysql_stmt_execute(stmtDet)) {
      printw("%s", mysql_stmt_error(stmtDet));
      getch();
      exit(1);
    }

    if (mysql_query(conn, "SELECT @resultDet")) {
      printw("%s", mysql_error(conn));
      getch();
      exit(1);
    }

    MYSQL_RES *resDet = mysql_store_result(conn);
    if (resDet == NULL) {
      printw("%s", mysql_error(conn));
      getch();
      exit(1);
    }

    //MYSQL_ROW rowDet  = mysql_fetch_row(resDet);
    //int result = atoi(rowDet[0]);
    mysql_free_result(resDet);
  }
  mysql_stmt_close(stmtDet);

  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
    deleteString(ids->data[i]);
  }
  deletePtrArray(rows);
  deletePtrArray(ids);

  char buf[70] = {0};
  sprintf(buf, "Esta cotización se guardó con el código %i.", quotationId);
  buf[69] = '\0';
  String *s = newString(buf);
  showAlert(NULL, s, 3, 0);
  deleteString(s);
}

void editQuotation(MYSQL *conn) {
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
  int quotationId = toInt(input);
  deleteString(input);

  if (mysql_query(conn, "SELECT * FROM Cotizacion")) {
    printw("Error al consultar cotizaciones: %s\n", mysql_error(conn));
    getch();
    exit(1);
  }

  MYSQL_RES *result = mysql_store_result(conn);
  if (!result) {
    printw("Error al obtener resultados de cotizaciones\n");
    getch();
    exit(1);
  }

  int flag = 0; // 0: unknown id. 1: invoiced id. 2: OK id.
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) {
    if (quotationId == atoi(row[0])) {
      if (strcmp(row[1], "Pendiente") == 0) {
        flag = 2;
      } else {
        flag = 1;
      }
      break;
    }
  }
  mysql_free_result(result);

  if (flag == 0) { // If quotation doesn't exist.
    String *title = newString("Cotización no encontrada.");
    String *msg = newString("Error: No existe ninguna cotización con el número introducido.");
    showAlert(title, msg, tHeight / 2, 1);
    deleteString(title);
    deleteString(msg);
    return;
  } else if (flag == 1) { // If quotation exists but is already invoiced.
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
  ptrArrayAppend(newString("Cantidad"), headings);
  ptrArrayAppend(newString("Precio"), headings);
  ptrArrayAppend(newString("Total"), headings);

  IntArray *widths = newIntArray();
  intArrayAppend(3, widths);
  intArrayAppend(20, widths);
  intArrayAppend(40, widths);
  intArrayAppend(10, widths);
  intArrayAppend(10, widths);
  intArrayAppend(15, widths);

  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  PtrArray *ids = newPtrArray(); // This stores only product ids.


  char buf[512] = {0};
  sprintf(buf, "SELECT p.descripcion, f.descripcion, dc.cantidad, p.precio, p.precio * dc.cantidad, p.id_producto FROM Producto p JOIN Familia f ON p.id_familia = f.id_familia JOIN DetalleCotizacion dc ON p.id_producto = dc.id_producto WHERE dc.id_cotizacion = %i;", quotationId);

  if (mysql_query(conn, buf)) {
    printw("Error al consultar: %s\n", mysql_error(conn));
    refresh();
    getch();
    return;
  }

  MYSQL_RES *res = mysql_store_result(conn);
  if (!res) {
    printw("Error al obtener resultados de familias\n");
    refresh();
    getch();
    return;
  }

  int i = 1;
  while ((row = mysql_fetch_row(res))) {
    PtrArray *tableRow = newPtrArray();
    ptrArrayAppend(newStringI(i), tableRow);
    ptrArrayAppend(newString(row[0]), tableRow);
    ptrArrayAppend(newString(row[1]), tableRow);
    ptrArrayAppend(newString(row[2]), tableRow);
    ptrArrayAppend(newString(row[3]), tableRow);
    ptrArrayAppend(newString(row[4]), tableRow);
    ptrArrayAppend(tableRow, rows);
    ptrArrayAppend(newString(row[5]), ids);
    i++;
  }
  mysql_free_result(res);

  String *helpBar1 = newString("Puede usar las flechas para subir y bajar");
  String *helpBar2 = newString("Agregar producto: +  |  Eliminar producto: -  |  Guardar: <Enter>");
  int height = 7;
  /*const int MAX_ROWS = tHeight - 15;*/
  const int MAX_ROWS = 5;
  if (rows->len > MAX_ROWS) {
    height += MAX_ROWS;
  } else {
    height += rows->len;
  }
  int width = 0;
  for (int i = 0; i < widths->len; i++) {
    width += widths->data[i];
  }
  width += headings->len + 1;

  int ulCornerRow = (tHeight - height) / 2;
  int ulCornerCol = (tWidth - width) / 2;
  Cell ulCorner = {ulCornerRow, ulCornerCol};

  String *subtotalStr = newString("Subtotal │");
  String *taxesStr = newString("Impuesto de venta 13% │");
  String *totalStr = newString("Total │");
  title = newString("Crear cotización");
  int initialRow = 0;
  int keyPressed = 0;
  int numVisibleRows = height - 7;
  do {
    height = 7;
    if (rows->len > MAX_ROWS) {
      height += MAX_ROWS;
    } else {
      height += rows->len;
    }
    numVisibleRows = height - 7;
    ulCornerRow = (tHeight - height) / 2;
    ulCornerCol = (tWidth - width) / 2;
    ulCorner = (Cell){ulCornerRow, ulCornerCol};

    clear();
    printRectangle(ulCorner, width, height);
    move(ulCornerRow + 1, ulCornerCol + 1);
    printRow(headings, widths);
    printLineD(ulCornerRow + 2, ulCornerCol + 1, width - 1, 1);

    int detailsRow = ulCornerRow + 3;
    for (int i = 0; i < numVisibleRows; i++) {
      PtrArray *row = rows->data[initialRow + i];
      String *num = newStringI(i + 1);
      deleteString(row->data[0]);
      row->data[0] = num;
      move(detailsRow + i, ulCornerCol + 1);
      printRow(row, widths);
    }
    int summaryRow = detailsRow + rows->len;
    printLineD(summaryRow, ulCornerCol + 1, width - 1, 1);

    double subtotal = 0;
    for (int i = 0; i < rows->len; i++) {
      PtrArray *row = rows->data[i];
      String *totalStr = row->data[5];
      subtotal += toDouble(totalStr);
    }
    double taxes = subtotal * 0.13;
    double total = subtotal + taxes;
    // The + 1 is to take into account the column separator.
    int c = ulCornerCol + width - widths->data[5] + 1;
    int subtotalCol = c - subtotalStr->len;
    int taxesCol = c - taxesStr->len;
    int totalCol = c - totalStr->len;
    mvprintw(summaryRow + 1, subtotalCol, "%.*s %.1f",
             subtotalStr->len, subtotalStr->text, subtotal);
    mvprintw(summaryRow + 2, taxesCol, "%.*s %.1f",
             taxesStr->len, taxesStr->text, taxes);
    mvprintw(summaryRow + 3, totalCol, "%.*s %.1f",
             totalStr->len, totalStr->text, total);
    move(tHeight - 3, ulCornerCol);
    printCentered(helpBar1, width);
    move(tHeight - 2, ulCornerCol);
    printCentered(helpBar2, width);
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
      String *id = showCatalog(conn);
      char query[256] = {0};
      sprintf(query,
              "SELECT * FROM Producto AS p JOIN Familia as f WHERE p.id_producto = \"%.*s\" && p.id_familia = f.id_familia",
              id->len,
              id->text);
      if (mysql_query(conn, query)) {
        printw("Error al consultar producto: %s\n", mysql_error(conn));
        refresh();
        getch();
        return;
      }
      MYSQL_RES *productResult = mysql_store_result(conn);
      if (!productResult) {
        printw("Error al obtener resultados de producto\n");
        refresh();
        getch();
        return;
      }
      int numRows = mysql_num_rows(productResult);
      if (numRows == 0) {
        mysql_free_result(productResult);
        deleteString(id);
        break;
      }
      clearBlock((Cell){0, 0}, width - 1, 5);
      String *title = newString("Ingrese la cantidad");
      String *amountStr = showInput(title, 2, 0);
      while (!amountStr || !isNumber(amountStr) || toInt(amountStr) < 1) {
        deleteString(amountStr);
        amountStr = showInput(title, 2, 1);
      }
      deleteString(title);

      int amount = toInt(amountStr);
      MYSQL_ROW dbRow = mysql_fetch_row(productResult);
      int stock = atoi(dbRow[2]);
      int currAmount = 0;
      int isUsed = 0;
      for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        if (compareStringToBuffer(row->data[1], dbRow[1])) {
          currAmount = toInt(row->data[3]);
          if (currAmount + amount > stock) {
            break;
          }
          double price = toDouble(row->data[4]);
          double total = (currAmount + amount) * price;
          deleteString(row->data[3]);
          row->data[3] = newStringI(currAmount + amount);
          deleteString(row->data[5]);
          row->data[5] = newStringD(total);
          isUsed = 1;
          break;
        }
      }

      if (currAmount + amount > stock) {
        String *msg = newString("Stock insuficiente");
        showAlert(NULL, msg, 3, 1);
        deleteString(msg);
        mysql_free_result(productResult);
        deleteString(amountStr);
        deleteString(id);
        break;
      }

      if (isUsed) {
        mysql_free_result(productResult);
        deleteString(amountStr);
        deleteString(id);
        break;
      }

      PtrArray *newRow = newPtrArray();
      ptrArrayAppend(newStringI(rows->len + 1), newRow); // Index.
      ptrArrayAppend(newString(dbRow[1]), newRow); // Name.
      ptrArrayAppend(newString(dbRow[7]), newRow); // Family.
      ptrArrayAppend(amountStr, newRow); // Amount.
      ptrArrayAppend(newString(dbRow[4]), newRow); // Price.
      ptrArrayAppend(newStringD(amount * atof(dbRow[4])), newRow); // Total.
      ptrArrayAppend(newRow, rows);
      mysql_free_result(productResult);
      ptrArrayAppend(id, ids);
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
        deleteString(ids->data[row]);
        ptrArrayRemove(row, ids);
      }

      break;
    }
    }
  } while (keyPressed != '\n');

  deleteString(title);
  deleteString(helpBar1);
  deleteString(helpBar2);
  deleteString(subtotalStr);
  deleteString(taxesStr);
  deleteString(totalStr);

  if (rows->len == 0) {
    String *s = newString("No se efectuaron cambios");
    showAlert(NULL, s, 3, 0);
    deleteString(s);
    deleteStringArray(headings);
    deleteIntArray(widths);
    for (int i = 0; i < rows->len; i++) {
      deleteStringArray(rows->data[i]);
      deleteString(ids->data[i]);
    }
    deletePtrArray(rows);
    deletePtrArray(ids);
    return;
  }

  char bufDel[100] = {0};
  sprintf(bufDel, "DELETE FROM DetalleCotizacion WHERE id_cotizacion = %i", quotationId);

  if (mysql_query(conn, bufDel)) {
    printw("Error al consultar: %s\n", mysql_error(conn));
    refresh();
    getch();
    return;
  }

  MYSQL_STMT *stmtDet;
  MYSQL_BIND bindDet[4];

  stmtDet = mysql_stmt_init(conn);
  if (stmtDet == NULL) {
    printw("Error inicializando stmt.");
    getch();
    exit(1);
  }

  for (int i = 0; i < rows->len; i++) {
    char query[] = "CALL AgregarDetalleCotizacion(?, ?, ?, @resultDet)";
    if (mysql_stmt_prepare(stmtDet, query, strlen(query))) {
      printw("%s", mysql_stmt_error(stmtDet));
      getch();
      exit(1);
    }

    memset(bindDet, 0, sizeof(bindDet));

    bindDet[0].buffer_type = MYSQL_TYPE_LONG;
    bindDet[0].buffer = (char*)&quotationId;
    bindDet[0].is_null = 0;
    bindDet[0].length = 0;

    String *id = ids->data[i];
    bindDet[1].buffer_type = MYSQL_TYPE_STRING;
    bindDet[1].buffer = id->text;
    bindDet[1].buffer_length = id->len;
    bindDet[1].is_null = 0;
    bindDet[1].length = 0;

    PtrArray *row = rows->data[i];
    String *amountStr = row->data[3];
    int amount = toInt(amountStr);
    bindDet[2].buffer_type = MYSQL_TYPE_LONG;
    bindDet[2].buffer = (char *)&amount;
    bindDet[2].is_null = 0;
    bindDet[2].length = 0;

    if (mysql_stmt_bind_param(stmtDet, bindDet)) {
      printw("%s", mysql_stmt_error(stmtDet));
      getch();
      exit(1);
    }

    if (mysql_stmt_execute(stmtDet)) {
      printw("%s", mysql_stmt_error(stmtDet));
      getch();
      exit(1);
    }

    if (mysql_query(conn, "SELECT @resultDet")) {
      printw("%s", mysql_error(conn));
      getch();
      exit(1);
    }

    MYSQL_RES *resDet = mysql_store_result(conn);
    if (resDet == NULL) {
      printw("%s", mysql_error(conn));
      getch();
      exit(1);
    }

    //MYSQL_ROW rowDet  = mysql_fetch_row(resDet);
    //int result = atoi(rowDet[0]);
    mysql_free_result(resDet);
  }
  mysql_stmt_close(stmtDet);

  deleteStringArray(headings);
  deleteIntArray(widths);
  for (int i = 0; i < rows->len; i++) {
    deleteStringArray(rows->data[i]);
    deleteString(ids->data[i]);
  }
  deletePtrArray(rows);
  deletePtrArray(ids);

  String *s = newString("Cotización editada satisfactoriamente");
  showAlert(NULL, s, 3, 0);
  deleteString(s);
}

void makeInvoice(MYSQL *conn) {
  int tWidth = 0;
  int tHeight = 0;
  getmaxyx(stdscr, tHeight, tWidth);
  clear();
  String *title = newString("Ingrese el número de cotización");
  String *input = showInput(title, tHeight / 2, 0);
  while (input == NULL || !isNumber(input) || toInt(input) < 1) {
    deleteString(input);
    input = showInput(title, tHeight / 2, 1);
  }
  int quotationId = toInt(input);
  deleteString(title);
  deleteString(input);

  if (mysql_query(conn, "SELECT * FROM Cotizacion")) {
    printw("Error al consultar cotizaciones: %s\n", mysql_error(conn));
    getch();
    exit(1);
  }

  MYSQL_RES *result = mysql_store_result(conn);
  if (!result) {
    printw("Error al obtener resultados de cotizaciones\n");
    getch();
    exit(1);
  }

  int flag = 0; // 0: unknown id. 1: invoiced id. 2: OK id.
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) {
    if (quotationId == atoi(row[0])) {
      if (strcmp(row[1], "Pendiente") == 0) {
        flag = 2;
      } else {
        flag = 1;
      }
      break;
    }
  }
  mysql_free_result(result);

  if (flag == 0) { // If quotation doesn't exist.
    String *title = newString("Cotización no encontrada.");
    String *msg = newString("Error: No existe ninguna cotización con el número introducido.");
    showAlert(title, msg, tHeight / 2, 1);
    deleteString(title);
    deleteString(msg);
    return;
  } else if (flag == 1) { // If quotation exists but is already invoiced.
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

  if (mysql_query(conn, "SELECT COALESCE(MAX(id_factura), 1) FROM Factura")) {
    printw("Error al consultar cotizaciones: %s\n", mysql_error(conn));
    getch();
    exit(1);
  }

  result = mysql_store_result(conn);
  if (!result) {
    printw("Error al obtener resultados de cotizaciones\n");
    getch();
    exit(1);
  }

  row = mysql_fetch_row(result);
  String *invoiceId = newStringI(atoi(row[0]));
  mysql_free_result(result);
  String *localName = newString("PulpeTEC");
  String *legalId = newString("3-101-123456");
  String *phoneNum = newString("1234-5678");
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);
  char buffer[1024] = {0};
  snprintf(buffer, 1024, "%i-%i-%i", lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
  String *date = newString(buffer);

  PtrArray *rows = newPtrArray(); // This is a list of lists of strings.
  PtrArray *ids = newPtrArray();

  char buf[512] = {0};
  sprintf(buf, "SELECT p.descripcion, f.descripcion, dc.cantidad, p.precio, p.precio * dc.cantidad, p.id_producto FROM Producto p JOIN Familia f ON p.id_familia = f.id_familia JOIN DetalleCotizacion dc ON p.id_producto = dc.id_producto WHERE dc.id_cotizacion = %i;", quotationId);

  if (mysql_query(conn, buf)) {
    printw("Error al consultar: %s\n", mysql_error(conn));
    refresh();
    getch();
    return;
  }

  MYSQL_RES *res = mysql_store_result(conn);
  if (!res) {
    printw("Error al obtener resultados de familias\n");
    refresh();
    getch();
    return;
  }

  int i = 1;
  while ((row = mysql_fetch_row(res))) {
    PtrArray *tableRow = newPtrArray();
    ptrArrayAppend(newStringI(i), tableRow);
    ptrArrayAppend(newString(row[0]), tableRow);
    /*ptrArrayAppend(newString(row[1]), tableRow);*/
    ptrArrayAppend(newString(row[2]), tableRow);
    ptrArrayAppend(newString(row[3]), tableRow);
    ptrArrayAppend(newString(row[4]), tableRow);
    ptrArrayAppend(tableRow, rows);
    ptrArrayAppend(newString(row[5]), ids);
    i++;
  }
  mysql_free_result(res);

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
  intArrayAppend(15, widths);

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


    double subtotal = 0;
    for (int i = 0; i < rows->len; i++) {
      PtrArray *row = rows->data[i];
      String *totalStr = row->data[4];
      subtotal += toDouble(totalStr);
    }
    double taxes = subtotal * 0.13;
    double total = subtotal + taxes;
    // The + 1 is to take into account the column separator.
    int c = ulCornerCol + width - widths->data[4] + 1;
    int subtotalCol = c - subtotalStr->len;
    int taxesCol = c - taxesStr->len;
    int totalCol = c - totalStr->len;
    mvprintw(summaryRow + 1, subtotalCol, "%.*s %.1f",
             subtotalStr->len, subtotalStr->text, subtotal);
    mvprintw(summaryRow + 2, taxesCol, "%.*s %.1f",
             taxesStr->len, taxesStr->text, taxes);
    mvprintw(summaryRow + 3, totalCol, "%.*s %.1f",
             totalStr->len, totalStr->text, total);
    keyPressed = getch();
  } while (keyPressed != '\n');

  String *s = newString("¿Desea facturar? Ingrese s/n");
  int opt = showCharAlert(NULL, s, 3, 0);
  deleteString(s);
  if (opt == 's') {
    MYSQL_STMT *stmtInv;
    MYSQL_BIND bindInv[4];

    stmtInv = mysql_stmt_init(conn);
    if (stmtInv == NULL) {
      printw("Error inicializando stmt.");
      getch();
      exit(1);
    }

    char query[] = "CALL AgregarFactura(?, ?, @idFactura, @resultado)";
    if (mysql_stmt_prepare(stmtInv, query, strlen(query))) {
      printw("%s", mysql_stmt_error(stmtInv));
      getch();
      exit(1);
    }

    memset(bindInv, 0, sizeof(bindInv));

    bindInv[0].buffer_type = MYSQL_TYPE_LONG;
    bindInv[0].buffer = (char*)&quotationId;
    bindInv[0].is_null = 0;
    bindInv[0].length = 0;

    bindInv[1].buffer_type = MYSQL_TYPE_STRING;
    bindInv[1].buffer = (char*)client->text;
    bindInv[1].buffer_length = 50;
    bindInv[1].is_null = 0;
    unsigned long len = client->len;
    bindInv[1].length = &len;

    if (mysql_stmt_bind_param(stmtInv, bindInv)) {
      printw("%s", mysql_stmt_error(stmtInv));
      getch();
      exit(1);
    }

    if (mysql_stmt_execute(stmtInv)) {
      printw("%s", mysql_stmt_error(stmtInv));
      getch();
      exit(1);
    }

    if (mysql_query(conn, "SELECT @idFactura, @resultado")) {
      printw("%s", mysql_error(conn));
      getch();
      exit(1);
    }

    MYSQL_RES *resInv = mysql_store_result(conn);
    if (resInv == NULL) {
      printw("%s", mysql_error(conn));
      getch();
      exit(1);
    }

    MYSQL_ROW rowInv  = mysql_fetch_row(resInv);
    int result = atoi(rowInv[1]);
    result++; // to use after.
    mysql_free_result(resInv);
    mysql_stmt_close(stmtInv);

    String *s = newString("Cotización facturada satisfactoriamente");
    showAlert(NULL, s, 3, 0);
    deleteString(s);
  }

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
    deleteString(ids->data[i]);
  }
  deletePtrArray(rows);
  deletePtrArray(ids);
  deleteString(invoiceId);
  deleteString(localName);
  deleteString(legalId);
  deleteString(phoneNum);
  deleteString(date);
  deleteString(client);
}

#endif
