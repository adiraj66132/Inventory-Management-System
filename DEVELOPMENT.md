# Development Guide

This guide is for developers who want to contribute to or extend the Inventory Management System.

## Table of Contents
- [Development Setup](#development-setup)
- [Code Structure](#code-structure)
- [Building and Testing](#building-and-testing)
- [Adding New Features](#adding-new-features)
- [Code Style](#code-style)
- [Contributing](#contributing)

## Development Setup

### Prerequisites

**Arch Linux:**
```bash
sudo pacman -S gcc ncurses make git gdb valgrind
```

**Debian/Ubuntu:**
```bash
sudo apt-get install build-essential libncurses-dev git gdb valgrind
```

### Getting the Source

```bash
git clone https://github.com/yourusername/inventory-system.git
cd inventory-system
```

### Development Build

```bash
# Debug build with symbols
make debug

# Run with debugger
gdb ./inventory
```

## Code Structure

### Main Components

```
inventory.c
├── Structs
│   ├── Item          - Main inventory item
│   └── Config        - Application config (ID counter)
├── Core Functions
│   ├── addItem()     - Add new item
│   ├── viewItems()   - Display inventory (paginated)
│   ├── updateItem()  - Modify existing item
│   ├── deleteItem()  - Remove item
│   └── searchItem()  - Search by name
├── Advanced Features
│   ├── exportToCSV() - Export data
│   ├── importFromCSV() - Import data
│   ├── sortAndView() - Sort and display
│   ├── viewByCategory() - Filter by category
│   ├── backupData()  - Create backup
│   ├── restoreData() - Restore from backup
│   └── viewAuditLog() - Show change history
├── Utilities
│   ├── getNextID()   - Auto-increment ID
│   ├── incrementID() - Increase ID counter
│   └── logAudit()    - Log changes
└── UI
    ├── menu()        - Main menu loop
    └── main()        - Init ncurses
```

### Data Structures

#### Item Structure
```c
struct Item {
    int id;                    // Unique identifier (auto-increment)
    char name[50];             // Item name
    int quantity;              // Stock quantity
    float price;               // Unit price
    char category[30];         // Category for grouping
    int low_stock_threshold;   // Alert threshold (0 = disabled)
};
```

**Size**: ~100 bytes per item

#### Config Structure
```c
struct Config {
    int next_id;      // Next auto-increment ID
    int items_count;  // Total items (currently unused)
};
```

### File Formats

#### inventory.dat
Binary file containing array of `struct Item`:
```
[Item 1][Item 2][Item 3]...
```

#### config.dat
Binary file containing `struct Config`:
```
[Config]
```

#### audit.log
Text file with format:
```
[Timestamp] User: username | Action: ACTION | Item ID: N | Details
```

#### CSV Format
```csv
ID,Name,Category,Quantity,Price,LowStockThreshold
1,"Item Name","Category",10,9.99,5
```

## Building and Testing

### Build Commands

```bash
# Standard build
make

# Debug build
make debug

# Static binary (no runtime dependencies)
make static

# Clean build artifacts
make clean

# Clean all data (CAUTION!)
make clean-data
```

### Testing

#### Manual Testing
```bash
# Build
make

# Run
./inventory

# Test each feature:
# 1. Add items
# 2. View with pagination
# 3. Search
# 4. Update
# 5. Delete
# 6. Export CSV
# 7. Import CSV
# 8. Categories
# 9. Sorting
# 10. Low stock alerts
# 11. Backup/restore
# 12. Audit log
```

#### Memory Testing
```bash
# Build with debug symbols
make debug

# Test with valgrind
valgrind --leak-check=full --show-leak-kinds=all ./inventory

# Look for:
# - Memory leaks
# - Invalid memory access
# - Uninitialized values
```

#### Stress Testing
```bash
# Import large dataset
# Create a CSV with 1000+ items
python3 << 'EOF'
with open('large_test.csv', 'w') as f:
    f.write('ID,Name,Category,Quantity,Price,LowStockThreshold\n')
    for i in range(1, 1001):
        f.write(f'{i},"Test Item {i}","Category {i%10}",{i%100},{i*0.99},{i%20}\n')
EOF

# Import it
./inventory
# Select: Import from CSV → large_test.csv

# Test pagination, search, sorting with large dataset
```

## Adding New Features

### Example: Adding a New Field

Let's add a "Supplier" field to items.

**Step 1: Update the Item structure**
```c
struct Item {
    int id;
    char name[50];
    int quantity;
    float price;
    char category[30];
    int low_stock_threshold;
    char supplier[50];  // NEW FIELD
};
```

**Step 2: Update addItem()**
```c
void addItem() {
    // ... existing code ...
    
    mvprintw(10, 2, "Enter Supplier: ");
    getnstr(item.supplier, sizeof(item.supplier) - 1);
    
    // ... rest of function ...
}
```

**Step 3: Update viewItems()**
```c
mvprintw(3, 2, "ID   Name            Category    Supplier    Qty  Price");
mvprintw(row++, 2, "%-4d %-15s %-11s %-11s %-4d $%.2f",
         item.id, item.name, item.category, item.supplier, 
         item.quantity, item.price);
```

**Step 4: Update CSV export/import**
```c
// Export
fprintf(csv, "ID,Name,Category,Supplier,Quantity,Price,LowStockThreshold\n");
fprintf(csv, "%d,\"%s\",\"%s\",\"%s\",%d,%.2f,%d\n",
        item.id, item.name, item.category, item.supplier,
        item.quantity, item.price, item.low_stock_threshold);

// Import
sscanf(line, "%d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",%d,%f,%d",
       &item.id, name_buf, cat_buf, supp_buf,
       &item.quantity, &item.price, &item.low_stock_threshold);
```

**Step 5: Update other functions** (updateItem, searchItem, etc.)

**Step 6: Test thoroughly**

### Example: Adding a New Menu Option

Let's add a "Statistics" feature.

**Step 1: Create the function**
```c
void showStatistics() {
    struct Item item;
    FILE *fp = fopen(FILE_NAME, "rb");
    
    int total_items = 0;
    float total_value = 0;
    int low_stock_count = 0;
    
    while (fread(&item, sizeof(item), 1, fp)) {
        total_items++;
        total_value += item.quantity * item.price;
        if (item.low_stock_threshold > 0 && 
            item.quantity <= item.low_stock_threshold) {
            low_stock_count++;
        }
    }
    
    fclose(fp);
    
    clear();
    mvprintw(2, 2, "=== Inventory Statistics ===");
    mvprintw(4, 2, "Total Items: %d", total_items);
    mvprintw(5, 2, "Total Value: $%.2f", total_value);
    mvprintw(6, 2, "Low Stock Items: %d", low_stock_count);
    mvprintw(8, 2, "Press any key to return...");
    getch();
}
```

**Step 2: Add to menu**
```c
void menu() {
    char *options[] = {
        // ... existing options ...
        "F. Statistics",  // NEW OPTION
        "G. Exit"
    };
    int num_options = 16;  // Update count
    
    // ... in switch statement ...
    case 14: showStatistics(); break;
    case 15: endwin(); exit(0);
}
```

## Code Style

### Formatting Rules

```c
// 1. Indentation: 4 spaces (no tabs)
void function() {
    if (condition) {
        do_something();
    }
}

// 2. Braces: K&R style
if (condition) {
    // code
} else {
    // code
}

// 3. Line length: Max 100 characters
// Break long lines appropriately

// 4. Variable naming: lowercase_with_underscores
int item_count;
char user_name[50];

// 5. Function naming: camelCase or lowercase_with_underscores
void addItem();
void get_next_id();

// 6. Constants: UPPERCASE_WITH_UNDERSCORES
#define MAX_ITEMS 1000
#define FILE_NAME "inventory.dat"
```

### Comments

```c
// Good: Explain WHY, not WHAT
// Calculate total value to warn user before deletion
float total = item.quantity * item.price;

// Bad: Obvious comment
// Set x to 5
int x = 5;

// Good: Function documentation
/**
 * Export inventory to CSV file
 * Creates a CSV file with all items in the database
 * Logs the export operation to audit trail
 */
void exportToCSV();
```

### Error Handling

```c
// Always check file operations
FILE *fp = fopen(FILE_NAME, "rb");
if (!fp) {
    mvprintw(10, 5, "Error opening file!");
    getch();
    return;
}

// Check user input
if (scanw("%d", &id) != 1) {
    mvprintw(10, 5, "Invalid input!");
    getch();
    return;
}

// Close resources
fclose(fp);
```

## Performance Considerations

### Current Limitations

1. **Linear search**: O(n) for all operations
2. **File I/O**: Reads entire file for most operations
3. **Sorting**: O(n²) bubble sort
4. **Memory**: Loads all items for sorting

### Optimization Ideas

1. **Indexing**: Create index file for faster lookups
2. **Caching**: Keep frequently accessed items in memory
3. **Better algorithms**: Use quicksort (O(n log n))
4. **Database**: Consider SQLite for larger datasets

### Memory Management

```c
// Current: Stack allocation
struct Item item;

// For large datasets: Heap allocation
struct Item *items = malloc(count * sizeof(struct Item));
if (!items) {
    // Handle allocation failure
}
// ... use items ...
free(items);
```

## Debugging Tips

### Common Issues

**Segmentation Fault**
```bash
# Run with gdb
gdb ./inventory
(gdb) run
# When it crashes:
(gdb) backtrace
(gdb) print variable_name
```

**File Corruption**
```bash
# Check file structure
hexdump -C inventory.dat | less

# Validate each struct
python3 << 'EOF'
import struct
with open('inventory.dat', 'rb') as f:
    while True:
        data = f.read(100)  # struct size
        if not data:
            break
        print(data)
EOF
```

**ncurses Issues**
```c
// Always cleanup
void safe_exit() {
    endwin();
    exit(0);
}

// Use in signal handlers
signal(SIGINT, safe_exit);
```

## Contributing

### Workflow

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feature/my-feature`
3. **Code** following style guidelines
4. **Test** thoroughly
5. **Commit** with descriptive messages
6. **Push** to your fork
7. **Submit** a pull request

### Commit Messages

```bash
# Good commit messages
git commit -m "Add supplier field to Item structure"
git commit -m "Fix memory leak in CSV import"
git commit -m "Improve pagination performance"

# Bad commit messages
git commit -m "fixed stuff"
git commit -m "updates"
```

### Pull Request Checklist

- [ ] Code follows style guidelines
- [ ] All functions are documented
- [ ] No compiler warnings
- [ ] Tested with sample data
- [ ] Tested with large dataset (1000+ items)
- [ ] Memory leaks checked with valgrind
- [ ] Updated README if needed
- [ ] Added to CHANGELOG

## Future Improvements

### Planned Features
- SQLite database backend
- Multi-user support
- Network synchronization
- REST API
- Web interface
- Mobile app
- Barcode scanning

### Architecture Changes
- Separate UI from business logic
- Add unit tests
- Configuration file support
- Plugin system
- Internationalization

---

**Happy Coding!** 🚀
