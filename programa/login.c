#include <ncurses.h>
#include <mysql/mysql.h>
#include <string.h>

// Función para validar el inicio de sesión del administrador
int validateAdminLogin(MYSQL *conn, const char *username, const char *password) {
    // Consulta SQL para obtener el hash almacenado en la base de datos
    char query[256];
    snprintf(query, sizeof(query), 
             "SELECT contrasena_admin FROM Empresa WHERE usuario_admin='%s'", username);

    if (mysql_query(conn, query)) {
        printw("Error al verificar las credenciales: %s\n", mysql_error(conn));
        return 0; // Credenciales incorrectas
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        printw("Error al obtener el resultado: %s\n", mysql_error(conn));
        return 0; // Credenciales incorrectas
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        printw("Usuario no encontrado.\n");
        mysql_free_result(result);
        return 0; // Usuario no existe
    }

    // Obtener el hash almacenado en la base de datos
    const char *storedHash = row[0];

    // Hashear la contraseña ingresada por el usuario usando SHA2
    char hashedPasswordQuery[256];
    snprintf(hashedPasswordQuery, sizeof(hashedPasswordQuery), 
             "SELECT SHA2('%s', 256)", password);

    if (mysql_query(conn, hashedPasswordQuery)) {
        printw("Error al hashear la contraseña: %s\n", mysql_error(conn));
        mysql_free_result(result);
        return 0; // Error al hashear
    }

    MYSQL_RES *hashedResult = mysql_store_result(conn);
    if (hashedResult == NULL) {
        printw("Error al obtener el hash de la contraseña: %s\n", mysql_error(conn));
        mysql_free_result(result);
        return 0; // Error al hashear
    }

    MYSQL_ROW hashedRow = mysql_fetch_row(hashedResult);
    const char *hashedPassword = hashedRow[0];

    // Comparar el hash almacenado con el hash de la contraseña ingresada
    if (strcmp(storedHash, hashedPassword) == 0) {
        mysql_free_result(result);
        mysql_free_result(hashedResult);
        return 1; // Credenciales correctas
    } else {
        printw("Nombre de usuario o contraseña incorrectos.\n");
        mysql_free_result(result);
        mysql_free_result(hashedResult);
        return 0; // Credenciales incorrectas
    }
}

// Función para solicitar credenciales y validarlas
int adminLogin(MYSQL *conn) {
    char username[100];
    char password[100];

    printw("Ingrese su nombre de usuario: ");
    echo(); // Habilitar la visualización de la entrada
    scanw("%s", username);
    noecho(); // Deshabilitar la visualización de la entrada

    printw("Ingrese su contraseña: ");
    echo();
    scanw("%s", password);
    noecho();

    // Validar las credenciales
    if (validateAdminLogin(conn, username, password)) {
        printw("Inicio de sesión exitoso.\n");
        return 1; // Credenciales correctas
    } else {
        printw("Nombre de usuario o contraseña incorrectos.\n");
        return 0; // Credenciales incorrectas
    }
}