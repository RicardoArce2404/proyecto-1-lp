-- Procedimientos almacenados 

DELIMITER //

CREATE PROCEDURE RegistrarFamilia(
    IN p_id_familia INT,
    IN p_descripcion VARCHAR(100),
    OUT p_resultado INT) -- 1: ID ya existe, 2: Éxito
BEGIN
    DECLARE v_existe INT DEFAULT 0;
    
    SELECT COUNT(*) INTO v_existe FROM Familia WHERE id_familia = p_id_familia;
    
    IF v_existe > 0 THEN
        SET p_resultado = 1;
    ELSE
        INSERT INTO Familia (id_familia, descripcion) 
        VALUES (p_id_familia, p_descripcion);
        SET p_resultado = 0;
    END IF;
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE RegistrarProducto(
    IN p_id_producto INT,
    IN p_descripcion VARCHAR(100),
    IN p_stock INT,
    IN p_costo DECIMAL(10,2),
    IN p_precio DECIMAL(10,2),
    IN p_id_familia INT,
    OUT p_resultado INT) -- 1: ID existe, 2: Familia no existe, 3: Costo/precio inválido, 4: Precio < Costo, 0: Éxito
BEGIN
    DECLARE v_existe_producto INT DEFAULT 0;
    DECLARE v_existe_familia INT DEFAULT 0;
    
    SELECT COUNT(*) INTO v_existe_producto FROM Producto WHERE id_producto = p_id_producto;
    SELECT COUNT(*) INTO v_existe_familia FROM Familia WHERE id_familia = p_id_familia;
    
    IF v_existe_producto > 0 THEN
        SET p_resultado = 1;
    ELSEIF v_existe_familia = 0 THEN
        SET p_resultado = 2;
    ELSEIF p_costo <= 0 OR p_precio <= 0 THEN
        SET p_resultado = 3;
    ELSEIF p_precio < p_costo THEN
        SET p_resultado = 4;
    ELSE
        INSERT INTO Producto (id_producto, descripcion, stock, costo, precio, id_familia)
        VALUES (p_id_producto, p_descripcion, p_stock, p_costo, p_precio, p_id_familia);
        SET p_resultado = 0;
    END IF;
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE CargarInventario(
    IN p_id_producto INT,
    IN p_cantidad INT,
    OUT p_resultado INT, -- 1: Producto no existe, 2: Cantidad inválida, 0: Éxito
    OUT p_stock_actualizado INT)
BEGIN
    DECLARE v_stock_actual INT DEFAULT 0;
    
    SELECT IFNULL(stock, -1) INTO v_stock_actual 
    FROM Producto WHERE id_producto = p_id_producto;
    
    IF v_stock_actual = -1 THEN
        SET p_resultado = 1;
        SET p_stock_actualizado = 0;
    ELSEIF p_cantidad <= 0 THEN
        SET p_resultado = 2;
        SET p_stock_actualizado = v_stock_actual;
    ELSE
        UPDATE Producto 
        SET stock = stock + p_cantidad 
        WHERE id_producto = p_id_producto;
        
        SELECT stock INTO p_stock_actualizado FROM Producto WHERE id_producto = p_id_producto;
        SET p_resultado = 0;
    END IF;
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE RegistrarCotizacion(
    OUT p_id_cotizacion INT,
    OUT p_resultado INT) -- 0: Éxito, 1: Error
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_resultado = 1;
        ROLLBACK;
    END;
    
    START TRANSACTION;
    
    INSERT INTO Cotizacion (estado) VALUES ('Pendiente');
    SET p_id_cotizacion = LAST_INSERT_ID();
    SET p_resultado = 0;
    
    COMMIT;
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE AgregarDetalleCotizacion(
    IN p_id_cotizacion INT,
    IN p_id_producto INT,
    IN p_cantidad INT,
    OUT p_resultado INT) -- 1: No existe cotización, 2: Stock insuficiente, 3: Producto no existe, 4: Éxito
BEGIN
    DECLARE v_id_familia INT;
    DECLARE v_estado VARCHAR(10);
    DECLARE v_existe_producto INT;
    DECLARE v_stock_disponible INT;
    DECLARE v_existe_detalle INT;
    DECLARE v_cantidad_actual INT;

    BEGIN
        SET p_resultado = 0; -- Error general
        ROLLBACK;
    END;
    
    START TRANSACTION;
    
    -- Verificar si la cotización existe y está pendiente
    SELECT estado INTO v_estado FROM Cotizacion WHERE id_cotizacion = p_id_cotizacion;
    
    IF v_estado IS NULL THEN
        SET p_resultado = 1; -- No existe la cotización
    ELSE
        -- Verificar producto y obtener familia
        SELECT COUNT(*), id_familia INTO v_existe_producto, v_id_familia 
        FROM Producto WHERE id_producto = p_id_producto;
        
        IF v_existe_producto = 0 THEN
            SET p_resultado = 3; -- Producto no existe
        ELSE
            -- Verificar stock disponible
            SELECT stock INTO v_stock_disponible FROM Producto WHERE id_producto = p_id_producto;
            
            -- Verificar si el producto ya está en la cotización
            SELECT COUNT(*), IFNULL(cantidad, 0) INTO v_existe_detalle, v_cantidad_actual
            FROM DetalleCotizacion 
            WHERE id_cotizacion = p_id_cotizacion AND id_producto = p_id_producto;
            
            IF v_stock_disponible < (CASE WHEN v_existe_detalle > 0 THEN v_cantidad_actual + p_cantidad ELSE p_cantidad END) THEN
                SET p_resultado = 2; -- Stock insuficiente
            ELSE
                IF v_existe_detalle > 0 THEN
                    -- Si ya existe, actualizar la cantidad
                    UPDATE DetalleCotizacion 
                    SET cantidad = cantidad + p_cantidad
                    WHERE id_cotizacion = p_id_cotizacion AND id_producto = p_id_producto;
                ELSE
                    -- Si no existe, insertar nuevo registro
                    INSERT INTO DetalleCotizacion (cantidad, id_producto, id_familia, id_cotizacion)
                    VALUES (p_cantidad, p_id_producto, v_id_familia, p_id_cotizacion);
                END IF;
                
                SET p_resultado = 4; -- Éxito
            END IF;
        END IF;
    END IF;
    
    COMMIT;
END 

DELIMITER ;

DELIMITER //

CREATE PROCEDURE AgregarFactura(
    IN p_id_cotizacion INT,
    OUT p_id_factura INT,
    OUT p_resultado INT) -- 1: Cotización no existe, 2: Sin items, 3: Stock insuficiente, 0: Éxito
BEGIN
    DECLARE v_estado VARCHAR(10);
    DECLARE v_existe_cotizacion INT;
    DECLARE v_total_items INT;
    DECLARE v_stock_insuficiente INT DEFAULT 0;
    
    START TRANSACTION;
    
    SELECT COUNT(*), estado INTO v_existe_cotizacion, v_estado 
    FROM Cotizacion WHERE id_cotizacion = p_id_cotizacion;
    
    IF v_existe_cotizacion = 0 THEN
        SET p_resultado = 1;
    ELSE
        SELECT COUNT(*) INTO v_total_items 
        FROM DetalleCotizacion 
        WHERE id_cotizacion = p_id_cotizacion;
        
        IF v_total_items = 0 THEN
            SET p_resultado = 2;
        ELSE
            SELECT COUNT(*) INTO v_stock_insuficiente
            FROM DetalleCotizacion dc
            JOIN Producto p ON dc.id_producto = p.id_producto
            WHERE dc.id_cotizacion = p_id_cotizacion AND dc.cantidad > p.stock;
            
            IF v_stock_insuficiente > 0 THEN
                SET p_resultado = 3;
            ELSE
                UPDATE Producto p
                JOIN DetalleCotizacion dc ON p.id_producto = dc.id_producto
                SET p.stock = p.stock - dc.cantidad
                WHERE dc.id_cotizacion = p_id_cotizacion;
                
                INSERT INTO Factura (fecha, hora, id_cotizacion)
                VALUES (CURDATE(), CURTIME(), p_id_cotizacion);
                
                SET p_id_factura = LAST_INSERT_ID();
                UPDATE Cotizacion SET estado = 'Facturado' WHERE id_cotizacion = p_id_cotizacion;
                SET p_resultado = 0;
            END IF;
        END IF;
    END IF;
    
    COMMIT;
END //

DELIMITER ;

