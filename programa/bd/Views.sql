
/*
- Productos por familia
- Productos
- Cotizaciones
- Facturas
*/

    
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

-- Vista para resumen de facturas
CREATE VIEW VistaResumenFacturas AS
SELECT 
    f.id_factura,
    f.fecha,
    f.hora,
    SUM(dc.cantidad * p.precio) as subtotal,
    SUM(dc.cantidad * p.precio) * 0.13 as impuesto,
    SUM(dc.cantidad * p.precio) * 1.13 as total
FROM Factura f
JOIN Cotizacion c ON f.id_cotizacion = c.id_cotizacion
JOIN DetalleCotizacion dc ON c.id_cotizacion = dc.id_cotizacion
JOIN Producto p ON dc.id_producto = p.id_producto
GROUP BY f.id_factura, f.fecha, f.hora;

-- Vista para detalles de factura
CREATE VIEW VistaDetallesFactura AS
SELECT 
    f.id_factura,
    p.id_producto,
    p.descripcion,
    dc.cantidad,
    p.precio as precio_unitario,
    (dc.cantidad * p.precio) as subtotal
FROM Factura f
JOIN Cotizacion c ON f.id_cotizacion = c.id_cotizacion
JOIN DetalleCotizacion dc ON c.id_cotizacion = dc.id_cotizacion
JOIN Producto p ON dc.id_producto = p.id_producto;

-- Vista para encabezado de factura
CREATE VIEW VistaEncabezadoFactura AS
SELECT 
    f.id_factura,
    f.fecha,
    f.hora,
    f.cliente,
    e.nombre as nombre_empresa,
    e.cedula_juridica,
    e.telefono,
    SUM(dc.cantidad * p.precio) as subtotal,
    SUM(dc.cantidad * p.precio) * 0.13 as impuesto,
    SUM(dc.cantidad * p.precio) * 1.13 as total
FROM Factura f
JOIN Cotizacion c ON f.id_cotizacion = c.id_cotizacion
JOIN DetalleCotizacion dc ON c.id_cotizacion = dc.id_cotizacion
JOIN Producto p ON dc.id_producto = p.id_producto
CROSS JOIN Empresa e
GROUP BY f.id_factura, f.fecha, f.hora, f.cliente, e.nombre, e.cedula_juridica, e.telefono;