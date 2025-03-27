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


void loadFamiliesFromFile(MYSQL *conn, String *filePath, PtrArray *families)
{
    if (!conn || !filePath || !families)
        return;
    
    
    File *file = readFileStr(filePath);
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

void loadProductsFromFile(MYSQL *conn, String *filePath, PtrArray *products)
{
    if (!conn || !filePath || !products)
        return;

    File *file = readFileStr(filePath);
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

    PtrArray *families = newPtrArray();
    loadFamiliesFromFile(conn, filePath, families);

    deleteString(filePath);
    deletePtrArray(families);
}

void registerProduct(MYSQL *conn)
{
    clear();

    String *filePrompt = newString("Ingrese la ruta del archivo de productos:");
    String *filePath = readStringFromUI(filePrompt, 17);
    deleteString(filePrompt);

    if (!filePath)
    {
        registerProduct(conn);
        return;
    }
    PtrArray *products = newPtrArray();

    loadProductsFromFile(conn, filePath, products);

    deleteString(filePath);
    
    deletePtrArray(products);
    
}