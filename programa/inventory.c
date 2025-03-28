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
        showInput(msg, 10, 0);
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
            case 3: errorMsg = "Costo/precio inválido"; break;
            case 4: errorMsg = "Precio menor que costo"; break;
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

void loadFamiliesFromFile(MYSQL *conn, String *filePath, PtrArray *families) {
    if (!conn || !filePath || !families) return;
    
    PtrArray *csvLines = readCsv(filePath);
    if (!csvLines || csvLines->len == 0) {
        String *msg = newString("Error: Archivo vacío o formato inválido");
        showInput(msg, 10, 1);
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

        if (resultado != 0) {  // 0 es éxito según tu procedimiento
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            printw("Esta es la longitud %i y esta es la cadena %.*s", id->len, id->len , id->text);
            getch();
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
                ptrArrayAppend(family, families);
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
        showInput(continueMsg, getmaxy(stdscr) - 2, 0);
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
    showInput(summary, 10, failedRecords > 0 ? 1 : 0);
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
        showInput(msg, 10, 1);
        deleteString(msg);
        if (csvLines) deletePtrArray(csvLines);
        return;
    }

    PtrArray *errors = newPtrArray();
    int totalRecords = 0, failedRecords = 0;

    for (int i = 0; i < csvLines->len; i++) {
        PtrArray *line = (PtrArray *)csvLines->data[i];
        
        // Validar que la línea tenga exactamente 6 campos
        if (line->len != 6) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString("N/A");
            error->description = newString("Formato inválido: Se esperaban 6 campos");
            error->error_code = -5;
            ptrArrayAppend(error, errors);
            failedRecords++;
            continue;
        }

        String *id = (String *)line->data[0];
        String *description = (String *)line->data[1];
        String *family_desc = (String *)line->data[2];
        String *costStr = (String *)line->data[3];
        String *priceStr = (String *)line->data[4];
        String *stockStr = (String *)line->data[5];

        float cost, price;
        int stock;
        
        if (!isNumber(costStr) || !isNumber(priceStr) || !isNumber(stockStr)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString(id->text);
            error->description = newString("Valores numéricos inválidos");
            error->error_code = -7;
            ptrArrayAppend(error, errors);
            failedRecords++;
            continue;
        }

        cost = atof(costStr->text);
        price = atof(priceStr->text);
        stock = atoi(stockStr->text);

        Product *product = (Product *)malloc(sizeof(Product));
        if (!product) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString(id->text);
            error->description = newString("Error asignando memoria");
            error->error_code = -9;
            ptrArrayAppend(error, errors);
            failedRecords++;
            continue;
        }

        product->id = newString(id->text);
        product->description = newString(description->text);
        product->family_desc = newString(family_desc->text);
        product->cost = cost;
        product->price = price;
        product->stock = stock;

        MYSQL_STMT *stmt;
        MYSQL_BIND params[7];
        int resultado = 0;

        stmt = mysql_stmt_init(conn);
        if (!stmt) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString(id->text);
            error->description = newString("Error al inicializar statement");
            error->error_code = -10;
            ptrArrayAppend(error, errors);
            freeProduct(product);
            failedRecords++;
            continue;
        }

        const char *query = "CALL RegistrarProducto(?, ?, ?, ?, ?, ?, ?)";
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString(id->text);
            error->description = newString("Error preparando statement");
            error->error_code = -11;
            ptrArrayAppend(error, errors);
            mysql_stmt_close(stmt);
            freeProduct(product);
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
            error->id = newString(id->text);
            error->description = newString("Error bindeando parámetros");
            error->error_code = -12;
            ptrArrayAppend(error, errors);
            mysql_stmt_close(stmt);
            freeProduct(product);
            failedRecords++;
            continue;
        }

        if (mysql_stmt_execute(stmt)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString(id->text);
            error->description = newString("Error ejecutando procedimiento");
            error->error_code = -13;
            ptrArrayAppend(error, errors);
            mysql_stmt_close(stmt);
            freeProduct(product);
            failedRecords++;
            continue;
        }

        // Paso crítico: Vincular y recuperar el parámetro de salida
        MYSQL_BIND out_param;
        memset(&out_param, 0, sizeof(out_param));
        out_param.buffer_type = MYSQL_TYPE_LONG;
        out_param.buffer = &resultado;
        out_param.is_unsigned = 0;
        out_param.is_null = 0;

        if (mysql_stmt_bind_result(stmt, &out_param)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString(id->text);
            error->description = newString("Error al vincular parámetro de salida");
            error->error_code = -15;
            ptrArrayAppend(error, errors);
            mysql_stmt_close(stmt);
            freeProduct(product);
            failedRecords++;
            continue;
        }

        if (mysql_stmt_fetch(stmt)) {
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString(id->text);
            error->description = newString("Error al obtener resultado");
            error->error_code = -16;
            ptrArrayAppend(error, errors);
            mysql_stmt_close(stmt);
            freeProduct(product);
            failedRecords++;
            continue;
        }

        // Ahora resultado debería tener el valor correcto
        printw("Resultado del procedimiento: %d\n", resultado);
        refresh();
        getch();

        if (resultado != 5) {  // 5 es éxito según tu procedimiento
            ProcessingError *error = (ProcessingError *)malloc(sizeof(ProcessingError));
            error->id = newString(id->text);
            error->description = newString(description->text);
            error->error_code = resultado;
            ptrArrayAppend(error, errors);
            freeProduct(product);
            failedRecords++;
        } else {
            ptrArrayAppend(product, products);
        }

        mysql_stmt_close(stmt);
        totalRecords++;
    }
    
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
    showInput(summary, 10, failedRecords > 0 ? 1 : 0);
    deleteString(summary);

    if (failedRecords > 0) {
        showProcessingErrors(errors);
    }

    for (int i = 0; i < errors->len; i++) {
        freeProcessingError(errors->data[i]);
    }
    deletePtrArray(errors);

    for (int i = 0; i < csvLines->len; i++) {
        PtrArray *line = csvLines->data[i];
        deleteStringArray(line);
    }
    deletePtrArray(csvLines);
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