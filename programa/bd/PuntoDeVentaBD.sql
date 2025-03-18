CREATE DATABASE puntodeventa;
USE puntodeventa;

CREATE TABLE Familia (
	id_familia int primary key ,
    descripcion varchar(100)
);

CREATE TABLE Producto (
	id_producto int primary key,
    descripcion varchar(100),
    stock int,
    costo float,
    precio float,
    id_familia int,
	foreign key (id_familia) references Familia(id_familia)
);

CREATE TABLE DetalleCotizacion (
	cantidad int,
    id_producto int,
    id_familia int,
    foreign key (id_producto) references Producto(id_producto),
    foreign key (id_familia) references Familia(id_familia)
);

CREATE TABLE Cotizacion (
	id_cotizacion int primary key auto_increment,
    estado varchar(10),
    CONSTRAINT chk_estado CHECK (estado IN ('Facturado', 'Pendiente'))
);

CREATE TABLE Factura (
	id_factura int primary key auto_increment,
    fecha date,
    hora time,
    id_cotizacion int,
    foreign key (id_cotizacion) references Cotizacion(id_cotizacion)
);