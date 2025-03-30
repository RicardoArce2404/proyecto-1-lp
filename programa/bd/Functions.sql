

DELIMITER //

CREATE FUNCTION CotizacionesPendientes() 
RETURNS INT
DETERMINISTIC
BEGIN
    DECLARE v_count INT;
    
    SELECT COUNT(*) INTO v_count 
    FROM Cotizacion 
    WHERE estado = 'Pendiente';
    
    RETURN v_count;
END //

DELIMITER ;

DELIMITER //

CREATE FUNCTION CotizacionesFacturadas() 
RETURNS INT
DETERMINISTIC
BEGIN
    DECLARE v_count INT;
    
    SELECT COUNT(*) INTO v_count 
    FROM Cotizacion 
    WHERE estado = 'Facturado';
    
    RETURN v_count;
END //

DELIMITER ;

DELIMITER //

CREATE FUNCTION fn_PromedioCompra() 
RETURNS FLOAT
DETERMINISTIC
BEGIN
    DECLARE v_promedio FLOAT;
    
    SELECT AVG(total) INTO v_promedio
    FROM (
        SELECT SUM(dc.cantidad * p.precio) AS total
        FROM Factura f
        JOIN Cotizacion c ON f.id_cotizacion = c.id_cotizacion
        JOIN DetalleCotizacion dc ON c.id_cotizacion = dc.id_cotizacion
        JOIN Producto p ON dc.id_producto = p.id_producto
        GROUP BY f.id_factura
    ) AS totals;
    
    RETURN IFNULL(v_promedio, 0);
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE sp_DetalleFactura(
    IN p_id_factura INT,
    OUT p_resultado TEXT
)
BEGIN
    DECLARE v_info_empresa TEXT;
    DECLARE v_info_factura TEXT;
    DECLARE v_detalle_productos TEXT;
    DECLARE v_subtotal DECIMAL(10,2);
    DECLARE v_impuesto DECIMAL(10,2);
    DECLARE v_total DECIMAL(10,2);
    
    -- Obtener información de la empresa
    SELECT CONCAT(
        'Nombre: ', nombre, '\n',
        'Teléfono: ', telefono, '\n',
        'Cédula Jurídica: ', cedula_juridica
    ) INTO v_info_empresa
    FROM Empresa
    LIMIT 1;
    
    -- Obtener información de la factura
    SELECT CONCAT(
        'Factura #: ', id_factura, '\n',
        'Fecha: ', DATE_FORMAT(fecha, '%d/%m/%Y'), '\n',
        'Hora: ', TIME_FORMAT(hora, '%H:%i'), '\n',
        'Cliente: ', IFNULL(cliente, 'Consumidor Final')
    ) INTO v_info_factura
    FROM Factura
    WHERE id_factura = p_id_factura;
    
    -- Obtener detalle de productos
    SELECT GROUP_CONCAT(
        CONCAT(
            'Producto: ', p.descripcion, '\n',
            'Precio unitario: ', FORMAT(p.precio, 2), '\n',
            'Cantidad: ', dc.cantidad, '\n',
            'Subtotal: ', FORMAT(dc.cantidad * p.precio, 2), '\n',
            '--------------------------------'
        ) SEPARATOR '\n'
    ) INTO v_detalle_productos
    FROM Factura f
    JOIN Cotizacion c ON f.id_cotizacion = c.id_cotizacion
    JOIN DetalleCotizacion dc ON c.id_cotizacion = dc.id_cotizacion
    JOIN Producto p ON dc.id_producto = p.id_producto
    WHERE f.id_factura = p_id_factura;
    
    -- Calcular totales
    SELECT SUM(dc.cantidad * p.precio) INTO v_subtotal
    FROM Factura f
    JOIN Cotizacion c ON f.id_cotizacion = c.id_cotizacion
    JOIN DetalleCotizacion dc ON c.id_cotizacion = dc.id_cotizacion
    JOIN Producto p ON dc.id_producto = p.id_producto
    WHERE f.id_factura = p_id_factura;
    
    SET v_impuesto = ROUND(v_subtotal * 0.13, 2);
    SET v_total = v_subtotal + v_impuesto;
    
    -- Construir resultado final
    SET p_resultado = CONCAT(
        '----- INFORMACIÓN DEL LOCAL -----\n',
        v_info_empresa, '\n\n',
        '----- DETALLE DE FACTURA -----\n',
        v_info_factura, '\n\n',
        '----- PRODUCTOS -----\n',
        IFNULL(v_detalle_productos, 'No hay productos'), '\n\n',
        '----- TOTALES -----\n',
        'Subtotal: ', FORMAT(v_subtotal, 2), '\n',
        'Impuesto (13%): ', FORMAT(v_impuesto, 2), '\n',
        'TOTAL: ', FORMAT(v_total, 2)
    );
END //

DELIMITER ;