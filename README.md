# Computer Retail Outlet Inventory Monitoring System (OOP)

A terminal-based Retail Outlet & Inventory Monitoring System written in C++ and backed by MySQL. Handles point-of-sale (POS) operations, inventory tracking, staff management, supplier tracking, purchase orders, and sales analytics inside a Windows console environment.

This is the **object-oriented refactor** of the original monolithic/procedural implementation. Same features and business logic, restructured into classes with clear domain separation instead of one large file of free functions operating on global state.

---

## Features

**1. Staff Management**
- Auto-generated staff IDs (`S001`, `S002`, etc.) with role-based access (Staff vs. Manager)
- CRUD operations (Register, View, Update, Deactivate, Search) with page-based navigation

**2. Product & Inventory Management**
- Product tracking with base price, selling price, quantity, type, and active status
- Search and filter products by price range or sort by stock levels/prices
- Low-stock auto-notifications and product performance analysis

**3. Point of Sale (POS) & Cart**
- Multi-item cart management with stock check validations
- Receipt generation (`receipt_<saleId>.txt`) supporting Cash, Card, and QR payment methods
- Sales refund management and cart item updates

**4. Supplier & Stock Ordering**
- Supplier profile management
- Stock purchase order creation, stock receiving, and inventory adjustment
- Invoice generation (`invoice_<invoiceId>.txt`) and local file logging

**5. Analytics & Reporting**
- Monthly sales and monthly stock order bar-chart reports
- Product-level popularity analysis (highest/lowest sellers per year)

---

## Architecture

Every original function operated on one ERD table (or a small cluster of them) via a `sql::Connection*` passed as its first argument — that's the class boundary this refactor follows:

| Class | Responsibility |
|---|---|
| `Database` | Singleton owning the single `sql::Connection*` |
| `Session` | Currently logged-in staff (replaces a global) |
| `Utils` | Input validation, console setup, misc helpers |
| `StaffService` | Staff CRUD + login (DFD 1.0, 2.0) |
| `ProductService` | Product CRUD, search/sort (DFD 3.0) |
| `SaleService` + `Cart` | POS sales, refunds, cart management (DFD 4.0) |
| `SupplierService` | Supplier CRUD |
| `StockOrderService` | Purchase orders, order refunds (DFD 5.0) |
| `InventoryService` | Stock receiving, invoices, low-stock alerts (DFD 6.0) |
| `ReportService` | Receipts/invoices, analytics reports (DFD 7.0) |
| `Application` | Top-level menu loop, wires all services together |

Each service pulls its DB connection from `Database::instance()` rather than taking it as a parameter, and entity structs (`Staff`, `Product`, `Sale`, ...) hold one row's worth of data with no behavior.

---

## Tech Stack & Requirements

- **Language**: C++ (C++14 or higher)
- **Compiler**: MSVC via Visual Studio (uses `Windows.h`, `conio.h`, and Windows console virtual terminal setup — Windows only)
- **Database**: MySQL Server 8.0
- **Libraries**:
  - [MySQL Connector/C++ 8.0](https://dev.mysql.com/downloads/connector/cpp/) (legacy `cppconn`/JDBC-style interface)
  - Custom `ansi.h` for terminal color output (not included in this repo — add your own copy to the project)

---

## Project Structure

```text
.
├── Core
│   ├── Database.h
│   ├── Database.cpp
│   ├── Utils.h 
│   ├── Utils.cpp
│   ├── Application.h
│   └── Application.cpp
├── Staff
│   ├── Staff.h / Staff.cpp
│   ├── Staff.cpp
│   ├── Staffservice.h
│   └── Staffservice.cpp
├── Product
│   ├── Product.h
│   └── Product.cpp
├── Sale
│   ├── Sale.h
│   ├── Sale.cpp
│   ├── Cart.h
│   └── Cart.cpp
├── Supplier
│   ├── Supplier.h
│   └── Supplier.cpp
├── Stockorder
│   ├── Stockorder.h
│   └── Stockorder.cpp
├── Inventory
│   ├── Inventory.h 
│   └── Inventory.cpp
├── Report
│   ├── Reportservice.h
│   └── Reportservice.cpp
├── External
│   └── ansi.h
├── main.cpp
├── README.md
├── Maximum Data for Reporting.sql
├── Minimal Data for demonstration.sql
├── DDL only.sql
└── ansi.h
```

---

## Database Setup

1. Make sure **MySQL Server** is running locally on port `3306`.
2. Default connection credentials (set in `Application::startup()`, `src/Application.cpp`):
   - **Host**: `tcp://127.0.0.1:3306`
   - **User**: `root`
   - **Password**: `1234`
   - **Schema**: `project1` (set in `Database::connect()`, `src/Database.cpp`)

   Update these in both files if your local setup differs.
3. Run either one of the three SQL script provided based on you own need (The data might be different due to market prices).

---

## How to Build and Run (Visual Studio)

This project depends on Windows-only headers and MSVC-specific APIs, so it's built through Visual Studio rather than g++/MinGW.

1. **Create a new empty C++ Console App project** in Visual Studio.
2. **Add all files** from `include/` and `src/` to the project (right-click the project → Add → Existing Item), plus your own copy of `ansi.h`.
3. **Point the project at your MySQL Connector/C++ install:**
   - *Project Properties → VC++ Directories* (or *C/C++ → General → Additional Include Directories*):
     ```
     C:\Program Files\MySQL\mysql-connector-c++-8.0.27-winx64\include\jdbc
     C:\Program Files\MySQL\mysql-connector-c++-8.0.27-winx64\include\mysql
     C:\Program Files\MySQL\MySQL Server 8.0\include\mysql
     $(ProjectDir)
     ```
   - *Linker → General → Additional Library Directories:*
     ```
     C:\Program Files\MySQL\mysql-connector-c++-8.0.27-winx64\lib64\vs14
     C:\Program Files\MySQL\MySQL Server 8.0\lib
     ```
   - *Linker → Input → Additional Dependencies:*
     ```
     mysqlcppconn.lib;libmysql.lib
     ```
   (Adjust paths/version numbers to match your actual Connector/C++ install.)
4. **Make the connector DLLs available at runtime** — copy `mysqlcppconn*.dll`, `libmysql.dll`, and any bundled OpenSSL DLLs (`libssl-*.dll`, `libcrypto-*.dll`) from the connector's `lib64\vs14` folder into your build output folder (e.g. `x64\Debug\`), or add that folder to your system `PATH`.
5. **Set configuration to x64** (Configuration Manager → Platform → x64), matching your MySQL Connector/C++ binaries.
6. **Build** (Build → Build Solution), then **Run with Ctrl+F5** (not F5) so the console window stays open if something fails — useful for reading connection errors.

---

## Known Behavior Notes

A few quirks carried over unchanged from the original implementation (not introduced by the refactor):

- `ProductService::searchForSelection()` only returns the last displayed page of search results, not the full match set — "Product No." selection only lines up correctly within that page.
- `SupplierService::deactivate()`'s delete query references a non-existent column, so it always falls through to deactivating rather than deleting a supplier.
- `InventoryService::createInventoryRecord()` generates a receipt ID with a lowercase `l0` instead of `L0` once the running count crosses two digits.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
