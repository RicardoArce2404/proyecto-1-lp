CREATE DATABASE puntodeventa;
USE puntodeventa;

CREATE TABLE Usuarios (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(100) NOT NULL,
    password_hash CHAR(64) NOT NULL -- Para almacenar el hash SHA-256
);

INSERT INTO Usuarios (username, password_hash)
VALUES ('admin', SHA2('admin123', 256)); -- SHA-256

CREATE TABLE Familia (
	id_familia varchar(10) primary key ,
    descripcion varchar(100)
);

CREATE TABLE Producto (
	id_producto varchar(10) primary key,
    descripcion varchar(100),
    stock int,
    costo float,
    precio float,
    id_familia varchar(10),
	foreign key (id_familia) references Familia(id_familia)
);

CREATE TABLE Cotizacion (
	id_cotizacion int primary key auto_increment,
    estado varchar(10),
    CONSTRAINT chk_estado CHECK (estado IN ('Facturado', 'Pendiente'))
);

CREATE TABLE DetalleCotizacion (
	cantidad int,
    id_producto varchar(10),
    id_cotizacion int,
    foreign key (id_producto) references Producto(id_producto),
    foreign key (id_cotizacion) references Cotizacion(id_cotizacion)
);

CREATE TABLE Factura (
	id_factura int primary key auto_increment,
    fecha date,
    hora time,
    cliente varchar(50),
    id_cotizacion int,
    foreign key (id_cotizacion) references Cotizacion(id_cotizacion)
);

CREATE TABLE Empresa (
    id INT AUTO_INCREMENT PRIMARY KEY, -- Identificador único
    nombre VARCHAR(100) NOT NULL,      -- Nombre del local comercial
    telefono VARCHAR(15) NOT NULL,     -- Número de teléfono
    cedula_juridica VARCHAR(20) NOT NULL, -- Cédula jurídica
    horario_atencion VARCHAR(50) NOT NULL, -- Horario de atención
    numero_secuencial_factura INT NOT NULL, -- Número secuencial de la siguiente factura
    usuario_admin VARCHAR(50) NOT NULL, -- Usuario del administrador
    contrasena_admin CHAR(64) NOT NULL  -- Contraseña del administrador (hash SHA-256)
);

INSERT INTO Empresa (
    nombre, 
    telefono, 
    cedula_juridica, 
    horario_atencion, 
    numero_secuencial_factura, 
    usuario_admin, 
    contrasena_admin
) VALUES (
    'PulpeTEC',         -- Nombre del local
    '1234-5678',                  -- Teléfono
    '3-101-123456',               -- Cédula jurídica
    'Lunes a Viernes, 8:00-17:00', -- Horario de atención
    1,                            -- Número secuencial inicial de factura
    'admin',                      -- Usuario del administrador
    SHA2('admin123', 256)         -- Contraseña hasheada del administrador
);