#include "ui.c"
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>
#include <ncurses.h>
#include "array.c"
#include "string.c"
#include "filehandling.c"

#define MAX_INPUT_LENGTH 1024
#define MAX_PATH_LENGTH 256
#define MAX_QUERY_LENGTH 512

// Estructura para almacenar información básica de factura
typedef struct {
    int id;
    String *fecha;
    String *hora;
    float subtotal;
    float impuesto;
    float total;
} InvoiceSummary;

// Estructura para almacenar detalles de factura
typedef struct {
    String *id_producto;
    String *descripcion;
    int cantidad;
    float precio_unitario;
    float subtotal;
} InvoiceDetail;

// Estructura para información completa de factura
typedef struct {
    int id;
    String *nombre_empresa;
    String *cedula_juridica;
    String *telefono;
    String *fecha;
    String *hora;
    String *cliente;
    PtrArray *detalles;  // Array de InvoiceDetail
    float subtotal;
    float impuesto;
    float total;
} InvoiceFull;

// Funciones para liberar memoria
void freeInvoiceSummary(void *summary) {
    if (!summary) return;
    InvoiceSummary *is = (InvoiceSummary *)summary;
    if (is->fecha) deleteString(is->fecha);
    if (is->hora) deleteString(is->hora);
    free(is);
}

void freeInvoiceDetail(void *detail) {
    if (!detail) return;
    InvoiceDetail *id = (InvoiceDetail *)detail;
    if (id->id_producto) deleteString(id->id_producto);
    if (id->descripcion) deleteString(id->descripcion);
    free(id);
}

void freeInvoiceFull(void *invoice) {
    if (!invoice) return;
    InvoiceFull *inv = (InvoiceFull *)invoice;
    if (inv->nombre_empresa) deleteString(inv->nombre_empresa);
    if (inv->cedula_juridica) deleteString(inv->cedula_juridica);
    if (inv->telefono) deleteString(inv->telefono);
    if (inv->fecha) deleteString(inv->fecha);
    if (inv->hora) deleteString(inv->hora);
    if (inv->cliente) deleteString(inv->cliente);
    if (inv->detalles) {
        for (int i = 0; i < inv->detalles->len; i++) {
            freeInvoiceDetail(inv->detalles->data[i]);
        }
        deletePtrArray(inv->detalles);
    }
    free(inv);
}

// Función para consultar el resumen de facturas usando vista
PtrArray *getInvoiceSummaries(MYSQL *conn) {
    PtrArray *summaries = newPtrArray();
    if (!conn) return summaries;

    const char *query = "SELECT id_factura, fecha, hora, subtotal, impuesto, total "
                       "FROM VistaResumenFacturas "
                       "ORDER BY id_factura DESC";

    if (mysql_query(conn, query)) {
        String *error = newString("Error al consultar resumen de facturas");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return summaries;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        String *error = newString("Error al obtener resultados de facturas");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return summaries;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        // Verificar que todos los campos necesarios no son NULL
        if (!row[0] || !row[1] || !row[2] || !row[3] || !row[4] || !row[5]) {
            continue; // O manejar el error adecuadamente
        }

        InvoiceSummary *summary = (InvoiceSummary *)malloc(sizeof(InvoiceSummary));
        if (!summary) continue;

        // Usar newStringN para especificar longitud máxima segura
        summary->id = atoi(row[0]);
        summary->fecha = row[1] ? newStringN(row[1], 64) : newString(""); // Fecha máxima 64 chars
        summary->hora = row[2] ? newStringN(row[2], 16) : newString("");  // Hora máxima 16 chars
        summary->subtotal = row[3] ? atof(row[3]) : 0.0;
        summary->impuesto = row[4] ? atof(row[4]) : 0.0;
        summary->total = row[5] ? atof(row[5]) : 0.0;

        ptrArrayAppend(summary, summaries);
    }

    mysql_free_result(result);
    return summaries;
}

// Función para consultar los detalles de una factura usando vistas
InvoiceFull *getInvoiceDetails(MYSQL *conn, int invoiceId) {
    if (!conn) {
        String *error = newString("Conexión a BD no disponible");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return NULL;
    }

    InvoiceFull *invoice = (InvoiceFull *)malloc(sizeof(InvoiceFull));
    if (!invoice) {
        String *error = newString("Error al asignar memoria para factura");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        return NULL;
    }

    invoice->detalles = newPtrArray();
    invoice->id = invoiceId;

    // Obtener encabezado de la factura usando vista
    char headerQuery[256];
    snprintf(headerQuery, sizeof(headerQuery), 
            "SELECT nombre_empresa, cedula_juridica, telefono, "
            "fecha, hora, cliente, subtotal, impuesto, total "
            "FROM VistaEncabezadoFactura "
            "WHERE id_factura = %d", invoiceId);

    if (mysql_query(conn, headerQuery)) {
        String *error = newString("Error al consultar encabezado de factura");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        freeInvoiceFull(invoice);
        return NULL;
    }

    MYSQL_RES *headerRes = mysql_store_result(conn);
    if (!headerRes || mysql_num_rows(headerRes) == 0) {
        String *error = newString("No se encontró la factura solicitada");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        if (headerRes) mysql_free_result(headerRes);
        freeInvoiceFull(invoice);
        return NULL;
    }

    MYSQL_ROW headerRow = mysql_fetch_row(headerRes);
    invoice->nombre_empresa = newString(headerRow[0]);
    invoice->cedula_juridica = newString(headerRow[1]);
    invoice->telefono = newString(headerRow[2]);
    invoice->fecha = newString(headerRow[3]);
    invoice->hora = newString(headerRow[4]);
    invoice->cliente = newString(headerRow[5]);
    invoice->subtotal = headerRow[6] ? atof(headerRow[6]) : 0.0;
    invoice->impuesto = headerRow[7] ? atof(headerRow[7]) : 0.0;
    invoice->total = headerRow[8] ? atof(headerRow[8]) : 0.0;
    
    mysql_free_result(headerRes);

    // Obtener detalles de productos usando vista
    char detailsQuery[256];
    snprintf(detailsQuery, sizeof(detailsQuery), 
            "SELECT id_producto, descripcion, cantidad, precio_unitario, subtotal "
            "FROM VistaDetallesFactura "
            "WHERE id_factura = %d", invoiceId);

    if (mysql_query(conn, detailsQuery)) {
        String *error = newString("Error al consultar detalles de factura");
        showAlert(NULL, error, 10, 1);
        deleteString(error);
        freeInvoiceFull(invoice);
        return NULL;
    }

    MYSQL_RES *detailsRes = mysql_store_result(conn);
    if (detailsRes) {
        MYSQL_ROW detailsRow;
        while (( detailsRow= mysql_fetch_row(detailsRes))) {
            InvoiceDetail *detail = (InvoiceDetail *)malloc(sizeof(InvoiceDetail));
            if (!detail) continue;

            detail->id_producto = newString(detailsRow[0]);
            detail->descripcion = newString(detailsRow[1]);
            detail->cantidad = detailsRow[2] ? atoi(detailsRow[2]) : 0;
            detail->precio_unitario = detailsRow[3] ? atof(detailsRow[3]) : 0.0;
            detail->subtotal = detailsRow[4] ? atof(detailsRow[4]) : 0.0;

            ptrArrayAppend(detail, invoice->detalles);
        }
        mysql_free_result(detailsRes);
    }

    return invoice;
}

// Función para mostrar el resumen de facturas
void showInvoiceSummaries(PtrArray *summaries) {
    if (!summaries || summaries->len == 0) {
        String *msg = newString("No hay facturas para mostrar");
        showAlert(NULL, msg, 10, 0);
        deleteString(msg);
        return;
    }

    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("ID"), headings);
    ptrArrayAppend(newString("Fecha"), headings);
    ptrArrayAppend(newString("Hora"), headings);
    ptrArrayAppend(newString("Subtotal"), headings);
    ptrArrayAppend(newString("Impuesto"), headings);
    ptrArrayAppend(newString("Total"), headings);

    IntArray *widths = newIntArray();
    intArrayAppend(5, widths);    // ID
    intArrayAppend(10, widths);   // Fecha
    intArrayAppend(8, widths);    // Hora
    intArrayAppend(10, widths);   // Subtotal
    intArrayAppend(10, widths);   // Impuesto
    intArrayAppend(10, widths);   // Total

    PtrArray *rows = newPtrArray();
    
    for (int i = 0; i < summaries->len; i++) {
        InvoiceSummary *summary = (InvoiceSummary *)summaries->data[i];
        PtrArray *row = newPtrArray();
        
        // ID
        char idStr[6];
        snprintf(idStr, sizeof(idStr), "%d", summary->id);
        ptrArrayAppend(newString(idStr), row);
        
        // Fecha
        ptrArrayAppend(newString(summary->fecha->text), row);
        
        // Hora (mostrar solo HH:MM:SS)
        char hora[9] = {0};
        strncpy(hora, summary->hora->text, 8);
        ptrArrayAppend(newString(hora), row);
        
        // Subtotal
        char subtotalStr[12];
        snprintf(subtotalStr, sizeof(subtotalStr), "%.2f", summary->subtotal);
        ptrArrayAppend(newString(subtotalStr), row);
        
        // Impuesto
        char impuestoStr[12];
        snprintf(impuestoStr, sizeof(impuestoStr), "%.2f", summary->impuesto);
        ptrArrayAppend(newString(impuestoStr), row);
        
        // Total
        char totalStr[12];
        snprintf(totalStr, sizeof(totalStr), "%.2f", summary->total);
        ptrArrayAppend(newString(totalStr), row);
        
        ptrArrayAppend(row, rows);
    }

    String *title = newString("Listado de Facturas");
    String *helpBar1 = newString("Use las flechas para navegar");
    String *helpBar2 = newString("Presione Enter para ver detalles o ESC para salir");

    int tWidth = 0, tHeight = 0;
    getmaxyx(stdscr, tHeight, tWidth);
    int initialRow = 0;
    int keyPressed = 0;

    do {
        showScrollableList(title, headings, rows, widths, initialRow);
        
        move(tHeight - 2, 1);
        printCentered(helpBar1, tWidth);
        move(tHeight - 1, 1);
        printCentered(helpBar2, tWidth);
        
        keyPressed = getch();
        if (keyPressed == KEY_UP && initialRow > 0) {
            initialRow--;
        } else if (keyPressed == KEY_DOWN && initialRow + 10 < rows->len) {
            initialRow++;
        }
    } while (keyPressed != '\n' && keyPressed != 27); // Enter o ESC

    deleteString(title);
    deleteString(helpBar1);
    deleteString(helpBar2);
    deleteStringArray(headings);
    deleteIntArray(widths);
    
    for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        deleteStringArray(row);
    }
    deletePtrArray(rows);
}

// Función para mostrar los detalles completos de una factura
void showInvoiceDetails(MYSQL *conn, int invoiceId) {
    InvoiceFull *invoice = getInvoiceDetails(conn, invoiceId);
    if (!invoice) {
        return;
    }

    // Mostrar encabezado de la factura
    clear();
    mvprintw(0, 0, "Factura #%d", invoice->id);
    mvprintw(1, 0, "Empresa: %s", invoice->nombre_empresa->text);
    mvprintw(2, 0, "Cédula: %s", invoice->cedula_juridica->text);
    mvprintw(3, 0, "Teléfono: %s", invoice->telefono->text);
    mvprintw(4, 0, "Fecha: %s  Hora: %s", invoice->fecha->text, invoice->hora->text);
    mvprintw(5, 0, "Cliente: %s", invoice->cliente->text);
    
    // Mostrar tabla de detalles
    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("ID Producto"), headings);
    ptrArrayAppend(newString("Descripción"), headings);
    ptrArrayAppend(newString("Cantidad"), headings);
    ptrArrayAppend(newString("Precio Unit."), headings);
    ptrArrayAppend(newString("Subtotal"), headings);

    IntArray *widths = newIntArray();
    intArrayAppend(12, widths);   // ID Producto
    intArrayAppend(25, widths);   // Descripción
    intArrayAppend(10, widths);   // Cantidad
    intArrayAppend(12, widths);   // Precio Unit.
    intArrayAppend(12, widths);   // Subtotal

    PtrArray *rows = newPtrArray();
    
    for (int i = 0; i < invoice->detalles->len; i++) {
        InvoiceDetail *detail = (InvoiceDetail *)invoice->detalles->data[i];
        PtrArray *row = newPtrArray();
        
        ptrArrayAppend(newString(detail->id_producto->text), row);
        
        // Descripción (limitada a 25 caracteres)
        char desc[26];
        snprintf(desc, sizeof(desc), "%.25s", detail->descripcion->text);
        ptrArrayAppend(newString(desc), row);
        
        // Cantidad
        char cantidadStr[12];
        snprintf(cantidadStr, sizeof(cantidadStr), "%d", detail->cantidad);
        ptrArrayAppend(newString(cantidadStr), row);
        
        // Precio Unitario
        char precioStr[12];
        snprintf(precioStr, sizeof(precioStr), "%.2f", detail->precio_unitario);
        ptrArrayAppend(newString(precioStr), row);
        
        // Subtotal
        char subtotalStr[12];
        snprintf(subtotalStr, sizeof(subtotalStr), "%.2f", detail->subtotal);
        ptrArrayAppend(newString(subtotalStr), row);
        
        ptrArrayAppend(row, rows);
    }

    // Mostrar la tabla de detalles
    String *title = newString("Detalles de la Factura");
    showScrollableList(title, headings, rows, widths, 0);
    
    // Mostrar totales
    int tHeight = getmaxy(stdscr);
    mvprintw(tHeight - 4, 0, "Subtotal: %.2f", invoice->subtotal);
    mvprintw(tHeight - 3, 0, "Impuesto (13%%): %.2f", invoice->impuesto);
    mvprintw(tHeight - 2, 0, "Total: %.2f", invoice->total);
    
    String *continueMsg = newString("Presione cualquier tecla para continuar");
    showAlert(NULL, continueMsg, tHeight - 1, 0);
    deleteString(continueMsg);

    // Liberar memoria
    deleteString(title);
    deleteStringArray(headings);
    deleteIntArray(widths);
    
    for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        deleteStringArray(row);
    }
    deletePtrArray(rows);
    
    freeInvoiceFull(invoice);
}

// Función principal para consultar facturas
void queryInvoice(MYSQL *conn) {
    clear();
    
    // Mostrar resumen de facturas
    PtrArray *summaries = getInvoiceSummaries(conn);
    if (!summaries || summaries->len == 0) {
        String *msg = newString("No hay facturas registradas");
        showAlert(NULL, msg, 10, 0);
        deleteString(msg);
        if (summaries) deletePtrArray(summaries);
        return;
    }

    showInvoiceSummaries(summaries);
    
    // Obtener entrada del usuario de manera segura
    String *prompt = newString("Ingrese el ID de la factura a consultar (0 para salir):");
    String *input = showInput(prompt, 10, 0);
    deleteString(prompt);
    
    if (!input || input->len == 0) {
        if (input) deleteString(input);
        for (int i = 0; i < summaries->len; i++) {
            freeInvoiceSummary(summaries->data[i]);
        }
        deletePtrArray(summaries);
        return;
    }

    if (!isNumber(input)) {
        String *error = newString("Error: Debe ingresar solo dígitos numéricos");
        showAlert(NULL, error, 11, 1);
        deleteString(error);
        deleteString(input);
        for (int i = 0; i < summaries->len; i++) {
            freeInvoiceSummary(summaries->data[i]);
        }
        deletePtrArray(summaries);
        return;
    }

    // Asegurar terminación nula para atoi
    char buffer[32] = {0};
    strncpy(buffer, input->text, input->len > 31 ? 31 : input->len);
    buffer[31] = '\0'; // Garantizar terminación nula
    
    int invoiceId = atoi(buffer);
    deleteString(input);

    if (invoiceId == 0) {
        for (int i = 0; i < summaries->len; i++) {
            freeInvoiceSummary(summaries->data[i]);
        }
        deletePtrArray(summaries);
        return;
    }

    // Verificar que el ID existe
    int found = 0;
    for (int i = 0; i < summaries->len; i++) {
        InvoiceSummary *s = (InvoiceSummary *)summaries->data[i];
        if (s->id == invoiceId) {
            found = 1;
            break;
        }
    }

    if (!found) {
        String *error = newString("Error: No existe una factura con ese ID");
        showAlert(NULL, error, 11, 1);
        deleteString(error);
        for (int i = 0; i < summaries->len; i++) {
            freeInvoiceSummary(summaries->data[i]);
        }
        deletePtrArray(summaries);
        return;
    }

    // Mostrar detalles de la factura seleccionada
    showInvoiceDetails(conn, invoiceId);

    // Liberar memoria
    for (int i = 0; i < summaries->len; i++) {
        freeInvoiceSummary(summaries->data[i]);
    }
    deletePtrArray(summaries);
}