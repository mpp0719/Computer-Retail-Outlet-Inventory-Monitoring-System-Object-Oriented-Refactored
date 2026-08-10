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