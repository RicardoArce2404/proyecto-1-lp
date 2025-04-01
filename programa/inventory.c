#ifndef INVENTORY_C
#define INVENTORY_C

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

// Definición de estructuras
typedef struct {
    String *id;
    String *description;
} Family;

typedef struct {
    String *id;
    String *description;
    String *family_desc;
    float cost;
    float price;
    int stock;
} Product;

typedef struct {
    String *id;
    String *description;
    int error_code;
} ProcessingError;


typedef struct {
    String *id_producto;
    int cantidad;
    int operacion; // 1 para sumar, 0 para restar
} InventoryUpdate;

typedef struct {
    String *id_producto;
    int cantidad;
    int error_code;
} InventoryError;

void freeInventoryUpdate(void *update) {
    if (!update) return;
    InventoryUpdate *iu = (InventoryUpdate *)update;
    if (iu->id_producto) deleteString(iu->id_producto);
    free(iu);
}

void freeInventoryError(void *error) {
    if (!error) return;
    InventoryError *ie = (InventoryError *)error;
    if (ie->id_producto) deleteString(ie->id_producto);
    free(ie);
}

// Funciones para liberar memoria
void freeFamily(void *family) {
    if (!family) return;
    Family *f = (Family *)family;
    if (f->id) deleteString(f->id);
    if (f->description) deleteString(f->description);
    free(f);
}

void freeProduct(void *product) {
    if (!product) return;
    Product *p = (Product *)product;
    if (p->id) deleteString(p->id);
    if (p->description) deleteString(p->description);
    if (p->family_desc) deleteString(p->family_desc);
    free(p);
}

void freeProcessingError(void *error) {
    if (!error) return;
    ProcessingError *e = (ProcessingError *)error;
    if (e->id) deleteString(e->id);
    if (e->description) deleteString(e->description);
    free(e);
}

String *readStringFromUI(String *prompt, int row) {
    if (!prompt) return NULL;
    return showInput(prompt, row, 0);
}

// Función para mostrar errores en tabla
void showProcessingErrors(PtrArray *errors) {
    if (!errors || errors->len == 0) {
        String *msg = newString("No hay errores para mostrar");
        showAlert(NULL, msg, 10, 0);
        deleteString(msg);
        return;
    }

    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("ID"), headings);
    ptrArrayAppend(newString("Descripción"), headings);
    ptrArrayAppend(newString("Error"), headings);

    IntArray *widths = newIntArray();
    intArrayAppend(10, widths);
    intArrayAppend(30, widths);
    intArrayAppend(30, widths);

    PtrArray *rows = newPtrArray();
    
    for (int i = 0; i < errors->len; i++) {
        ProcessingError *error = (ProcessingError *)errors->data[i];
        PtrArray *row = newPtrArray();
        
        ptrArrayAppend(newString(error->id->text), row);
        ptrArrayAppend(newString(error->description->text), row);
        
        char *errorMsg;
        switch(error->error_code) {
            case 1: errorMsg = "ID ya existe"; break;
            case 2: errorMsg = "Familia no existe"; break;
            case 3: errorMsg = "Costo o precio inválido (debe ser > 0)"; break;
            case 4: errorMsg = "Precio no puede ser menor que costo"; break;
            
            // System errors
            case -1: errorMsg = "Error de sistema: No se pudo inicializar statement"; break;
            case -2: case -11: errorMsg = "Error de sistema: Fallo al preparar consulta"; break;
            case -3: case -12: errorMsg = "Error de sistema: Fallo al bindear parámetros"; break;
            case -4: case -13: errorMsg = "Error de sistema: Fallo al ejecutar procedimiento"; break;
            case -5: errorMsg = "Formato inválido: Campos faltantes en el archivo"; break;
            case -6: errorMsg = "Error de sistema: Fallo al crear strings"; break;
            case -7: errorMsg = "Error de formato: Valores numéricos inválidos"; break;
            case -8: errorMsg = "Error de validación: Valores negativos o cero no permitidos"; break;
            case -9: errorMsg = "Error de sistema: No hay memoria disponible"; break;
            case -10: errorMsg = "Error de sistema: No se pudo inicializar statement"; break;
            case -14: errorMsg = "Error de sistema: Fallo al obtener resultados"; break;
            default: errorMsg = "Error desconocido"; break;
        }
        
        ptrArrayAppend(newString(errorMsg), row);
        ptrArrayAppend(row, rows);
    }

    String *title = newString("Registros no procesados");
    String *helpBar1 = newString("Use las flechas para navegar");
    String *helpBar2 = newString("Presione Enter para continuar");

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
    } while (keyPressed != '\n');

    // Liberar memoria
    deleteString(title);
    deleteString(helpBar1);
    deleteString(helpBar2);
    deleteStringArray(headings);
    deleteIntArray(widths);
    
    for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        for (int j = 0; j < row->len; j++) {
            deleteString(row->data[j]);
        }
        deletePtrArray(row);
    }
    deletePtrArray(rows);
}

// Función para mostrar productos exitosos en tabla
void showSuccessfulProducts(PtrArray *products) {
    if (!products || products->len == 0) {
        String *msg = newString("No hay productos exitosos para mostrar");
        showAlert(NULL, msg, 10, 0);
        deleteString(msg);
        return;
    }

    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("ID"), headings);
    ptrArrayAppend(newString("Descripción"), headings);
    ptrArrayAppend(newString("Familia"), headings);
    ptrArrayAppend(newString("Costo"), headings);
    ptrArrayAppend(newString("Precio"), headings);
    ptrArrayAppend(newString("Stock"), headings);

    IntArray *widths = newIntArray();
    intArrayAppend(10, widths);   // ID
    intArrayAppend(25, widths);   // Descripción
    intArrayAppend(15, widths);   // Familia
    intArrayAppend(10, widths);   // Costo
    intArrayAppend(10, widths);   // Precio
    intArrayAppend(8, widths);    // Stock

    PtrArray *rows = newPtrArray();
    
    for (int i = 0; i < products->len; i++) {
        Product *product = (Product *)products->data[i];
        PtrArray *row = newPtrArray();
        
        // ID
        ptrArrayAppend(newString(product->id->text), row);
        
        // Descripción (limitada a 25 caracteres)
        char desc[26];
        snprintf(desc, sizeof(desc), "%.25s", product->description->text);
        ptrArrayAppend(newString(desc), row);
        
        // Familia (limitada a 15 caracteres)
        char family[16];
        snprintf(family, sizeof(family), "%.15s", product->family_desc->text);
        ptrArrayAppend(newString(family), row);
        
        // Costo (formateado a 2 decimales)
        char costStr[12];
        snprintf(costStr, sizeof(costStr), "%.2f", product->cost);
        ptrArrayAppend(newString(costStr), row);
        
        // Precio (formateado a 2 decimales)
        char priceStr[12];
        snprintf(priceStr, sizeof(priceStr), "%.2f", product->price);
        ptrArrayAppend(newString(priceStr), row);
        
        // Stock
        char stockStr[8];
        snprintf(stockStr, sizeof(stockStr), "%d", product->stock);
        ptrArrayAppend(newString(stockStr), row);
        
        ptrArrayAppend(row, rows);
    }

    String *title = newString("Productos Registrados Exitosamente");
    String *helpBar1 = newString("Use las flechas para navegar");
    String *helpBar2 = newString("Presione Enter para continuar");

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
    } while (keyPressed != '\n');

    // Liberar memoria
    deleteString(title);
    deleteString(helpBar1);
    deleteString(helpBar2);
    deleteStringArray(headings);
    deleteIntArray(widths);
    
    for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        for (int j = 0; j < row->len; j++) {
            deleteString(row->data[j]);
        }
        deletePtrArray(row);
    }
    deletePtrArray(rows);
}

void loadFamiliesFromFile(MYSQL *conn, String *filePath, PtrArray *families) {
    if (!conn || !filePath || !families) return;
    
    PtrArray *csvLines = readCsv(filePath);
    if (!csvLines || csvLines->len == 0) {
        String *msg = newString("Error: Archivo vacío o formato inválido");
        showAlert(NULL, msg, 10, 1);
        deleteString(msg);
        if (csvLines) deletePtrArray(csvLines);
        return;
    }

    PtrArray *successfulFamilies = newPtrArray();
    PtrArray *errorFamilies = newPtrArray();
    int totalRecords = 0, failedRecords = 0;

    for (int i = 0; i < csvLines->len; i++) {
        PtrArray *line = (PtrArray *)csvLines->data[i];
        
        // Validar que la línea tenga exactamente 2 campos (ID y descripción)
        if (line->len != 2) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString("N/A");
            error->description = newString("Formato inválido: Se esperaban 2 campos");
            error->error_code = -5;
            ptrArrayAppend(error, errorFamilies);
            failedRecords++;
            continue;
        }

        String *id = (String *)(line->data[0]);
        String *description = (String *)(line->data[1]);

        // Llamar al procedimiento almacenado
        MYSQL_STMT *stmt;
        MYSQL_BIND params[3];
        int resultado = 0;

        stmt = mysql_stmt_init(conn);
        if (!stmt) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text,id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -1;
            ptrArrayAppend(error, errorFamilies);
            failedRecords++;
            continue;
        }

        const char *query = "CALL RegistrarFamilia(?, ?, ?)";
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id =  newStringN(id->text,id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -2;
            ptrArrayAppend(error, errorFamilies);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        memset(params, 0, sizeof(params));

        params[0].buffer_type = MYSQL_TYPE_STRING;
        params[0].buffer = id->text;
        params[0].buffer_length = id->len;

        params[1].buffer_type = MYSQL_TYPE_STRING;
        params[1].buffer = description->text;
        params[1].buffer_length = description->len;

        params[2].buffer_type = MYSQL_TYPE_LONG;
        params[2].buffer = &resultado;
        params[2].is_unsigned = 0;
        params[2].is_null = 0;

        if (mysql_stmt_bind_param(stmt, params)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text,id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -3;
            ptrArrayAppend(error, errorFamilies);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        if (mysql_stmt_execute(stmt)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text,id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -4;
            ptrArrayAppend(error, errorFamilies);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        // Vincular y recuperar el parámetro de salida
        MYSQL_BIND out_param;
        memset(&out_param, 0, sizeof(out_param));
        out_param.buffer_type = MYSQL_TYPE_LONG;
        out_param.buffer = &resultado;
        out_param.is_unsigned = 0;
        out_param.is_null = 0;

        if (mysql_stmt_bind_result(stmt, &out_param)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text,id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -7;
            ptrArrayAppend(error, errorFamilies);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        if (mysql_stmt_fetch(stmt)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text,id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -8;
            ptrArrayAppend(error, errorFamilies);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        if (resultado != 6) {  
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));

            error->id = newStringN(id->text,id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = resultado;
            ptrArrayAppend(error, errorFamilies);
            failedRecords++;
        } else {
            Family *family = (Family *)malloc(sizeof(Family));
            if (family) {
                family->id = newStringN(id->text,id->len);
                family->description = newStringN(description->text, description->len);
                /*ptrArrayAppend(family, families);*/
                ptrArrayAppend(family, successfulFamilies);
            }
        }

        mysql_stmt_close(stmt);
        totalRecords++;
    }

    // Mostrar resultados exitosos en tabla bien formateada
    if (successfulFamilies->len > 0) {
        clear();
        
        // Configuración de la tabla
        PtrArray *headings = newPtrArray();
        ptrArrayAppend(newString("ID"), headings);
        ptrArrayAppend(newString("Descripción"), headings);

        IntArray *widths = newIntArray();
        intArrayAppend(10, widths);  // Ancho para ID
        intArrayAppend(30, widths);   // Ancho para Descripción

        PtrArray *rows = newPtrArray();
        for (int i = 0; i < successfulFamilies->len; i++) {
            Family *family = (Family *)successfulFamilies->data[i];
            PtrArray *row = newPtrArray();
            
            // Formatear ID para asegurar 10 caracteres
            char idBuffer[11] = "          "; // 10 espacios
            int copyLen = family->id->len < 10 ? family->id->len : 10;
            strncpy(idBuffer, family->id->text, copyLen);
            idBuffer[10] = '\0';
            
            // Formatear descripción para asegurar 30 caracteres
            char descBuffer[31] = "                              "; // 30 espacios
            copyLen = family->description->len < 30 ? family->description->len : 30;
            strncpy(descBuffer, family->description->text, copyLen);
            descBuffer[30] = '\0';
            
            ptrArrayAppend(newString(idBuffer), row);
            ptrArrayAppend(newString(descBuffer), row);
            ptrArrayAppend(row, rows);
        }

        // Mostrar tabla
        String *title = newString("Familias Registradas Exitosamente");
        showScrollableList(title, headings, rows, widths, 0);
        
        // Esperar confirmación
        String *continueMsg = newString("Presione Enter para continuar");
        showAlert(NULL, continueMsg, getmaxy(stdscr) - 2, 0);
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
    }

    // Mostrar resumen
    char summaryText[100];
    snprintf(summaryText, sizeof(summaryText), 
             "Resumen: %d registros procesados, %d fallidos",
             totalRecords, failedRecords);
    
    String *summary = newString(summaryText);
    showAlert(NULL, summary, 10, failedRecords > 0 ? 1 : 0);
    deleteString(summary);

    // Mostrar errores si los hay
    if (errorFamilies->len > 0) {
        showProcessingErrors(errorFamilies);
    }

    // Liberar memoria
    for (int i = 0; i < errorFamilies->len; i++) {
        freeProcessingError(errorFamilies->data[i]);
    }
    deletePtrArray(errorFamilies);
    for (int i = 0; i < successfulFamilies->len; i++) {
      freeFamily(successfulFamilies->data[i]);
    }
    deletePtrArray(successfulFamilies);

    for (int i = 0; i < csvLines->len; i++) {
        PtrArray *line = csvLines->data[i];
        deleteStringArray(line);
    }
    deletePtrArray(csvLines);
}

void loadProductsFromFile(MYSQL *conn, String *filePath, PtrArray *products) {
    if (!conn || !filePath || !products) return;

    PtrArray *csvLines = readCsv(filePath);
    if (!csvLines || csvLines->len == 0) {
        String *msg = newString("Error: Archivo vacío o formato inválido");
        showAlert(NULL, msg, 10, 1);
        deleteString(msg);
        if (csvLines) deletePtrArray(csvLines);
        return;
    }

    PtrArray *successfulProducts = newPtrArray();
    PtrArray *errorProducts = newPtrArray();
    int totalRecords = 0, failedRecords = 0;

    for (int i = 0; i < csvLines->len; i++) {
        PtrArray *line = (PtrArray *)csvLines->data[i];
        
        // Validar que la línea tenga exactamente 6 campos
        if (line->len != 6) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString("N/A");
            error->description = newString("Formato inválido: Se esperaban 6 campos");
            error->error_code = -5;
            ptrArrayAppend(error, errorProducts);
            failedRecords++;
            continue;
        }

        String *id = (String *)line->data[0];
        String *description = (String *)line->data[1];
        String *family_desc = (String *)line->data[2];
        String *costStr = (String *)line->data[3];
        String *priceStr = (String *)line->data[4];
        String *stockStr = (String *)line->data[5];

        // Validar que los strings tengan contenido
        if (id->len <= 0 || description->len <= 0 || family_desc->len <= 0 || 
            costStr->len <= 0 || priceStr->len <= 0 || stockStr->len <= 0) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len > 0 ? id->len : 1);
            error->description = newString("Campos vacíos en el archivo");
            error->error_code = -6;
            ptrArrayAppend(error, errorProducts);
            failedRecords++;
            continue;
        }

        // Validar campos numéricos
        if (!isNumber(costStr) || !isNumber(priceStr) || !isNumber(stockStr)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -7;
            ptrArrayAppend(error, errorProducts);
            failedRecords++;
            continue;
        }

        float cost = atof(costStr->text);
        float price = atof(priceStr->text);
        int stock = atoi(stockStr->text);

        // Validar valores positivos
        if (cost <= 0 || price <= 0 || stock < 0) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -8;
            ptrArrayAppend(error, errorProducts);
            failedRecords++;
            continue;
        }

        // Llamar al procedimiento almacenado
        MYSQL_STMT *stmt;
        MYSQL_BIND params[7];
        int resultado = 0;

        stmt = mysql_stmt_init(conn);
        if (!stmt) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -10;
            ptrArrayAppend(error, errorProducts);
            failedRecords++;
            continue;
        }

        const char *query = "CALL RegistrarProducto(?, ?, ?, ?, ?, ?, ?)";
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -11;
            ptrArrayAppend(error, errorProducts);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        memset(params, 0, sizeof(params));

        params[0].buffer_type = MYSQL_TYPE_STRING;
        params[0].buffer = id->text;
        params[0].buffer_length = id->len;

        params[1].buffer_type = MYSQL_TYPE_STRING;
        params[1].buffer = description->text;
        params[1].buffer_length = description->len;

        params[2].buffer_type = MYSQL_TYPE_LONG;
        params[2].buffer = &stock;
        params[2].is_unsigned = 1;

        params[3].buffer_type = MYSQL_TYPE_FLOAT;
        params[3].buffer = &cost;

        params[4].buffer_type = MYSQL_TYPE_FLOAT;
        params[4].buffer = &price;

        params[5].buffer_type = MYSQL_TYPE_STRING;
        params[5].buffer = family_desc->text;
        params[5].buffer_length = family_desc->len;

        params[6].buffer_type = MYSQL_TYPE_LONG;
        params[6].buffer = &resultado;
        params[6].is_unsigned = 0;
        params[6].is_null = 0;

        if (mysql_stmt_bind_param(stmt, params)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -12;
            ptrArrayAppend(error, errorProducts);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        if (mysql_stmt_execute(stmt)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -13;
            ptrArrayAppend(error, errorProducts);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        // Vincular y recuperar el parámetro de salida
        MYSQL_BIND out_param;
        memset(&out_param, 0, sizeof(out_param));
        out_param.buffer_type = MYSQL_TYPE_LONG;
        out_param.buffer = &resultado;
        out_param.is_unsigned = 0;
        out_param.is_null = 0;

        if (mysql_stmt_bind_result(stmt, &out_param)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -15;
            ptrArrayAppend(error, errorProducts);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        if (mysql_stmt_fetch(stmt)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = -16;
            ptrArrayAppend(error, errorProducts);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        if (resultado != 5) {  // 0 es éxito según tu procedimiento
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newStringN(id->text, id->len);
            error->description = newStringN(description->text, description->len);
            error->error_code = resultado;
            ptrArrayAppend(error, errorProducts);
            failedRecords++;
        } else {
            Product *product = (Product *)malloc(sizeof(Product));
            if (product) {
                product->id = newStringN(id->text, id->len);
                product->description = newStringN(description->text, description->len);
                product->family_desc = newStringN(family_desc->text, family_desc->len);
                product->cost = cost;
                product->price = price;
                product->stock = stock;
                ptrArrayAppend(product, products);
                ptrArrayAppend(product, successfulProducts);
            }
        }

        mysql_stmt_close(stmt);
        totalRecords++;
    }
    
    // Mostrar resultados exitosos en tabla bien formateada
    if (successfulProducts->len > 0) {
        clear();
        showSuccessfulProducts(successfulProducts);
    }

    // Mostrar resumen
    char summaryText[150];
    if (failedRecords > 0) {
        snprintf(summaryText, sizeof(summaryText), 
                "Resumen Productos: %d procesados, %d fallidos (Ver detalles con F1)",
                totalRecords - failedRecords, failedRecords);
    } else {
        snprintf(summaryText, sizeof(summaryText), 
                "Resumen Productos: Todos los %d se procesaron exitosamente",
                totalRecords);
    }

    String *summary = newString(summaryText);
    showAlert(NULL, summary, 10, failedRecords > 0 ? 1 : 0);
    deleteString(summary);

    if (failedRecords > 0) {
        showProcessingErrors(errorProducts);
    }

    // Liberar memoria
    for (int i = 0; i < errorProducts->len; i++) {
        freeProcessingError(errorProducts->data[i]);
    }
    deletePtrArray(errorProducts);
    for (int i = 0; i < successfulProducts->len; i++) {
      freeProduct(successfulProducts->data[i]);
    }
    deletePtrArray(successfulProducts);

    for (int i = 0; i < csvLines->len; i++) {
        PtrArray *line = csvLines->data[i];
        deleteStringArray(line);
    }
    deletePtrArray(csvLines);
}


void showInventoryUpdateResults(PtrArray *successfulUpdates, PtrArray *errorUpdates) {
    if (!successfulUpdates && !errorUpdates) return;

    // Mostrar actualizaciones exitosas
    if (successfulUpdates && successfulUpdates->len > 0) {
        PtrArray *headings = newPtrArray();
        ptrArrayAppend(newString("ID Producto"), headings);
        ptrArrayAppend(newString("Operación"), headings);
        ptrArrayAppend(newString("Cantidad"), headings);

        IntArray *widths = newIntArray();
        intArrayAppend(15, widths);
        intArrayAppend(15, widths);
        intArrayAppend(10, widths);

        PtrArray *rows = newPtrArray();
        
        for (int i = 0; i < successfulUpdates->len; i++) {
            InventoryUpdate *update = (InventoryUpdate *)successfulUpdates->data[i];
            PtrArray *row = newPtrArray();
            
            ptrArrayAppend(newString(update->id_producto->text), row);
            ptrArrayAppend(newString(update->operacion ? "SUMAR" : "RESTAR"), row);
            
            char cantidadStr[12];
            snprintf(cantidadStr, sizeof(cantidadStr), "%d", update->cantidad);
            ptrArrayAppend(newString(cantidadStr), row);
            
            ptrArrayAppend(row, rows);
        }

        String *title = newString("Actualizaciones de Inventario Exitosas");
        showScrollableList(title, headings, rows, widths, 0);
        
        // Esperar confirmación
        String *continueMsg = newString("Presione Enter para continuar");
        showAlert(NULL, continueMsg, getmaxy(stdscr) - 2, 0);
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
    }

    // Mostrar errores si los hay
    if (errorUpdates && errorUpdates->len > 0) {
        PtrArray *headings = newPtrArray();
        ptrArrayAppend(newString("ID Producto"), headings);
        ptrArrayAppend(newString("Cantidad"), headings);
        ptrArrayAppend(newString("Error"), headings);

        IntArray *widths = newIntArray();
        intArrayAppend(15, widths);
        intArrayAppend(10, widths);
        intArrayAppend(40, widths);

        PtrArray *rows = newPtrArray();
        
        for (int i = 0; i < errorUpdates->len; i++) {
            InventoryError *error = (InventoryError *)errorUpdates->data[i];
            PtrArray *row = newPtrArray();
            
            ptrArrayAppend(newString(error->id_producto->text), row);
            
            char cantidadStr[12];
            snprintf(cantidadStr, sizeof(cantidadStr), "%d", error->cantidad);
            ptrArrayAppend(newString(cantidadStr), row);
            
            char *errorMsg;
            switch(error->error_code) {
                case 2: errorMsg = "Producto no encontrado"; break;
                case 3: errorMsg = "Stock insuficiente para restar"; break;
                case -1: errorMsg = "Error de sistema: No se pudo inicializar statement"; break;
                case -2: errorMsg = "Error de sistema: Fallo al preparar consulta"; break;
                case -3: errorMsg = "Error de sistema: Fallo al bindear parámetros"; break;
                case -4: errorMsg = "Error de sistema: Fallo al ejecutar procedimiento"; break;
                case -5: errorMsg = "Formato inválido: Campos faltantes en el archivo"; break;
                case -6: errorMsg = "Error de validación: Campos vacíos en el archivo"; break;
                case -7: errorMsg = "Error de validación: Cantidad no válida"; break;
                case -8: errorMsg = "Error de validación: Cantidad negativa no permitida"; break;
                case -9: errorMsg = "Error de validación: ID de producto no válido"; break;
                default: errorMsg = "Error desconocido"; break;
            }
            
            ptrArrayAppend(newString(errorMsg), row);
            ptrArrayAppend(row, rows);
        }

        String *title = newString("Errores en Actualización de Inventario");
        String *helpBar1 = newString("Use las flechas para navegar");
        String *helpBar2 = newString("Presione Enter para continuar");

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
        } while (keyPressed != '\n');

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
}

void loadInventoryFromFile(MYSQL *conn, String *filePath) {
    if (!conn || !filePath) return;

    PtrArray *csvLines = readCsv(filePath);
    if (!csvLines || csvLines->len == 0) {
        String *msg = newString("Error: Archivo vacío o formato inválido");
        showAlert(NULL, msg, 10, 1);
        deleteString(msg);
        if (csvLines) deletePtrArray(csvLines);
        return;
    }

    PtrArray *successfulUpdates = newPtrArray();
    PtrArray *errorUpdates = newPtrArray();
    int totalRecords = 0, failedRecords = 0;

    for (int i = 0; i < csvLines->len; i++) {
        PtrArray *line = (PtrArray *)csvLines->data[i];
        
        // Validación de formato (2 campos: ID y cantidad)
        if (line->len != 2) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newString("N/A");
            error->cantidad = 0;
            error->error_code = -5;
            ptrArrayAppend(error, errorUpdates);
            failedRecords++;
            continue;
        }

        String *id = (String *)line->data[0];
        String *cantidadStr = (String *)line->data[1];

        // Validación de campos no vacíos
        if (id->len <= 0 || cantidadStr->len <= 0) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len > 0 ? id->len : 1);
            error->cantidad = 0;
            error->error_code = -6;
            ptrArrayAppend(error, errorUpdates);
            failedRecords++;
            continue;
        }

        
        if (!isNumber(cantidadStr)) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len);
            error->cantidad = 0;
            error->error_code = -7;
            ptrArrayAppend(error, errorUpdates);
            failedRecords++;
            continue;
        }

        int cantidad = atoi(cantidadStr->text);

        // Preparar llamada al procedimiento almacenado
        MYSQL_STMT *stmt = mysql_stmt_init(conn);
        if (!stmt) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len);
            error->cantidad = cantidad;
            error->error_code = -1;
            ptrArrayAppend(error, errorUpdates);
            failedRecords++;
            continue;
        }

        const char *query = "CALL CargarInventario(?, ?, ?)";
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len);
            error->cantidad = cantidad;
            error->error_code = -2;
            ptrArrayAppend(error, errorUpdates);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        // Inicializar parámetros de forma segura
        MYSQL_BIND params[3];
        memset(params, 0, sizeof(params));

        // Parámetro 1: ID del producto
        unsigned long id_len = (unsigned long)id->len;
        params[0].buffer_type = MYSQL_TYPE_STRING;
        params[0].buffer = (char *)id->text;
        params[0].buffer_length = id_len;
        params[0].length = &id_len;

        // Parámetro 2: Cantidad (puede ser positiva o negativa)
        params[1].buffer_type = MYSQL_TYPE_LONG;
        params[1].buffer = &cantidad;
        params[1].is_unsigned = 0;
        params[1].is_null = 0;
        params[1].length = 0;

        // Parámetro 3: Resultado (salida)
        int resultado = 0;
        params[2].buffer_type = MYSQL_TYPE_LONG;
        params[2].buffer = &resultado;
        params[2].is_unsigned = 0;
        params[2].is_null = 0;

        if (mysql_stmt_bind_param(stmt, params)) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len);
            error->cantidad = cantidad;
            error->error_code = -3;
            ptrArrayAppend(error, errorUpdates);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        if (mysql_stmt_execute(stmt)) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len);
            error->cantidad = cantidad;
            error->error_code = -4;
            ptrArrayAppend(error, errorUpdates);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        // Vincular y recuperar el parámetro de salida
        MYSQL_BIND out_param;
        memset(&out_param, 0, sizeof(out_param));
        out_param.buffer_type = MYSQL_TYPE_LONG;
        out_param.buffer = &resultado;
        out_param.is_unsigned = 0;
        out_param.is_null = 0;

        if (mysql_stmt_bind_result(stmt, &out_param)) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len);
            error->cantidad = cantidad;
            error->error_code = -8;
            ptrArrayAppend(error, errorUpdates);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        if (mysql_stmt_fetch(stmt)) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len);
            error->cantidad = cantidad;
            error->error_code = -9;
            ptrArrayAppend(error, errorUpdates);
            mysql_stmt_close(stmt);
            failedRecords++;
            continue;
        }

        // Interpretar códigos de resultado
        String *errorMsg = NULL;
        switch(resultado) {
            case 1: // Éxito
                break;
            case 2:
                errorMsg = newString("Producto no encontrado");
                break;
            case 3:
                errorMsg = newString("Stock insuficiente para restar");
                break;
            default:
                errorMsg = newString("Error desconocido");
        }

        if (resultado != 1) {
            InventoryError *error = (InventoryError *)malloc(sizeof(InventoryError));
            error->id_producto = newStringN(id->text, id->len);
            error->cantidad = cantidad;
            error->error_code = resultado;
            ptrArrayAppend(error, errorUpdates);
            failedRecords++;
        } else {
            InventoryUpdate *update = (InventoryUpdate *)malloc(sizeof(InventoryUpdate));
            if (update) {
                update->id_producto = newStringN(id->text, id->len);
                update->cantidad = cantidad;
                ptrArrayAppend(update, successfulUpdates);
            }
            if (errorMsg) deleteString(errorMsg);
        }

        mysql_stmt_close(stmt);
        totalRecords++;
    }

    // Mostrar resumen
    char summaryText[150];
    if (failedRecords > 0) {
        snprintf(summaryText, sizeof(summaryText), 
                "Resumen: %d actualizaciones procesadas, %d fallidas",
                totalRecords - failedRecords, failedRecords);
    } else {
        snprintf(summaryText, sizeof(summaryText), 
                "Resumen: Todas las %d actualizaciones se procesaron exitosamente",
                totalRecords);
    }

    String *summary = newString(summaryText);
    showAlert(NULL, summary, 10, failedRecords > 0 ? 1 : 0);
    deleteString(summary);

    // Mostrar resultados
    showInventoryUpdateResults(successfulUpdates, errorUpdates);

    // Liberar memoria
    for (int i = 0; i < successfulUpdates->len; i++) {
        freeInventoryUpdate(successfulUpdates->data[i]);
    }
    deletePtrArray(successfulUpdates);

    for (int i = 0; i < errorUpdates->len; i++) {
        freeInventoryError(errorUpdates->data[i]);
    }
    deletePtrArray(errorUpdates);

    for (int i = 0; i < csvLines->len; i++) {
        PtrArray *line = csvLines->data[i];
        deleteStringArray(line);
    }
    deletePtrArray(csvLines);
}

void LoadInventory(MYSQL *conn) {
    clear();
    
    String *filePrompt = newString("Ingrese la ruta del archivo para cargar inventario:");
    String *filePath = showInput(filePrompt, 10, 0);
    deleteString(filePrompt);
    
    if (filePath) {
        loadInventoryFromFile(conn, filePath);
        deleteString(filePath);
    }
}

void registerProductFamily(MYSQL *conn) {
    clear();
    String *filePrompt = newString("Ingrese la ruta del archivo de familias:");
    String *filePath = readStringFromUI(filePrompt, 9);
    deleteString(filePrompt);

    if (!filePath) {
        registerProductFamily(conn);
        return;
    }

    PtrArray *families = newPtrArray();
    loadFamiliesFromFile(conn, filePath, families);
    deleteString(filePath);
    deletePtrArray(families);
}

void registerProduct(MYSQL *conn) {
    clear();
    String *filePrompt = newString("Ingrese la ruta del archivo de productos:");
    String *filePath = readStringFromUI(filePrompt, 17);
    deleteString(filePrompt);

    if (!filePath) {
        registerProduct(conn);
        return;
    }
    
    PtrArray *products = newPtrArray();
    loadProductsFromFile(conn, filePath, products);
    deleteString(filePath);
    deletePtrArray(products);
}


int showProductsTable(MYSQL *conn, int row) {
    // Consultar todos los productos
    const char *query = "SELECT p.id_producto, p.descripcion, f.descripcion, p.costo, p.precio, p.stock "
                        "FROM Producto p JOIN Familia f ON p.id_familia = f.id_familia "
                        "ORDER BY p.id_producto";
    
    if (mysql_query(conn, query)) {
        String *errorMsg = newString("Error al consultar productos");
        showAlert(NULL, errorMsg, row, 1);
        deleteString(errorMsg);
        return 0;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        String *errorMsg = newString("Error al obtener resultados");
        showAlert(NULL, errorMsg, row, 1);
        deleteString(errorMsg);
        return 0;
    }

    // Preparar encabezados
    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("ID"), headings);
    ptrArrayAppend(newString("Descripción"), headings);
    ptrArrayAppend(newString("Familia"), headings);
    ptrArrayAppend(newString("Costo"), headings);
    ptrArrayAppend(newString("Precio"), headings);
    ptrArrayAppend(newString("Stock"), headings);

    // Anchos de columnas
    IntArray *widths = newIntArray();
    intArrayAppend(10, widths);   // ID
    intArrayAppend(25, widths);   // Descripción
    intArrayAppend(15, widths);   // Familia
    intArrayAppend(10, widths);   // Costo
    intArrayAppend(10, widths);   // Precio
    intArrayAppend(8, widths);    // Stock

    // Preparar filas de datos
    PtrArray *rows = newPtrArray();
    MYSQL_ROW mysql_row;
    
    while ((mysql_row = mysql_fetch_row(result))) {
        PtrArray *row = newPtrArray();
        
        // ID
        ptrArrayAppend(newString(mysql_row[0]), row);
        
        // Descripción (limitada a 25 caracteres)
        char desc[26];
        snprintf(desc, sizeof(desc), "%.25s", mysql_row[1]);
        ptrArrayAppend(newString(desc), row);
        
        // Familia (limitada a 15 caracteres)
        char family[16];
        snprintf(family, sizeof(family), "%.15s", mysql_row[2]);
        ptrArrayAppend(newString(family), row);
        
        // Costo (formateado a 2 decimales)
        char costStr[12];
        snprintf(costStr, sizeof(costStr), "%.2f", atof(mysql_row[3]));
        ptrArrayAppend(newString(costStr), row);
        
        // Precio (formateado a 2 decimales)
        char priceStr[12];
        snprintf(priceStr, sizeof(priceStr), "%.2f", atof(mysql_row[4]));
        ptrArrayAppend(newString(priceStr), row);
        
        // Stock
        ptrArrayAppend(newString(mysql_row[5]), row);
        
        ptrArrayAppend(row, rows);
    }
    mysql_free_result(result);

    if (rows->len==0){
        return 0;
    }

    // Mostrar la tabla
    String *title = newString("Listado de Productos");
    showScrollableList(title, headings, rows, widths, 0);
    
    // Liberar memoria
    deleteString(title);
    deleteStringArray(headings);
    deleteIntArray(widths);
    
    for (int i = 0; i < rows->len; i++) {
        PtrArray *row = rows->data[i];
        deleteStringArray(row);
    }
    deletePtrArray(rows);
    return 1;
}

// Función principal para eliminar producto
void deleteProduct(MYSQL *conn) {
    clear();
    
    // Mostrar tabla de productos en la parte superior
    int opt=showProductsTable(conn, 0);
    
    if (opt == 0){
        String *msg= newString("No hay productos disponibles en la base de datos");
        showAlert(NULL,msg, 3, 0);
        deleteString(msg);
        return;
    }
    // Calcular posición segura para la entrada
    int maxy = getmaxy(stdscr);
    int input_row = (maxy > 3) ? (maxy - 3) : 1;
    
    // Pedir ID del producto a eliminar
    String *prompt = newString("Ingrese ID del producto a eliminar:");
    if (!prompt){
        return;
    }
    
    String *productId = showInput(prompt, input_row, 0);
    deleteString(prompt);
    
    if (!productId || productId->len == 0) {
        if (productId) deleteString(productId);
        return;
    }

    // Validar longitud del ID antes de continuar
    if (productId->len > 10) { // Asumiendo que los IDs no exceden 10 caracteres
        String *errorMsg = newString("Error: ID demasiado largo (max 10 chars)");
        showAlert(NULL, errorMsg, input_row, 1);
        deleteString(errorMsg);
        deleteString(productId);
        return;
    }

    // Confirmar eliminación en posición segura
    int confirm_row = (maxy > 1) ? (maxy - 2) : 1;
    char confirmMsg[100]={0};
    snprintf(confirmMsg, sizeof(confirmMsg), "¿Eliminar producto %.*s? (S/N)", productId->len, productId->text);
    
    String *confirmPrompt = newString(confirmMsg);
    if (!confirmPrompt) {
        deleteString(productId);
        return;
    }
    
    String *confirm = showInput(confirmPrompt, confirm_row, 0);
    deleteString(confirmPrompt);

    if (!confirm || (confirm->text[0] != 'S' && confirm->text[0] != 's')) {
        deleteString(productId);
        if (confirm) deleteString(confirm);
        return;
    }
    deleteString(confirm);

    // Llamar al procedimiento almacenado con manejo seguro
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        String *errorMsg = newString("Error al inicializar statement");
        showAlert(NULL, errorMsg, confirm_row, 1);
        deleteString(errorMsg);
        deleteString(productId);
        return;
    }

    const char *query = "CALL EliminarProducto(?, ?)";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        String *errorMsg = newString("Error al preparar statement");
        showAlert(NULL, errorMsg, confirm_row, 1);
        deleteString(errorMsg);
        mysql_stmt_close(stmt);
        deleteString(productId);
        return;
    }

    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    int resultado = 0;

    // Parámetro 1: ID del producto (con verificación de longitud)
    char id_buffer[11] = {0}; // 10 chars + null terminator
    strncpy(id_buffer, productId->text, productId->len);


    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = id_buffer;
    params[0].buffer_length = strlen(id_buffer);

    // Parámetro 2: Resultado (salida)
    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &resultado;
    params[1].is_unsigned = 0;
    params[1].is_null = 0;

    if (mysql_stmt_bind_param(stmt, params)) {
        String *errorMsg = newString("Error al bindear parámetros");
        showAlert(NULL, errorMsg, confirm_row, 1);
        deleteString(errorMsg);
        mysql_stmt_close(stmt);
        deleteString(productId);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        String *errorMsg = newString("Error al ejecutar procedimiento");
        showAlert(NULL, errorMsg, confirm_row, 1);
        deleteString(errorMsg);
        mysql_stmt_close(stmt);
        deleteString(productId);
        return;
    }

    // Vincular y recuperar el parámetro de salida
    MYSQL_BIND out_param;
    memset(&out_param, 0, sizeof(out_param));
    out_param.buffer_type = MYSQL_TYPE_LONG;
    out_param.buffer = &resultado;
    out_param.is_unsigned = 0;
    out_param.is_null = 0;

    if (mysql_stmt_bind_result(stmt, &out_param)) {
        String *errorMsg = newString("Error al vincular parámetro de salida");
        showAlert(NULL, errorMsg, confirm_row, 1);
        deleteString(errorMsg);
        mysql_stmt_close(stmt);
        deleteString(productId);
        return;
    }

    if (mysql_stmt_fetch(stmt)) {
        String *errorMsg = newString("Error al obtener resultado");
        showAlert(NULL, errorMsg, confirm_row, 1);
        deleteString(errorMsg);
        mysql_stmt_close(stmt);
        deleteString(productId);
        return;
    }

    mysql_stmt_close(stmt);

    // Mostrar resultado con manejo seguro de posición
    String *resultMsg = NULL;
    int isError = 0;
    switch(resultado) {
        case 1:
            resultMsg = newString("Producto eliminado exitosamente");
            isError = 0;
            break;
        case 2:
            resultMsg = newString("Error: El producto no existe");
            isError = 1;
            break;
        case 3:
            resultMsg = newString("Error: Producto está en cotizaciones/facturas");
            isError = 1;
            break;
        default:
            resultMsg = newString("Error desconocido al eliminar producto");
            isError = 1;
            break;
    }

    if (resultMsg) {
        showAlert(newString("Resultado"), resultMsg, (maxy > 3) ? (maxy - 3) : 1, isError);
        deleteString(resultMsg);
    }
    
    deleteString(productId);
    
    // Volver a mostrar la tabla actualizada con manejo seguro
    clear();
    showProductsTable(conn, 0);
    
    String *continueMsg = newString("Presione cualquier tecla para continuar");
    if (continueMsg) {
        showAlert(NULL, continueMsg, (maxy > 1) ? (maxy - 1) : 1, 0);
        deleteString(continueMsg);
    }
}

#endif
