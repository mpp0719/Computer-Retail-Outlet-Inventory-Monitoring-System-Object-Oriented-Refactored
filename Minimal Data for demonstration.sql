drop database project1;
create database project1;
use project1;

create table product
(
	productId varchar(5) primary key,
	productName varchar(200) not null,
	productType varchar(50) not null,
    basePrice decimal(10,2) not null,
    quantity int not null,
    sellingPrice decimal(10,2) not null,
    status boolean not null
);
create table staff
(
	staffId varchar(5) primary key,
    staffName varchar(50) not null,
    staffPos varchar(50) not null,
    staffAddress varchar(200) not null,
    staffEmail varchar(50) not null,
    staffPhone varchar(12) not null,
    staffPassword varchar(6) not null,
    status boolean not null
);
create table sale
(
	saleId varchar(5) primary key,
    paymentMethod varchar(25) not null,
    saleDate date not null,
    subtotal decimal(10,2) not null,
    staffId varchar(5),
    refundTotal decimal(10,2) default 0,
    foreign key (staffId) references staff(staffId)
);
create table sale_detail
(
	productId varchar(5),
	saleId varchar(5),
    quantity int,
    total decimal(10,2),
    refundQtt int default 0,
    refundStatus bool default 0,
    primary key (productId, saleId),
	foreign key (productId) references product(productId),
    foreign key (saleId) references sale(saleId)
);
create table supplier
(
	supplierId varchar(5) primary key,
    supplierName varchar(50) not null,
    supplierPhone varchar(15) not null,
    supplierAddress varchar(200) not null,
    supplierEmail varchar(50) not null,
    status boolean not null
);
create table order_stock
(
	orderId varchar(5) primary key,
    orderDate date,
    staffId varchar(5),
    supplierId varchar(5),
    foreign key (staffId) references staff(staffId),
    foreign key (supplierId) references supplier(supplierId)
);
create table invoice
(
	invoiceId varchar(5) primary key,
    invoiceDate date,
    subtotal decimal(10,2),
    supplierId varchar(5),
    foreign key (supplierId) references supplier(supplierId)
);
create table order_detail
(
	orderDetailId varchar(5),
    orderQuantity int,
    remainingQuantity int,
    status boolean not null,
	itemTotal decimal(10,2),
    orderId varchar(5),
    productId varchar(5),
    primary key (orderDetailId, productId, orderId),
    foreign key (productId) references product(productId),
    foreign key (orderId) references order_stock(orderId)
);
create table inventory
(
	receiveId varchar(5),
    receiveQuantity int not null,
    orderDetailId varchar(5),
    invoiceId varchar(5),
    primary key (receiveId, orderDetailId, invoiceId),
    foreign key (orderDetailId) references order_detail(orderDetailId),
    foreign key (invoiceId) references invoice(invoiceId)
);

insert into product values
("P0001", "Gigabyte B840M EAGLE WIFI6", "Motherboard", 499.90, 5, 529.90, 1),
("P0002", "Gigabyte Z790 AORUS Master", "Motherboard", 1799.90, 6, 1999.90, 1),
("P0003", "ASRock B760M Steel Legend", "Motherboard", 699.90, 8, 829.90, 1),
("P0004", "MSI MAG Z790 Tomahawk", "Motherboard", 1499.90, 12, 1699.90, 1),
("P0005", "ASUS TUF Gaming B650-PLUS", "Motherboard", 799.90, 11, 899.90, 1),
("P0006", "Intel Core i3-14100", "CPU Processor", 350.00, 14, 429.00, 1),
("P0007", "Intel Core i3-13100F", "CPU Processor", 300.00, 8, 399.00, 1),
("P0008", "AMD Ryzen 7 7700X", "CPU Processor", 1099.00, 8, 1399.00, 1),
("P0009", "AMD Ryzen 7 7800X3D", "CPU Processor", 1499.00, 6, 1899.00, 1),
("P0010", "Intel Core i9-14900K", "CPU Processor", 1799.00, 6, 2299.00, 1),
("P0011", "ASUS Phoenix GeForce RTX 3050 8GB", "Graphic Card", 799.90, 12, 949.90, 1),
("P0012", "AMD Radeon RX 7900 XTX 24GB", "Graphic Card", 4299.90, 6, 4899.90, 1),
("P0013", "MSI GeForce GTX 1650 D6 VENTUS XS 4G", "Graphic Card", 599.90, 15, 749.90, 1),
("P0014", "NVIDIA GeForce RTX 4060 Ti 8GB", "Graphic Card", 1699.90, 10, 1899.90, 1),
("P0015", "NVIDIA GeForce RTX 4090 24GB", "Graphic Card", 5899.90, 3, 6899.90, 1),
("P0016", "Corsair RM750x 750W 80+ Gold", "Power Supply", 399.90, 8, 499.90, 1),
("P0017", "Seasonic Focus GX-850 850W 80+ Gold", "Power Supply", 499.90, 6, 599.90, 1),
("P0018", "Cooler Master MWE Gold 650W V2", "Power Supply", 299.90, 10, 379.90, 1),
("P0019", "G.Skill Trident Z5 64GB DDR5 6400MHz", "Memory", 899.90, 10, 1099.90, 1),
("P0020", "Team Group T-Force Delta RGB 32GB DDR4 3600MHz", "Memory", 299.90, 12, 389.90, 1),
("P0021", "Seagate Barracuda 2TB 3.5\" HDD", "Storage", 199.90, 30, 279.90, 1),
("P0022", "Kingston NV2 500GB NVMe M.2 SSD", "Storage", 149.90, 25, 199.90, 1),
("P0023", "Sabrent Rocket Q4 4TB NVMe M.2 SSD", "Storage", 1299.90, 8, 1599.90, 1),
("P0024", "ASUS Dual GeForce RTX 4070 SUPER 12GB", "Graphic Card", 2699.90, 6, 2999.90, 1),
("P0025", "XFX Speedster MERC310 RX 7800 XT 16GB", "Graphic Card", 2299.90, 8, 2599.90, 1),
("P0026", "Gigabyte Gaming OC RTX 4060 8GB", "Graphic Card", 1399.90, 15, 1599.90, 1),
("P0027", "Sapphire PULSE RX 7600 8GB", "Graphic Card", 1249.90, 12, 1449.90, 1),
("P0028", "Zotac Gaming RTX 4080 SUPER 16GB", "Graphic Card", 4599.90, 8, 4999.90, 1),
("P0029", "Samsung 980 PRO 1TB NVMe M.2 SSD", "Storage", 429.90, 20, 529.90, 1),
("P0030", "WD Black SN850X 2TB NVMe M.2 SSD", "Storage", 699.90, 15, 849.90, 1),
("P0031", "Crucial P3 Plus 4TB NVMe M.2 SSD", "Storage", 999.90, 8, 1199.90, 1),
("P0032", "Seagate FireCuda 530 1TB NVMe M.2 SSD", "Storage", 529.90, 18, 649.90, 1),
("P0033", "Kingston KC3000 2TB NVMe M.2 SSD", "Storage", 729.90, 12, 899.90, 1),
("P0034", "SanDisk Extreme PRO 1TB SD Card", "Storage", 599.90, 25, 749.90, 1),
("P0035", "WD Blue 4TB 3.5\" HDD", "Storage", 459.90, 30, 579.90, 1),
("P0036", "AMD Ryzen 5 7600X", "CPU Processor", 1099.90, 15, 1299.90, 1),
("P0037", "Intel Core i5-14600K", "CPU Processor", 1349.90, 12, 1549.90, 1),
("P0038", "AMD Ryzen 9 7950X", "CPU Processor", 2599.90, 6, 2999.90, 1),
("P0039", "Intel Core i7-14700K", "CPU Processor", 1899.90, 10, 2199.90, 1),
("P0040", "AMD Ryzen 5 5600G", "CPU Processor", 799.90, 20, 949.90, 1),
("P0041", "NZXT H7 Flow RGB Mid Tower", "Casing", 429.90, 15, 549.90, 1),
("P0042", "Lian Li Lancool 216 RGB", "Casing", 469.90, 18, 599.90, 1),
("P0043", "Fractal Design North Charcoal", "Casing", 649.90, 8, 799.90, 1),
("P0044", "Cooler Master MasterBox TD500", "Casing", 499.90, 12, 629.90, 1),
("P0045", "Corsair 4000D Airflow", "Casing", 429.90, 20, 549.90, 1),
("P0046", "Phanteks Eclipse G360A", "Casing", 389.90, 25, 499.90, 1),
("P0047", "NZXT H9 Flow Dual Chamber", "Casing", 749.90, 6, 899.90, 1),
("P0048", "Thermaltake CTE C750", "Casing", 999.90, 9, 1199.90, 1),
("P0049", "be quiet! Pure Base 500DX", "Casing", 549.90, 10, 699.90, 1),
("P0050", "Hyte Y60 Modern Aesthetic", "Casing", 899.90, 7, 1099.90, 1);

insert into supplier values
("B0001", "Intel Corp SDN BHD", "013-72158653", "Seksyen 27A, Jalan Industri 3, Bukit Jalil", "intelcorp@gmail.com", 1),
("B0002", "NVIDIA Malaysia", "012-8877665", "Mont Kiara, Kuala Lumpur", "nvidia.my@gmail.com", 1),
("B0003", "Kingston Tech Malaysia", "019-1122334", "Sunway Geo, Bandar Sunway", "kingston.my@yahoo.com", 1),
("B0004", "Seagate Malaysia", "011-6655443", "Cyberjaya, Selangor", "seagate.my@gmail.com", 1),
("B0005", "Gigabyte Malaysia", "016-7788990", "Damansara Perdana, Petaling Jaya", "gigabyte.my@gmail.com", 1),
("B0006", "Corsair Components", "017-5566778", "Bangsar South, Kuala Lumpur", "corsair.my@yahoo.com", 1),
("B0007", "Asrock Corporation", "013-4918146", "Putra Heights, Kuala Lumpur", "asrock@gmail.com", 1),
("B0008", "MSI Computer Tech Sdn Bhd", "019-1375457", "Bandar Baru Bangi, Selangor", "msitech@outlook.com", 1),
("B0009", "ASUS Republic", "012-9878935", "Cheras, Kuala Lumpur", "asusrepublic@outlook.com", 1),
("B0010", "AMD Solutions Sdn Bhd", "011-56528654", "Petaling Jaya, Selangor", "amdcapital@yahoo.com", 1);

insert into staff values
("S0001", "Mun Pin Pin", "Staff", "Pangsapuri Ixora, Bukit Beruang", "mpp@gmail.com", "013-2723806", "munpin", 1),
("S0002", "Hew Hao Hnam", "Cashier", "456, Jalan Muzaffar, Taman Muzaffar", "hew@gmail.com", "018-6326941", "hewhao", 1),
("S0003", "Aiman Sashimi", "Manager", "Kolej Kediaman Lestari B-G-58", "aiman@gmail.com", "018-24561822", "MANAGE", 1),
("S0004", "James Bone", "Secretary", "01/A, Taman Scientex Seksyen 2", "james@hotmail.com", "012-81405811", "jamesb", 1),
("S0005", "Amirul", "Supervisor", "Kolej Kediaman Satria Tuah", "amirul@outlook.com", "013-3498612", "amirul", 1),
("S0006", "Michael Jack", "Customer Support", "727, Jalan Cameron S2, Taman Cameron", "jack@hotmail.com", "018-8717194", "michae", 1),
("S0007", "Cyndi Wong", "Manager", "C320, Jalan Ayer Keroh, Taman Connaught", "cyndi@gmail.com", "012-98124685", "MANAGE", 1),
("S0008", "Ray Ong", "Mechanic", "R-24/A, Pangsapuri Megah, Taman Durian Tunggal", "rayong@outlook.com", "013-4617902", "rayong", 1),
("S0009", "Tung Sahuri", "Staff", "45, Jalan S2, Taman Bukit Katil", "tungtungsahur@outlook.com", "011-14175042", "tungsa", 1),
("S0010", "Yogesh Mohan", "Secretary", "77, Jalan Kerubong 3, Taman Kerubong Jaya", "yogesh@gmail.com", "011-0927952", "yogesh", 1);

insert into sale values
('Q0001', 'Card', '2025-11-15', 4549.40, 'S0005', 0.00),
('Q0002', 'Digital', '2025-11-19', 2229.70, 'S0006', 0.00),
('Q0003', 'Cash', '2025-11-26', 10849.50, 'S0003', 0.00),
('Q0004', 'Card', '2025-12-03', 9199.40, 'S0001', 0.00),
('Q0005', 'Digital', '2025-12-09', 3379.60, 'S0004', 0.00),
('Q0006', 'Cash', '2025-12-10', 6389.40, 'S0002', 0.00),
('Q0007', 'Card', '2025-12-12', 6499.60, 'S0002', 0.00),
('Q0008', 'Cash', '2025-12-18', 12649.30, 'S0006', 0.00),
('Q0009', 'Card', '2025-12-20', 9899.80, 'S0004', 0.00),
('Q0010', 'Digital', '2025-12-22', 6848.50, 'S0001', 0.00);

insert into sale_detail values
('P0001', 'Q0005', 1, 529.90, 0, 0),
('P0003', 'Q0002', 1, 829.90, 0, 0),
('P0004', 'Q0003', 2, 3399.80, 0, 0),
('P0004', 'Q0006', 1, 1699.90, 0, 0),
('P0004', 'Q0008', 1, 1699.90, 0, 0),
('P0005', 'Q0008', 1, 899.90, 0, 0),
('P0011', 'Q0004', 2, 1899.80, 0, 0),
('P0012', 'Q0008', 1, 4899.90, 0, 0),
('P0013', 'Q0010', 2, 1499.80, 0, 0),
('P0014', 'Q0006', 1, 1899.90, 0, 0),
('P0015', 'Q0009', 1, 6899.90, 0, 0),
('P0016', 'Q0008', 1, 499.90, 0, 0),
('P0020', 'Q0001', 5, 1949.50, 0, 0),
('P0021', 'Q0010', 10, 2799.00, 0, 0),
('P0023', 'Q0004', 1, 1599.90, 0, 0),
('P0023', 'Q0008', 2, 3199.80, 0, 0),
('P0025', 'Q0001', 1, 2599.90, 0, 0),
('P0025', 'Q0004', 2, 5199.80, 0, 0),
('P0027', 'Q0003', 1, 1449.90, 0, 0),
('P0027', 'Q0008', 1, 1449.90, 0, 0),
('P0029', 'Q0006', 3, 1589.70, 0, 0),
('P0030', 'Q0007', 1, 849.90, 0, 0),
('P0030', 'Q0010', 3, 2549.70, 0, 0),
('P0032', 'Q0005', 2, 1299.80, 0, 0),
('P0037', 'Q0005', 1, 1549.90, 0, 0),
('P0037', 'Q0007', 1, 1549.90, 0, 0),
('P0038', 'Q0003', 2, 5999.80, 0, 0),
('P0038', 'Q0007', 1, 2999.90, 0, 0),
('P0038', 'Q0009', 1, 2999.90, 0, 0),
('P0046', 'Q0004', 1, 499.90, 0, 0),
('P0048', 'Q0006', 1, 1199.90, 0, 0),
('P0049', 'Q0002', 2, 1399.80, 0, 0),
('P0050', 'Q0007', 1, 1099.90, 0, 0);

insert into order_stock values
('R0001', '2025-11-15', 'S0001', 'B0002'),
('R0002', '2025-11-28', 'S0006', 'B0010'),
('R0003', '2025-12-04', 'S0003', 'B0003'),
('R0004', '2025-12-12', 'S0003', 'B0008'),
('R0005', '2025-12-19', 'S0001', 'B0005'),
('R0006', '2025-12-24', 'S0004', 'B0009'),
('R0007', '2025-12-25', 'S0007', 'B0007'),
('R0008', '2025-12-25', 'S0007', 'B0006');

insert into order_detail values
('D0001', 5, 0, 1, 29499.50, 'R0001', 'P0015'),
('D0002', 3, 0, 1, 3297.00, 'R0002', 'P0008'),
('D0003', 5, 0, 1, 7495.00, 'R0002', 'P0009'),
('D0004', 5, 0, 1, 21499.50, 'R0002', 'P0012'),
('D0005', 30, 0, 1, 21897.00, 'R0003', 'P0033'),
('D0006', 5, 0, 1, 7499.50, 'R0004', 'P0004'),
('D0007', 10, 0, 1, 4999.00, 'R0005', 'P0001'),
('D0008', 10, 0, 1, 17999.00, 'R0005', 'P0002'),
('D0009', 15, 0, 1, 20998.50, 'R0005', 'P0026'),
('D0010', 10, 0, 1, 26999.00, 'R0006', 'P0024'),
('D0011', 10, 0, 1, 7999.00, 'R0006', 'P0011'),
('D0012', 15, 15, 0, 10498.50, 'R0007', 'P0003'),
('D0013', 20, 20, 0, 7998.00, 'R0008', 'P0016');

insert into invoice values
('V0001', '2025-11-22', 29499.50, 'B0002'),
('V0002', '2025-11-28', 7299.00, 'B0003'),
('V0003', '2025-12-01', 17499.00, 'B0009'),
('V0004', '2025-12-07', 10792.00, 'B0010'),
('V0005', '2025-12-13', 13998.50, 'B0005'),
('V0006', '2025-12-13', 21499.50, 'B0010'),
('V0007', '2025-12-16', 22998.50, 'B0005'),
('V0008', '2025-12-19', 14598.00, 'B0003'),
('V0009', '2025-12-22', 17499.00, 'B0009'),
('V0010', '2025-12-24', 7499.50, 'B0008'),
('V0011', '2025-12-24', 6999.50, 'B0005');

insert into inventory values
('L0001', 5, 'D0001', 'V0001'),
('L0002', 10, 'D0005', 'V0002'),
('L0003', 5, 'D0010', 'V0003'),
('L0004', 5, 'D0011', 'V0003'),
('L0005', 3, 'D0002', 'V0004'),
('L0006', 5, 'D0003', 'V0004'),
('L0007', 10, 'D0007', 'V0005'),
('L0008', 5, 'D0008', 'V0005'),
('L0009', 5, 'D0004', 'V0006'),
('L0010', 5, 'D0008', 'V0007'),
('L0011', 10, 'D0009', 'V0007'),
('L0012', 20, 'D0005', 'V0008'),
('L0013', 5, 'D0010', 'V0009'),
('L0014', 5, 'D0011', 'V0009'),
('L0015', 5, 'D0006', 'V0010'),
('L0016', 5, 'D0009', 'V0011');






