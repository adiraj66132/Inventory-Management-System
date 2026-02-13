# Quick Start Guide

Get started with the Inventory Management System in 5 minutes!

## Installation (Arch Linux)

```bash
# 1. Install dependencies
sudo pacman -S gcc ncurses make

# 2. Build the application
make

# 3. Run it!
./inventory
```

## First Steps

### 1. Add Your First Item

1. Start the application: `./inventory`
2. Press **Enter** on "Add Item"
3. Fill in the details:
   ```
   Name: USB Cable
   Category: Electronics
   Quantity: 50
   Price: 5.99
   Low Stock Threshold: 10
   ```
4. Item is saved automatically!

### 2. View Your Inventory

1. Navigate to "View Items" with arrow keys
2. Press **Enter**
3. Use **← →** to navigate pages
4. Press **Q** to return to menu

### 3. Set Up Low Stock Alerts

1. Select "Set Low Stock Alert"
2. Enter the Item ID
3. Enter threshold (e.g., 10)
4. Items below threshold appear in **red** in the view

### 4. Import Sample Data

```bash
# Use the provided sample file
./inventory
# Select "Import from CSV"
# Enter: sample_inventory.csv
```

Now you have 20 items to explore!

## Essential Commands

| Action | Steps |
|--------|-------|
| **Add Item** | Menu → Add Item → Fill details |
| **Search** | Menu → Search → Type name |
| **View Category** | Menu → View by Category → Type category |
| **Sort** | Menu → Sort & View → Choose option |
| **Export** | Menu → Export to CSV → Enter filename |
| **Backup** | Menu → Backup Data → Auto-timestamped |

## Tips for New Users

1. **Start Small**: Add 5-10 items to get familiar
2. **Use Categories**: Group similar items together
3. **Set Thresholds**: Enable alerts for critical items
4. **Export Often**: Keep CSV backups
5. **Check Audit Log**: Review what you've changed

## Common Keyboard Shortcuts

- **↑ ↓**: Navigate menu
- **Enter**: Select option
- **← →**: Navigate pages (in View mode)
- **Q**: Return to menu / Exit view
- **E**: Exit application

## Your First Workflow

Here's a typical workflow for setting up your inventory:

```bash
# 1. Start the application
./inventory

# 2. Import sample data or add items manually
#    Select: Import from CSV → sample_inventory.csv

# 3. View the inventory
#    Select: View Items

# 4. Set up categories for organization
#    Select: View by Category → Electronics

# 5. Configure low stock alerts
#    Select: Set Low Stock Alert
#    Enter ID: 1, Threshold: 5

# 6. Export for backup
#    Select: Export to CSV → my_inventory.csv

# 7. Check the audit trail
#    Select: View Audit Log
```

## Sample Categories

Here are some suggested categories to organize your inventory:

- **Electronics**: Computers, cables, devices
- **Office Supplies**: Pens, paper, staplers
- **Furniture**: Desks, chairs, shelves
- **Hardware**: Tools, components, parts
- **Consumables**: Items that need regular restocking
- **Equipment**: Larger items, machinery

## Next Steps

After mastering the basics:

1. **Read the full README** for advanced features
2. **Set up regular backups** using the Backup feature
3. **Explore sorting options** to analyze inventory
4. **Create custom CSV imports** for bulk updates
5. **Review the audit log** to track changes

## Troubleshooting Quick Fixes

**Application won't start?**
```bash
export TERM=xterm-256color
./inventory
```

**Can't see colors?**
```bash
echo $TERM  # Should show xterm-256color or similar
```

**File errors?**
```bash
# Make sure you're in a writable directory
cd ~
./path/to/inventory
```

## Need Help?

- Full documentation: `README.md`
- Troubleshooting: `TROUBLESHOOTING.md`
- Examples: `sample_inventory.csv`

Happy inventory managing! 🎉
