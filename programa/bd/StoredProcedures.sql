-- Procedimientos almacenados 

DELIMITER //

CREATE PROCEDURE RegistrarFamilia(
    IN p_id_familia INT,
    IN p_descripcion VARCHAR(100),
    OUT p_mensaje VARCHAR(100)
)
BEGIN
    DECLARE v_existe INT DEFAULT 0;
    
    SELECT COUNT(*) INTO v_existe FROM Familia WHERE id_familia = p_id_familia;
    
    IF v_existe > 0 THEN
        SET p_mensaje = 'Error: El ID de familia ya existe';
    ELSEIF p_descripcion IS NULL OR p_descripcion = '' THEN
        SET p_mensaje = 'Error: La descripción no puede estar vacía';
    ELSE
        INSERT INTO Familia (id_familia, descripcion) 
        VALUES (p_id_familia, p_descripcion);
        
        SET p_mensaje = CONCAT('Familia ', p_descripcion, ' registrada con éxito');
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
    OUT p_mensaje VARCHAR(100)
)
BEGIN
    DECLARE v_existe_producto INT DEFAULT 0;
    DECLARE v_existe_familia INT DEFAULT 0;
    
    SELECT COUNT(*) INTO v_existe_producto FROM Producto WHERE id_producto = p_id_producto;
    SELECT COUNT(*) INTO v_existe_familia FROM Familia WHERE id_familia = p_id_familia;
    
    IF v_existe_producto > 0 THEN
        SET p_mensaje = 'Error: El ID de producto ya existe';
    ELSEIF v_existe_familia = 0 THEN
        SET p_mensaje = 'Error: La familia especificada no existe';
    ELSEIF p_costo <= 0 OR p_precio <= 0 THEN
        SET p_mensaje = 'Error: Costo y precio deben ser mayores a cero';
    ELSEIF p_precio < p_costo THEN
        SET p_mensaje = 'Error: El precio no puede ser menor al costo';
    ELSE
        INSERT INTO Producto (id_producto, descripcion, stock, costo, precio, id_familia)
        VALUES (p_id_producto, p_descripcion, p_stock, p_costo, p_precio, p_id_familia);
        
        SET p_mensaje = CONCAT('Producto ', p_descripcion, ' registrado con éxito');
    END IF;
END //

DELIMITER ;

DELIMITER //

CREATE PROCEDURE CargarInventario(
    IN p_id_producto INT,
    IN p_cantidad INT,
    OUT p_mensaje VARCHAR(100),
    OUT p_stock_actualizado INT
)
BEGIN
    DECLARE v_stock_actual INT DEFAULT 0;
    
    SELECT IFNULL(stock, -1) INTO v_stock_actual 
    FROM Producto WHERE id_producto = p_id_producto;
    
    IF v_stock_actual = -1 THEN
        SET p_mensaje = 'Error: Producto no encontrado';
        SET p_stock_actualizado = 0;
    ELSEIF p_cantidad <= 0 THEN
        SET p_mensaje = 'Error: La cantidad debe ser positiva';
        SET p_stock_actualizado = v_stock_actual;
    ELSE
        UPDATE Producto 
        SET stock = stock + p_cantidad 
        WHERE id_producto = p_id_producto;
        
        SELECT stock INTO p_stock_actualizado FROM Producto WHERE id_producto = p_id_producto;
        
        SET p_mensaje = CONCAT('Inventario actualizado. Stock anterior: ', v_stock_actual, 
                             ', Cantidad agregada: ', p_cantidad, 
                             ', Nuevo stock: ', p_stock_actualizado);
    END IF;
END //

DELIMITER ;
