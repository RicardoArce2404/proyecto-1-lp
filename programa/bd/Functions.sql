

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
