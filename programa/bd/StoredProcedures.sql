-- Procedimientos almacenados 

DELIMITER //

CREATE PROCEDURE RegistrarFamilia(
    IN p_id_familia VARCHAR(10),
    IN p_descripcion VARCHAR(100),
    OUT p_resultado INT) -- 1: ID ya existe, 6: Éxito
BEGIN
    DECLARE v_existe INT DEFAULT 0;
    
    SELECT COUNT(*) INTO v_existe FROM Familia WHERE id_familia = p_id_familia;
    
    IF v_existe > 0 THEN
        SET p_resultado = 1;
    ELSE
        INSERT INTO Familia (id_familia, descripcion) 
        VALUES (p_id_familia, p_descripcion);
        SET p_resultado = 6;
    END IF;
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE RegistrarProducto(
    IN p_id_producto VARCHAR(10),
    IN p_descripcion VARCHAR(100),
    IN p_stock INT,
    IN p_costo FLOAT,
    IN p_precio FLOAT,
    IN p_familia_desc VARCHAR(100),  -- Cambiado a descripción de familia
    OUT p_resultado INT) -- 1: ID existe, 2: Familia no existe, 3: Costo/precio inválido, 4: Precio < Costo, 5: Éxito
BEGIN
    DECLARE v_existe_producto INT DEFAULT 0;
    DECLARE v_existe_familia INT DEFAULT 0;
    DECLARE v_id_familia VARCHAR(10);  -- Para almacenar el ID de familia encontrado

    -- Primero buscamos el ID de familia basado en la descripción
    SELECT id_familia INTO v_id_familia 
    FROM Familia 
    WHERE descripcion = p_familia_desc
    LIMIT 1;
    
    -- Verificamos si encontramos la familia
    IF v_id_familia IS NULL THEN
        SET v_existe_familia = 0;
    ELSE
        SET v_existe_familia = 1;
    END IF;
    
    -- Verificamos si el producto ya existe
    SELECT COUNT(*) INTO v_existe_producto 
    FROM Producto 
    WHERE id_producto = p_id_producto;
    
    -- Lógica de validación
    IF v_existe_producto > 0 THEN
        SET p_resultado = 1;  -- ID de producto ya existe
    ELSEIF v_existe_familia = 0 THEN
        SET p_resultado = 2;  -- Familia no existe
    ELSEIF p_costo <= 0 OR p_precio <= 0 THEN
        SET p_resultado = 3;  -- Costo o precio inválido
    ELSEIF p_precio < p_costo THEN
        SET p_resultado = 4;  -- Precio menor que costo
    ELSE
        -- Todo correcto, insertamos el producto
        INSERT INTO Producto (id_producto, descripcion, stock, costo, precio, id_familia)
        VALUES (p_id_producto, p_descripcion, p_stock, p_costo, p_precio, v_id_familia);
        SET p_resultado = 5;  -- Éxito
    END IF;
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE EliminarProducto(
    IN p_id_producto VARCHAR(10),
    OUT p_resultado INT) -- 1: Éxito, 2: Producto no existe, 3: Producto está en cotizaciones/facturas
BEGIN
    DECLARE v_existe INT DEFAULT 0;
    DECLARE v_en_uso INT DEFAULT 0;
    
    -- Verificar si el producto existe
    SELECT COUNT(*) INTO v_existe FROM Producto WHERE id_producto = p_id_producto;
    
    IF v_existe = 0 THEN
        SET p_resultado = 2; -- Producto no existe
    ELSE
        -- Verificar si el producto está en uso en cotizaciones o facturas
        SELECT COUNT(*) INTO v_en_uso FROM DetalleCotizacion WHERE id_producto = p_id_producto;
        
        IF v_en_uso > 0 THEN
            SET p_resultado = 3; -- Producto está en uso
        ELSE
            -- Eliminar el producto
            DELETE FROM Producto WHERE id_producto = p_id_producto;
            SET p_resultado = 1; -- Éxito
        END IF;
    END IF;
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE CargarInventario(
    IN p_id_producto VARCHAR(10),
    IN p_cantidad INT,
    OUT p_resultado INT
)
/*
Códigos de retorno:
1: Éxito en la operación
2: Producto no encontrado
3: Stock insuficiente para restar (resultaría en negativo)
*/
BEGIN
    DECLARE v_stock_actual INT;
    DECLARE v_nuevo_stock INT;

    -- Verificar si el producto existe
    SELECT COUNT(*) INTO @existe FROM Producto WHERE id_producto = p_id_producto;
    
    IF @existe = 0 THEN
        SET p_resultado = 2;
    ELSE
        -- Obtener el stock actual
        SELECT stock INTO v_stock_actual FROM Producto WHERE id_producto = p_id_producto;
        

        IF (((p_cantidad<0 && (v_stock_actual + p_cantidad)<0))) THEN
            SET p_resultado = 3;
        ELSE
            SET v_nuevo_stock = v_stock_actual + p_cantidad;
            UPDATE Producto SET stock = v_nuevo_stock WHERE id_producto = p_id_producto;
            SET p_resultado = 1;
        END IF; 
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
    IN p_id_producto VARCHAR(10),
    IN p_cantidad INT,
    OUT p_resultado INT) -- 1: No existe cotización, 2: Stock insuficiente, 3: Producto no existe, 4: Éxito
BEGIN
    DECLARE v_id_familia VARCHAR(10);
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
        FROM Producto WHERE id_producto = p_id_producto
	    GROUP BY id_familia;

        IF v_existe_producto = 0 THEN
            SET p_resultado = 3; -- Producto no existe
        ELSE
            -- Verificar stock disponible
            SELECT stock INTO v_stock_disponible FROM Producto WHERE id_producto = p_id_producto;

            -- Verificar si el producto ya está en la cotización
            SELECT COUNT(*), IFNULL(cantidad, 0) INTO v_existe_detalle, v_cantidad_actual
            FROM DetalleCotizacion
            WHERE id_cotizacion = p_id_cotizacion AND id_producto = p_id_producto
	        GROUP BY cantidad;

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
                    INSERT INTO DetalleCotizacion (cantidad, id_producto, id_cotizacion)
                    VALUES (p_cantidad, p_id_producto, p_id_cotizacion);
                END IF;

                SET p_resultado = 4; -- Éxito
            END IF;
        END IF;
    END IF;
    
    COMMIT;
END //

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

DELIMITER //

CREATE PROCEDURE sp_DetalleInfoCotizacion(IN id_cotizacion INT)
BEGIN
    SELECT p.descripcion, f.descripcion, dc.cantidad, p.precio, p.precio * dc.cantidad, p.id_producto
    FROM Producto p
    JOIN Familia f ON p.id_familia = f.id_familia
    JOIN DetalleCotizacion dc ON p.id_producto = dc.id_producto
    WHERE dc.id_cotizacion = id_cotizacion;
END //

DELIMITER ;

