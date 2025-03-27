
/*
- Productos por familia
- Productos
- Cotizaciones
- Facturas
*/

CREATE VIEW VistaProductosPorFamilia AS
SELECT 
    p.id_producto,
    p.descripcion AS producto,
    p.stock,
    p.precio,
    f.descripcion AS familia
FROM 
    Producto p
JOIN 
    Familia f ON p.id_familia = f.id_familia
ORDER BY 
    f.descripcion, p.descripcion;
    
CREATE VIEW VistaProductos AS
SELECT 
    p.id_producto,
    p.descripcion AS producto,
    p.stock,
    p.costo,
    p.precio,
    f.descripcion AS familia
FROM 
    Producto p
JOIN 
    Familia f ON p.id_familia = f.id_familia;
    
CREATE VIEW VistaCotizaciones AS
SELECT 
    c.id_cotizacion,
    c.estado,
    COUNT(dc.id_producto) AS cantidad_productos,
    SUM(dc.cantidad * p.precio) AS total_estimado
FROM 
    Cotizacion c
LEFT JOIN 
    DetalleCotizacion dc ON c.id_cotizacion = dc.id_cotizacion
LEFT JOIN
    Producto p ON dc.id_producto = p.id_producto
GROUP BY 
    c.id_cotizacion, c.estado;


CREATE VIEW VistaTop5ProductosMasVendidos AS
SELECT 
    p.id_producto,
    p.descripcion AS producto,
    SUM(dc.cantidad) AS cantidad_vendida
FROM 
    DetalleCotizacion dc
JOIN 
    Producto p ON dc.id_producto = p.id_producto
JOIN
    Cotizacion c ON dc.id_cotizacion = c.id_cotizacion
WHERE 
    c.estado = 'Facturado'
GROUP BY 
    p.id_producto, p.descripcion
ORDER BY 
    cantidad_vendida DESC
LIMIT 5;

CREATE VIEW VistaProductoMasVendidoPorFamilia AS
WITH VentasPorProducto AS (
    SELECT 
        p.id_familia,
        p.id_producto,
        p.descripcion AS producto,
        SUM(dc.cantidad) AS cantidad_vendida,
        RANK() OVER (PARTITION BY p.id_familia ORDER BY SUM(dc.cantidad) DESC) AS ranking
    FROM 
        DetalleCotizacion dc
    JOIN 
        Producto p ON dc.id_producto = p.id_producto
    JOIN
        Cotizacion c ON dc.id_cotizacion = c.id_cotizacion
    WHERE 
        c.estado = 'Facturado'
    GROUP BY 
        p.id_familia, p.id_producto, p.descripcion
)
SELECT 
    f.descripcion AS familia,
    v.producto,
    v.cantidad_vendida
FROM 
    VentasPorProducto v
JOIN
    Familia f ON v.id_familia = f.id_familia
WHERE 
    v.ranking = 1;
    
CREATE VIEW VistaMontoVendidoPorFamilia AS
SELECT 
    f.descripcion AS familia,
    SUM(dc.cantidad * p.precio) AS monto_total
FROM 
    DetalleCotizacion dc
JOIN 
    Producto p ON dc.id_producto = p.id_producto
JOIN
    Familia f ON p.id_familia = f.id_familia
JOIN
    Cotizacion c ON dc.id_cotizacion = c.id_cotizacion
WHERE 
    c.estado = 'Facturado'
GROUP BY 
    f.id_familia, f.descripcion
ORDER BY 
    monto_total DESC;

