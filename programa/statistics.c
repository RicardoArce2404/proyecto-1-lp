#include "ui.c"
#include <mysql/mysql.h>
#include "array.c"
#include "string.c"



/**
 * Muestra la cantidad de cotizaciones pendientes
 */
void showPendingQuotes(MYSQL *conn) {
    if (mysql_query(conn, "SELECT CotizacionesPendientes()")) {
        String *error = newString("Error al consultar cotizaciones pendientes");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        String *error = newString("Error al obtener resultados");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row && row[0]) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Cotizaciones pendientes: %s", row[0]);
        String *message = newString(msg);
        showAlert(NULL, message, 10, 0);
        deleteString(message);
    }

    mysql_free_result(result);
}

/**
 * Muestra la cantidad de cotizaciones facturadas
 */
void showInvoicedQuotes(MYSQL *conn) {
    if (mysql_query(conn, "SELECT CotizacionesFacturadas()")) {
        String *error = newString("Error al consultar cotizaciones facturadas");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        String *error = newString("Error al obtener resultados");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row && row[0]) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Cotizaciones facturadas: %s", row[0]);
        String *message = newString(msg);
        showAlert(NULL, message, 10, 0);
        deleteString(message);
    }

    mysql_free_result(result);
}

/**
 * Muestra el promedio de compra
 */
void showAveragePurchase(MYSQL *conn) {
    if (mysql_query(conn, "SELECT fn_PromedioCompra()")) {
        String *error = newString("Error al consultar promedio de compra");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        String *error = newString("Error al obtener resultados");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row && row[0]) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Promedio de compra: ₡%.2f", atof(row[0]));
        String *message = newString(msg);
        showAlert(NULL, message, 10, 0);
        deleteString(message);
    }

    mysql_free_result(result);
}


/**
 * Muestra el top 5 de productos más vendidos
 */
void showTop5Products(MYSQL *conn) {
    if (mysql_query(conn, "SELECT producto, cantidad_vendida FROM VistaTop5ProductosMasVendidos")) {
        String *error = newString("Error al consultar top 5 productos");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        String *error = newString("Error al obtener resultados");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("Producto"), headings);
    ptrArrayAppend(newString("Cantidad Vendida"), headings);

    IntArray *widths = newIntArray();
    intArrayAppend(40, widths);
    intArrayAppend(20, widths);

    PtrArray *rows = newPtrArray();
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        PtrArray *dataRow = newPtrArray();
        ptrArrayAppend(newString(row[0]), dataRow);
        
        char cantidad[20];
        snprintf(cantidad, sizeof(cantidad), "%s", row[1]);
        ptrArrayAppend(newString(cantidad), dataRow);
        
        ptrArrayAppend(dataRow, rows);
    }

    String *title = newString("Top 5 Productos Más Vendidos");
    showScrollableList(title, headings, rows, widths, 0);
    
    // Mostrar mensaje en la parte inferior
    int tHeight = getmaxy(stdscr);
    String *helpBar = newString("Presione cualquier tecla para continuar.");
    mvprintw(tHeight - 1, (getmaxx(stdscr) - helpBar->len) / 2, "%.*s", helpBar->len, helpBar->text);
    deleteString(helpBar);
    refresh();
    getch();
    
    deleteString(title);
    deleteStringArray(headings);
    deleteIntArray(widths);
    
    for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        deleteStringArray(row);
    }
    deletePtrArray(rows);
    
    mysql_free_result(result);
}

/**
 * Muestra el producto más vendido por familia
 */
void showTopProductByFamily(MYSQL *conn) {
    if (mysql_query(conn, "SELECT familia, producto, cantidad_vendida FROM VistaProductoMasVendidoPorFamilia")) {
        String *error = newString("Error al consultar productos por familia");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        String *error = newString("Error al obtener resultados");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("Familia"), headings);
    ptrArrayAppend(newString("Producto"), headings);
    ptrArrayAppend(newString("Cantidad Vendida"), headings);

    IntArray *widths = newIntArray();
    intArrayAppend(25, widths);
    intArrayAppend(30, widths);
    intArrayAppend(20, widths);

    PtrArray *rows = newPtrArray();
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        PtrArray *dataRow = newPtrArray();
        ptrArrayAppend(newString(row[0]), dataRow);
        ptrArrayAppend(newString(row[1]), dataRow);
        
        char cantidad[20];
        snprintf(cantidad, sizeof(cantidad), "%s", row[2]);
        ptrArrayAppend(newString(cantidad), dataRow);
        
        ptrArrayAppend(dataRow, rows);
    }

    String *title = newString("Producto Más Vendido por Familia");
    showScrollableList(title, headings, rows, widths, 0);
    
    // Mostrar mensaje en la parte inferior
    int tHeight = getmaxy(stdscr);
    String *helpBar = newString("Presione cualquier tecla para continuar.");
    mvprintw(tHeight - 1, (getmaxx(stdscr) - helpBar->len) / 2, "%.*s", helpBar->len, helpBar->text);
    deleteString(helpBar);
    refresh();
    getch();
    
    deleteString(title);
    deleteStringArray(headings);
    deleteIntArray(widths);
    
    for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        deleteStringArray(row);
    }
    deletePtrArray(rows);
    
    mysql_free_result(result);
}

/**
 * Muestra el monto vendido por familia
 */
void showSalesByFamily(MYSQL *conn) {
    if (mysql_query(conn, "SELECT familia, monto_total FROM VistaMontoVendidoPorFamilia")) {
        String *error = newString("Error al consultar ventas por familia");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        String *error = newString("Error al obtener resultados");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return;
    }

    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("Familia"), headings);
    ptrArrayAppend(newString("Monto Total"), headings);

    IntArray *widths = newIntArray();
    intArrayAppend(30, widths);
    intArrayAppend(20, widths);

    PtrArray *rows = newPtrArray();
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        PtrArray *dataRow = newPtrArray();
        ptrArrayAppend(newString(row[0]), dataRow);
        
        char monto[20];
        snprintf(monto, sizeof(monto), "₡%.2f", atof(row[1]));
        ptrArrayAppend(newString(monto), dataRow);
        
        ptrArrayAppend(dataRow, rows);
    }

    String *title = newString("Monto Vendido por Familia");
    showScrollableList(title, headings, rows, widths, 0);
    
    // Mostrar mensaje en la parte inferior
    int tHeight = getmaxy(stdscr);
    String *helpBar = newString("Presione cualquier tecla para continuar.");
    mvprintw(tHeight - 1, (getmaxx(stdscr) - helpBar->len) / 2, "%.*s", helpBar->len, helpBar->text);
    deleteString(helpBar);
    refresh();
    getch();
    
    deleteString(title);
    deleteStringArray(headings);
    deleteIntArray(widths);
    
    for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        deleteStringArray(row);
    }
    deletePtrArray(rows);
    
    mysql_free_result(result);
}

/**
 * Muestra el menú principal de estadísticas
 */
void statistics(MYSQL *conn) {
    clear();
    PtrArray *options = newPtrArray();
    ptrArrayAppend(newString("Cantidad de cotizaciones pendientes"), options);
    ptrArrayAppend(newString("Cantidad de cotizaciones facturadas"), options);
    ptrArrayAppend(newString("Promedio de compra"), options);
    ptrArrayAppend(newString("Top 5 productos más vendidos"), options);
    ptrArrayAppend(newString("Producto más vendido por familia"), options);
    ptrArrayAppend(newString("Monto vendido por familia"), options);
    ptrArrayAppend(newString("Regresar"), options);

    int choice;
    do {
        choice = showMenu(options);
        clear();
        
        switch(choice) {
            case 0:
                showPendingQuotes(conn);
                break;
            case 1:
                showInvoicedQuotes(conn);
                break;
            case 2:
                showAveragePurchase(conn);
                break;
            case 3:
                showTop5Products(conn);
                break;
            case 4:
                showTopProductByFamily(conn);
                break;
            case 5:
                showSalesByFamily(conn);
                break;
            case 6:
                break;
        }
    } while(choice != 6);

    // Corregido: Liberar memoria correctamente
    for (int i = 0; i < options->len; i++) {
        deleteString(options->data[i]);
    }
    deletePtrArray(options);
}