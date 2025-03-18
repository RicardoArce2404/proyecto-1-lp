#include <stdio.h>
#include <stdlib.h>
#include "main.c"

void registerProductFamily(MYSQL *conn) {
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

void registerProduct(MYSQL *conn) {
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