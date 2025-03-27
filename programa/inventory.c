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
typedef struct
{
    int id;
    String *description;
} Family;

typedef struct
{
    int id;
    String *description;
    int family_id;
    float cost;
    float price;
    int stock;
} Product;

// Función para liberar memoria de familia
void freeFamily(void *family)
{
    if (!family)
        return;
    Family *f = (Family *)family;
    if (f->description)
        deleteString(f->description);
    free(f);
}

// Función para liberar memoria de producto
void freeProduct(void *product)
{
    if (!product)
        return;
    Product *p = (Product *)product;
    if (p->description)
        deleteString(p->description);
    free(p);
}

String *readStringFromUI(String *prompt, int row)
{
    if (!prompt)
        return NULL;

    String *input = showInput(prompt, row, 0);

    return input;
}

int showFamilyInputDialog(int *id, String **description)
{
    PtrArray *inputs = newPtrArray();
    if (!inputs)
        return 0;

    // Crear campos de entrada
    String *idPrompt = newString("Ingrese ID de familia:");
    String *descPrompt = newString("Ingrese descripción de familia:");

    if (!idPrompt || !descPrompt)
    {
        if (idPrompt)
            deleteString(idPrompt);
        if (descPrompt)
            deleteString(descPrompt);
        deletePtrArray(inputs);
        return 0;
    }

    ptrArrayAppend(idPrompt, inputs);
    ptrArrayAppend(descPrompt, inputs);

    // Mostrar diálogo de entrada
    String *formId= newString("Ingrese ID de familia:");
    String *idStr = readStringFromUI(formId, 5);
    if (!idStr || !isNumber(idStr))
    {
        deleteString(idStr);
        deletePtrArray(inputs);
        return 0;
    }
    deleteString(formId);

    String *formDescr= newString("Ingrese descripción de familia:");
    String *descStr = readStringFromUI(formDescr, 7);
    if (!descStr)
    {
        deleteString(idStr);
        deleteString(descStr);
        deletePtrArray(inputs);
        return 0;
    }
    deleteString(formDescr);

    *id = toInt(idStr);
    *description = descStr;

    deleteString(idStr);
    deletePtrArray(inputs);
    return 1;
}
/*
int showProductInputDialog(Product *product)
{
    PtrArray *inputs = newPtrArray();
    if (!inputs)
        return 0;

    // Crear prompts
    String *prompts[] = {
        newString("Ingrese ID de producto:"),
        newString("Ingrese descripción de producto:"),
        newString("Ingrese ID de familia:"),
        newString("Ingrese costo:"),
        newString("Ingrese precio:"),
        newString("Ingrese stock:")};
    
    // Obtener valores de entrada
    int startRow = 5;
    String *inputValues[6];

    for (int i = 0; i < 6; i++)
    {
        inputValues[i] = readStringFromUI(prompts[i], startRow + i * 2);
        if (!inputValues[i])
        {
            for (int j = 0; j < i; j++)
                deleteString(inputValues[j]);
            deletePtrArray(inputs);
            return 0;
        }
    }

    // Validar entradas numéricas
    if (!isNumber(inputValues[0]))
        goto cleanup;
    if (!isNumber(inputValues[2]))
        goto cleanup;
    if (!isNumber(inputValues[3]))
        goto cleanup;
    if (!isNumber(inputValues[4]))
        goto cleanup;
    if (!isNumber(inputValues[5]))
        goto cleanup;

    product->id = toInt(inputValues[0]);
    product->description = inputValues[1];
    product->family_id = toInt(inputValues[2]);
    product->cost = (float)atof(inputValues[3]->text);
    product->price = (float)atof(inputValues[4]->text);
    product->stock = toInt(inputValues[5]);

    // Limpiar strings temporales
    deleteString(inputValues[0]);
    deleteString(inputValues[2]);
    deleteString(inputValues[3]);
    deleteString(inputValues[4]);
    deleteString(inputValues[5]);

    deletePtrArray(inputs);
    return 1;

cleanup:
    for (int i = 0; i < 6; i++)
        deleteString(inputValues[i]);
    deletePtrArray(inputs);
    return 0;
}
*/
int addFamilyToFile(const char *filePath, PtrArray *families, int id, String *description)
{
    if (!filePath || !families || !description)
        return 0;

    Family *family = (Family *)malloc(sizeof(Family));
    if (!family)
        return 0;

    family->id = id;
    family->description = description;

    ptrArrayAppend(family, families);

    FILE *file = fopen(filePath, "a");
    if (!file)
    {
        printw("Error: No se pudo abrir el archivo de familias.\n");
        return 0;
    }

    fprintf(file, "%d,%.*s\n", id, description->len, description->text);
    fclose(file);
    return 1;
}

int addProductToFile(const char *filePath, PtrArray *products, int id, String *description,
                     int family_id, float cost, float price, int stock)
{
    if (!filePath || !products || !description)
        return 0;

    Product *product = (Product *)malloc(sizeof(Product));
    if (!product)
        return 0;

    product->id = id;
    product->description = description;
    product->family_id = family_id;
    product->cost = cost;
    product->price = price;
    product->stock = stock;

    ptrArrayAppend(product, products);

    FILE *file = fopen(filePath, "a");
    if (!file)
    {
        printw("Error: No se pudo abrir el archivo de productos.\n");
        return 0;
    }

    fprintf(file, "%d,%.*s,%d,%.2f,%.2f,%d\n",
            id, description->len, description->text,
            family_id, cost, price, stock);
    fclose(file);
    return 1;
}

void loadFamiliesFromFile(MYSQL *conn, char *filePath, PtrArray *families)
{
    if (!conn || !filePath || !families)
        return;

    File *file = readFile(filePath);
    if (!file || file->len == 0)
    {
        printw("Error: No se pudo leer el archivo o está vacío.\n");
        if (file)
            freeFile(file);
        return;
    }

    int totalRecords = 0, failedRecords = 0;
    char *content = file->content;
    char *line = content;
    char *end = content + file->len;

    printw("Registros fallidos:\n");

    while (line < end)
    {
        char *nextLine = strchr(line, '\n');
        if (nextLine)
            *nextLine = '\0';

        char *idStr = line;
        char *desc = strchr(line, ',');
        if (!desc)
        {
            line = nextLine ? nextLine + 1 : end;
            continue;
        }
        *desc = '\0';
        desc++;

        int id = atoi(idStr);
        String *description = newString(desc);
        if (!description)
        {
            line = nextLine ? nextLine + 1 : end;
            continue;
        }

        Family *family = (Family *)malloc(sizeof(Family));
        if (!family)
        {
            deleteString(description);
            line = nextLine ? nextLine + 1 : end;
            continue;
        }

        family->id = id;
        family->description = description;

        ptrArrayAppend(family, families);
        char query[MAX_QUERY_LENGTH];
        snprintf(query, sizeof(query),
                 "INSERT INTO Family (id_family, description) VALUES (%d, '%.*s')",
                 id, description->len, description->text);

        if (mysql_query(conn, query))
        {
            printw("ID: %d, Descripción: %.*s\n", id, description->len, description->text);
            failedRecords++;
        }

        totalRecords++;
        line = nextLine ? nextLine + 1 : end;
    }

    freeFile(file);

    printw("\nTotal de registros: %d\n", totalRecords);
    printw("Registros fallidos: %d\n", failedRecords);
    refresh();
    getch();
    return;
}

void loadProductsFromFile(MYSQL *conn, char *filePath, PtrArray *products)
{
    if (!conn || !filePath || !products)
        return;

    File *file = readFile(filePath);
    if (!file || file->len == 0)
    {
        printw("Error: No se pudo leer el archivo o está vacío.\n");
        if (file)
            freeFile(file);
        return;
    }

    int totalRecords = 0, failedRecords = 0;
    char *content = file->content;
    char *line = content;
    char *end = content + file->len;

    printw("Registros fallidos:\n");

    while (line < end)
    {
        char *nextLine = strchr(line, '\n');
        if (nextLine)
            *nextLine = '\0';

        char *idStr = line;
        char *desc = strchr(line, ',');
        if (!desc)
        {
            line = nextLine ? nextLine + 1 : end;
            continue;
        }
        *desc = '\0';
        desc++;

        int id = atoi(idStr);

        char *familyStr = strchr(desc, ',');
        if (!familyStr)
        {
            line = nextLine ? nextLine + 1 : end;
            continue;
        }
        *familyStr = '\0';
        familyStr++;

        int family_id;
        float cost, price;
        int stock;
        if (sscanf(familyStr, "%d,%f,%f,%d", &family_id, &cost, &price, &stock) != 4)
        {
            line = nextLine ? nextLine + 1 : end;
            continue;
        }

        String *description = newString(desc);
        if (!description)
        {
            line = nextLine ? nextLine + 1 : end;
            continue;
        }

        Product *product = (Product *)malloc(sizeof(Product));
        if (!product)
        {
            deleteString(description);
            line = nextLine ? nextLine + 1 : end;
            continue;
        }

        product->id = id;
        product->description = description;
        product->family_id = family_id;
        product->cost = cost;
        product->price = price;
        product->stock = stock;

        ptrArrayAppend(product, products);

        char query[MAX_QUERY_LENGTH];
        snprintf(query, sizeof(query),
                 "INSERT INTO Product (id, description, stock, cost, price, id_family) "
                 "VALUES (%d, '%.*s', %d, %f, %f, %d)",
                 id, description->len, description->text, stock, cost, price, family_id);

        if (mysql_query(conn, query))
        {
            printw("ID: %d, Nombre: %.*s\n", id, description->len, description->text);
            failedRecords++;
        }

        totalRecords++;
        line = nextLine ? nextLine + 1 : end;
    }

    freeFile(file);

    printw("\nTotal de registros: %d\n", totalRecords);
    printw("Registros fallidos: %d\n", failedRecords);
    refresh();
    getch();
    return;
}

int loadFamiliesFromDB(MYSQL *conn, PtrArray *families)
{
    if (!conn || !families)
        return 0;

    if (mysql_query(conn, "SELECT id_family, description FROM Family"))
    {
        return 0;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result)
        return 0;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        int id = atoi(row[0]);
        String *desc = newString(row[1]);
        if (!desc)
            continue;

        Family *family = (Family *)malloc(sizeof(Family));
        if (!family)
        {
            deleteString(desc);
            continue;
        }

        family->id = id;
        family->description = desc;

        ptrArrayAppend(family, families);
    }

    mysql_free_result(result);
    return 1;
}

int showFamilySelection(MYSQL *conn, int *selectedId)
{
    PtrArray *families = newPtrArray();
    if (!families)
        return 0;

    if (!loadFamiliesFromDB(conn, families))
    {
        deletePtrArray(families);
        return 0;
    }

    // Preparar elementos de UI
    PtrArray *headings = newPtrArray();
    ptrArrayAppend(newString("ID"), headings);
    ptrArrayAppend(newString("Descripción"), headings);

    IntArray *widths = newIntArray();
    intArrayAppend(10, widths);
    intArrayAppend(40, widths);

    PtrArray *rows = newPtrArray();
    for (int i = 0; i < families->len; i++)
    {
        Family *f = (Family *)families->data[i];
        PtrArray *row = newPtrArray();
        char idStr[20];
        snprintf(idStr, sizeof(idStr), "%d", f->id);
        ptrArrayAppend(newString(idStr), row);
        ptrArrayAppend(newString(f->description->text), row);
        ptrArrayAppend(row, rows);
    }

    String *title = newString("Seleccionar Familia");
    int initialRow = 0;
    int selection = -1;

    while (1)
    {
        int visibleRows = showScrollableList(title, headings, rows, widths, initialRow);
        int key = getch();

        if (key == KEY_UP && initialRow > 0)
        {
            initialRow--;
        }
        else if (key == KEY_DOWN && initialRow + visibleRows < rows->len)
        {
            initialRow++;
        }
        else if (key == '\n')
        {
            selection = initialRow;
            break;
        }
        else if (key == 27)
        { // ESC
            break;
        }
    }

    if (selection >= 0 && selection < families->len)
    {
        Family *f = (Family *)families->data[selection];
        *selectedId = f->id;
    }

    // Limpiar
    deleteString(title);
    deleteStringArray(headings);
    deleteIntArray(widths);
    for (int i = 0; i < rows->len; i++)
    {
        deleteStringArray(rows->data[i]);
    }
    deletePtrArray(rows);
    deletePtrArray(families);

    return selection != -1;
}

void registerProductFamily(MYSQL *conn)
{
    clear();

    String *filePrompt = newString("Ingrese la ruta del archivo de familias:");
    String *filePath = readStringFromUI(filePrompt, 9);
    deleteString(filePrompt);

    if (!filePath)
    {
        registerProductFamily(conn);
        return;
    }

    int id;
    String *description = NULL;

    if (!showFamilyInputDialog(&id, &description))
    {
        return;
    }

    PtrArray *families = newPtrArray();
    if (!families)
    {
        deleteString(description);
        return;
    }

    if (!addFamilyToFile(filePath->text, families, id, description))
    {
        deleteString(filePath);
        deleteString(description);
        deletePtrArray(families);
        return;
    }

    loadFamiliesFromFile(conn, filePath->text, families);

    deleteString(filePath);
    deletePtrArray(families);
}

void registerProduct()//MYSQL *conn)
{
    /*
    Product product = {0};
    
    if (!showProductInputDialog(&product))
    {
        return;
    }

    PtrArray *products = newPtrArray();
    if (!products)
    {
    adm
        freeProduct(&product);
        return;
    }

    String *filePrompt = newString("Ingrese la ruta del archivo de productos:");
    String *filePath = readStringFromUI(filePrompt->text, 17);
    deleteString(filePrompt);

    if (!filePath)
    {
        freeProduct(&product);
        deletePtrArray(products);
        return;
    }

    if (!addProductToFile(filePath->text, products, product.id, product.description,
                          product.family_id, product.cost, product.price, product.stock))
    {
        deleteString(filePath);
        freeProduct(&product);
        deletePtrArray(products);
        return;
    }

    loadProductsFromFile(conn, filePath->text, products);

    deleteString(filePath);
    deletePtrArray(products);
    */
}