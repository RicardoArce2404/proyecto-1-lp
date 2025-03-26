#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>
#include <ncurses.h>
#include "array.c"  // Incluir el archivo array.c
#include "string.c" // Incluir el archivo string.c

// Definición de structs
typedef struct {
    int id;
    String *descripcion; // Usar String en lugar de char[]
} Familia;

typedef struct {
    int id;
    String *descripcion; // Usar String en lugar de char[]
    int id_familia;
    float costo;
    float precio;
    int stock;
} Producto;

// Función para leer una cadena dinámica desde la entrada del usuario
String *readStringFromInput() {
    char buffer[1024]; // Buffer temporal para leer la entrada
    echo(); // Habilitar la visualización de la entrada
    scanw("%1023[^\n]", buffer); // Leer hasta 1023 caracteres (evita desbordamiento)
    noecho(); // Deshabilitar la visualización de la entrada
    return newString(buffer); // Crear un String dinámico a partir del buffer
}

// Función para agregar una familia al archivo y al array dinámico
void addFamilyToFile(const char *filePath, PtrArray *familias, int id, String *descripcion) {
    // Crear una nueva familia
    Familia *familia = (Familia *)malloc(sizeof(Familia));
    familia->id = id;
    familia->descripcion = descripcion;

    // Agregar la familia al array dinámico
    ptrArrayAppend(familia, familias);

    // Agregar la familia al archivo
    FILE *file = fopen(filePath, "a"); // Abrir en modo append (agregar al final)
    if (!file) {
        printw("Error: No se pudo abrir el archivo de familias.\n");
        return;
    }

    fprintf(file, "%d,%.*s\n", id, descripcion->len, descripcion->text); // Agregar la nueva familia
    fclose(file);

    printw("Familia agregada al archivo: %s\n", filePath);
}

// Función para agregar un producto al archivo y al array dinámico
void addProductToFile(const char *filePath, PtrArray *productos, int id, String *descripcion, int id_familia, float costo, float precio, int stock) {
    // Crear un nuevo producto
    Producto *producto = (Producto *)malloc(sizeof(Producto));
    producto->id = id;
    producto->descripcion = descripcion;
    producto->id_familia = id_familia;
    producto->costo = costo;
    producto->precio = precio;
    producto->stock = stock;

    // Agregar el producto al array dinámico
    ptrArrayAppend(producto, productos);

    // Agregar el producto al archivo
    FILE *file = fopen(filePath, "a"); // Abrir en modo append (agregar al final)
    if (!file) {
        printw("Error: No se pudo abrir el archivo de productos.\n");
        return;
    }

    fprintf(file, "%d,%.*s,%d,%.2f,%.2f,%d\n", id, descripcion->len, descripcion->text, id_familia, costo, precio, stock); // Agregar el nuevo producto
    fclose(file);

    printw("Producto agregado al archivo: %s\n", filePath);
}

// Función para cargar familias desde un archivo al array dinámico y luego a la base de datos
void loadFamiliesFromFile(MYSQL *conn, const char *filePath, PtrArray *familias) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printw("Error: No se pudo abrir el archivo.\n");
        refresh();
        getch();
        return;
    }

    char line[1024]; // Buffer temporal para leer cada línea del archivo
    int totalRecords = 0, failedRecords = 0;

    printw("Registros no insertados:\n");
    while (fgets(line, sizeof(line), file)) {
        int id;
        char *descripcion = strchr(line, ','); // Buscar la coma que separa el ID de la descripción
        if (!descripcion) {
            printw("Error: Formato de archivo incorrecto.\n");
            fclose(file);
            refresh();
            getch();
            return;
        }
        *descripcion = '\0'; // Separar el ID de la descripción
        descripcion++; // Mover el puntero al inicio de la descripción

        id = atoi(line); // Convertir el ID a entero
        String *descripcionStr = newString(descripcion); // Crear un String dinámico para la descripción

        totalRecords++;

        // Crear una nueva familia
        Familia *familia = (Familia *)malloc(sizeof(Familia));
        familia->id = id;
        familia->descripcion = descripcionStr;

        // Agregar la familia al array dinámico
        ptrArrayAppend(familia, familias);

        // Insertar la familia en la base de datos
        char query[256];
        snprintf(query, sizeof(query), "INSERT INTO Familia (id_familia, descripcion) VALUES (%d, '%.*s')", id, descripcionStr->len, descripcionStr->text);

        if (mysql_query(conn, query)) {
            // Mostrar el registro no insertado en la consola
            printw("ID: %d, Descripción: %.*s\n", id, descripcionStr->len, descripcionStr->text);
            failedRecords++;
        }
    }

    fclose(file);

    // Mostrar resumen
    printw("\nTotal de registros: %d\n", totalRecords);
    printw("\nRegistros fallidos: %d\n", failedRecords);
    refresh();
    getch();
}

// Función para cargar productos desde un archivo al array dinámico y luego a la base de datos
void loadProductsFromFile(MYSQL *conn, const char *filePath, PtrArray *productos) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        printw("Error: No se pudo abrir el archivo.\n");
        refresh();
        getch();
        return;
    }

    char line[1024]; // Buffer temporal para leer cada línea del archivo
    int totalRecords = 0, failedRecords = 0;

    printw("Registros no insertados:\n");
    while (fgets(line, sizeof(line), file)) {
        int id, stock, id_familia;
        float costo, precio;
        char *descripcion = strchr(line, ','); // Buscar la primera coma
        if (!descripcion) {
            printw("Error: Formato de archivo incorrecto.\n");
            fclose(file);
            refresh();
            getch();
            return;
        }
        *descripcion = '\0'; // Separar el ID de la descripción
        descripcion++; // Mover el puntero al inicio de la descripción

        id = atoi(line); // Convertir el ID a entero

        char *familia = strchr(descripcion, ','); // Buscar la segunda coma
        if (!familia) {
            printw("Error: Formato de archivo incorrecto.\n");
            fclose(file);
            refresh();
            getch();
            return;
        }
        *familia = '\0'; // Separar la descripción de la familia
        familia++; // Mover el puntero al inicio de la familia

        if (sscanf(familia, "%d,%f,%f,%d", &id_familia, &costo, &precio, &stock) != 4) {
            printw("Error: Formato de archivo incorrecto.\n");
            fclose(file);
            refresh();
            getch();
            return;
        }

        String *descripcionStr = newString(descripcion); // Crear un String dinámico para la descripción

        totalRecords++;

        // Crear un nuevo producto
        Producto *producto = (Producto *)malloc(sizeof(Producto));
        producto->id = id;
        producto->descripcion = descripcionStr;
        producto->id_familia = id_familia;
        producto->costo = costo;
        producto->precio = precio;
        producto->stock = stock;

        // Agregar el producto al array dinámico
        ptrArrayAppend(producto, productos);

        // Insertar el producto en la base de datos
        char query[256];
        snprintf(query, sizeof(query), 
                 "INSERT INTO Producto (id, descripcion, stock, costo, precio, id_familia) "
                 "VALUES (%d, '%.*s', %d, %f, %f, %d)", 
                 id, descripcionStr->len, descripcionStr->text, stock, costo, precio, id_familia);

        if (mysql_query(conn, query)) {
            // Mostrar el registro no insertado en la consola
            printw("ID: %d, Nombre: %.*s\n", id, descripcionStr->len, descripcionStr->text);
            failedRecords++;
        }
    }

    fclose(file);

    // Mostrar resumen
    printw("\nTotal de registros: %d\n", totalRecords);
    printw("Registros fallidos: %d\n", failedRecords);
    refresh();
    getch();
}

// Función para registrar una familia de productos
void registerProductFamily(MYSQL *conn) {
    PtrArray *familias =newPtrArray();
    char filePath[256];
    printw("Ingrese la ruta del archivo de familias: ");
    echo(); // Habilitar la visualización de la entrada
    scanw("%255s", filePath); // Leer la ruta del archivo
    noecho(); // Deshabilitar la visualización de la entrada

    // Agregar la nueva familia al archivo y al array dinámico
    int id;
    printw("Ingrese el ID de la familia: ");
    scanw("%d", &id);
    printw("Ingrese la descripción de la familia: ");
    String *descripcionStr = readStringFromInput(); // Leer la descripción dinámicamente

    addFamilyToFile(filePath, familias, id, descripcionStr);

    // Cargar familias desde el archivo a la base de datos
    loadFamiliesFromFile(conn, filePath, familias);
}

// Función para registrar un producto
void registerProduct(MYSQL *conn) {
    PtrArray *productos = newPtrArray();
    char filePath[256];
    printw("Ingrese la ruta del archivo de productos: ");
    echo(); // Habilitar la visualización de la entrada
    scanw("%255s", filePath); // Leer la ruta del archivo
    noecho(); // Deshabilitar la visualización de la entrada

    // Agregar el nuevo producto al archivo y al array dinámico
    int id, stock, id_familia;
    float costo, precio;
    printw("Ingrese el ID del producto: ");
    scanw("%d", &id);
    printw("Ingrese la descripción del producto: ");
    String *descripcionStr = readStringFromInput(); // Leer la descripción dinámicamente
    printw("Ingrese la familia del producto: ");
    scanw("%d", &id_familia);
    printw("Ingrese el costo del producto: ");
    scanw("%f", &costo);
    printw("Ingrese el precio del producto: ");
    scanw("%f", &precio);
    printw("Ingrese el stock del producto: ");
    scanw("%d", &stock);

    addProductToFile(filePath, productos, id, descripcionStr, id_familia, costo, precio, stock);

    // Cargar productos desde el archivo a la base de datos
    loadProductsFromFile(conn, filePath, productos);
}
