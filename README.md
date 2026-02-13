# Inventory Management System

A powerful terminal-based inventory management application built with C and ncurses. Features include CSV import/export, category management, low-stock alerts, sorting, pagination, and comprehensive audit logging.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![Language](https://img.shields.io/badge/language-C-green.svg)

## Features

### Core Functionality
- ✅ **Add, Update, Delete** inventory items
- 🔍 **Search** by name with highlighting
- 📊 **View** inventory with pagination (15 items per page)
- 🏷️ **Categories** for organizing items
- 📈 **Sorting** by name, quantity, price, or category

### Advanced Features
- 📥 **CSV Import/Export** - Share inventory data with spreadsheets
- ⚠️ **Low Stock Alerts** - Visual warnings for items below threshold
- 📝 **Audit Logging** - Track all changes with user/timestamp
- 💾 **Backup/Restore** - Timestamped backups with easy restore
- 🔢 **Auto-incrementing IDs** - No manual ID management needed

### User Interface
- ⌨️ **Keyboard Navigation** - Arrow keys and intuitive controls
- 🎨 **Color-coded Alerts** - Low stock items highlighted in red
- 📄 **Paginated Views** - Navigate large inventories easily
- 🖥️ **Terminal-based** - Works over SSH, lightweight and fast

## Screenshots

```
╔════════════════════════════════════╗
║   INVENTORY MANAGEMENT SYSTEM      ║
╚════════════════════════════════════╝

    1. Add Item
    2. View Items
    3. Update Item
    4. Delete Item
    5. Search Item
    6. Export to CSV
    7. Import from CSV
    8. View by Category
    9. Sort & View
    A. Set Low Stock Alert
    B. Backup Data
    C. Restore Data
    D. View Audit Log
    E. Exit

Use ↑↓ to navigate, Enter to select
```

## Installation

### Arch Linux

#### Quick Install (Recommended)

```bash
# Install dependencies
sudo pacman -S gcc ncurses make

# Clone the repository
git clone https://github.com/yourusername/inventory-system.git
cd inventory-system

# Build and install
make
sudo make install

# Run the application
inventory
```

#### Building a Package with makepkg

For Arch users who prefer using `makepkg`:

```bash
# Navigate to the packaging directory
cd packaging/arch

# Build and install the package
makepkg -si

# The package will be installed system-wide
# Run with: inventory
```

#### Manual Build (Without make)

```bash
# Install dependencies
sudo pacman -S gcc ncurses

# Compile directly
gcc -Wall -Wextra -O2 -o inventory inventory.c -lncurses

# Run
./inventory
```

### Debian/Ubuntu

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install gcc libncurses-dev make

# Build and install
make
sudo make install

# Run
inventory
```

### Other Linux Distributions

Most distributions include gcc and ncurses in their repositories:

- **Fedora/RHEL**: `sudo dnf install gcc ncurses-devel make`
- **openSUSE**: `sudo zypper install gcc ncurses-devel make`
- **Gentoo**: `sudo emerge sys-devel/gcc sys-libs/ncurses`

Then follow the standard build process:

```bash
make
sudo make install
```

## Usage

### Starting the Application

```bash
./inventory    # If running from the build directory
# OR
inventory      # If installed system-wide
```

### Basic Operations

#### Adding Items
1. Select "Add Item" from the menu
2. Enter item details:
   - Name (e.g., "USB Cable")
   - Category (e.g., "Electronics")
   - Quantity (e.g., 50)
   - Price (e.g., 5.99)
   - Low stock threshold (e.g., 10)

#### Viewing Inventory
- Use arrow keys (← →) to navigate pages
- Low stock items appear in **bold red**
- Press 'Q' to return to menu

#### Searching
- Enter keywords to find items
- Search is case-insensitive
- Results are highlighted

#### CSV Import/Export

**Exporting:**
```bash
# From the menu, select "Export to CSV"
# Enter filename: inventory_export.csv
```

**Importing:**
```bash
# Prepare a CSV file with this format:
# ID,Name,Category,Quantity,Price,LowStockThreshold
# 1,"Laptop","Electronics",5,899.99,2

# From the menu, select "Import from CSV"
# Enter the filename
```

#### Backup and Restore

**Creating a backup:**
```bash
# Select "Backup Data" from menu
# Automatic timestamp: inventory_backup_20260204_143022.dat
```

**Restoring from backup:**
```bash
# Select "Restore Data"
# Enter the backup filename
```

## File Structure

```
inventory-system/
├── inventory.c              # Main source code
├── Makefile                 # Build automation
├── README.md                # This file
├── LICENSE                  # MIT License
├── packaging/
│   └── arch/
│       └── PKGBUILD         # Arch Linux package
├── docs/
│   ├── TROUBLESHOOTING.md   # Common issues and solutions
│   └── DEVELOPMENT.md       # Development guide
└── examples/
    ├── sample_inventory.csv # Sample data
    └── import_template.csv  # CSV import template
```

## Data Files

The application creates these files in the current directory:

- `inventory.dat` - Binary inventory database
- `audit.log` - Text log of all operations
- `config.dat` - Configuration (auto-increment counter)
- `*.csv` - Exported CSV files
- `inventory_backup_*.dat` - Backup files with timestamps

## Troubleshooting

### Common Issues on Arch Linux

#### 1. "ncurses.h not found" Error

```bash
# Ensure ncurses is installed
sudo pacman -S ncurses

# If still failing, check for pkg-config
sudo pacman -S pkg-config
```

#### 2. Linking Errors with ncurses

```bash
# Try compiling with explicit library path
gcc -o inventory inventory.c -lncurses -I/usr/include -L/usr/lib
```

#### 3. Permission Denied When Running

```bash
# Make the binary executable
chmod +x inventory
```

#### 4. Terminal Display Issues

```bash
# Set the TERM environment variable
export TERM=xterm-256color

# Or for tmux users
export TERM=screen-256color
```

### General Troubleshooting

See [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) for detailed solutions.

## Building Variations

### Debug Build
```bash
make debug
# Includes debugging symbols and debug output
```

### Static Binary
```bash
make static
# Creates a statically-linked binary (no runtime dependencies)
```

### Clean Build
```bash
make clean      # Remove built files
make            # Fresh build
```

## Development

### Prerequisites for Development
- GCC 7.0 or later
- ncurses 6.0 or later
- make (optional but recommended)
- git (for version control)

### Code Style
- K&R indentation style
- 4 spaces per indent level
- Maximum line length: 100 characters
- Comments for complex logic

### Contributing
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Advanced Usage

### Automation and Scripting

While the application is interactive, you can automate backups:

```bash
#!/bin/bash
# Automated backup script
cd /path/to/inventory
# Trigger backup via menu automation could be added
# For now, files can be backed up with:
cp inventory.dat "backup_$(date +%Y%m%d).dat"
```

### Integration with Other Tools

Export to CSV and use with:
- **LibreOffice Calc** - Open and edit inventory
- **Python pandas** - Analyze inventory data
- **R** - Statistical analysis
- **PostgreSQL** - Import into database

```python
# Example: Python integration
import pandas as pd

# Read exported CSV
df = pd.read_csv('inventory_export.csv')

# Analyze low stock items
low_stock = df[df['Quantity'] <= df['LowStockThreshold']]
print(low_stock)
```

## Performance

- **Startup**: < 100ms
- **Search**: O(n) linear scan (fast for < 10,000 items)
- **Sort**: O(n²) bubble sort (adequate for < 1,000 items)
- **Memory**: ~200KB base + (100 bytes × items)
- **Disk**: ~100 bytes per item

## Security Considerations

- Files are stored in the current working directory
- No encryption of data files (use filesystem encryption if needed)
- Audit log tracks username via `getlogin()`
- Suitable for single-user or trusted multi-user environments
- For production use, consider adding:
  - File permissions management
  - Data encryption
  - User authentication
  - Network isolation

## Roadmap

### Planned Features
- [ ] Multi-user support with authentication
- [ ] Database backend (SQLite)
- [ ] Advanced reporting and analytics
- [ ] Barcode scanning support
- [ ] REST API interface
- [ ] Web dashboard
- [ ] Email notifications for low stock
- [ ] Receipt/invoice generation

### Version History
- **v2.0.0** (2026-02-04)
  - Added CSV import/export
  - Category management
  - Low stock alerts with visual indicators
  - Sorting and filtering
  - Pagination for large inventories
  - Comprehensive audit logging
  - Backup/restore functionality
  - Auto-incrementing IDs

- **v1.0.0** (Initial)
  - Basic CRUD operations
  - Search functionality
  - Simple ncurses interface

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [ncurses](https://invisible-island.net/ncurses/) library
- Inspired by classic terminal-based inventory systems
- Thanks to the Arch Linux community for packaging guidelines

## Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/inventory-system/issues)
- **Documentation**: [docs/](docs/)
- **Email**: your.email@example.com

## FAQ

**Q: Can I use this for commercial purposes?**  
A: Yes, the MIT license allows commercial use.

**Q: Does it support Windows?**  
A: Currently Linux-only. Windows support via WSL or Cygwin may work but is untested.

**Q: How do I migrate data to a database?**  
A: Export to CSV, then import into your database. Example SQL import scripts in `docs/`.

**Q: Can multiple users access the same inventory?**  
A: Currently designed for single-user. File locking is not implemented.

**Q: How do I customize colors?**  
A: Edit the `init_pair()` calls in `main()` function.

---

**Made with ❤️ for terminal enthusiasts**
