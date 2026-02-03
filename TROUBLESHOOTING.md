# Troubleshooting Guide

This document covers common issues and their solutions for the Inventory Management System.

## Table of Contents
- [Compilation Issues](#compilation-issues)
- [Runtime Issues](#runtime-issues)
- [Display Problems](#display-problems)
- [Data Issues](#data-issues)
- [Arch Linux Specific](#arch-linux-specific)

## Compilation Issues

### Issue: "ncurses.h: No such file or directory"

**Cause**: ncurses development headers not installed.

**Solution (Arch Linux)**:
```bash
sudo pacman -S ncurses
# If still failing, try:
sudo pacman -S ncurses base-devel
```

**Solution (Debian/Ubuntu)**:
```bash
sudo apt-get install libncurses-dev
```

**Solution (Fedora)**:
```bash
sudo dnf install ncurses-devel
```

### Issue: "undefined reference to 'initscr'"

**Cause**: Not linking against ncurses library.

**Solution**:
```bash
# Ensure you're using the -lncurses flag
gcc -o inventory inventory.c -lncurses

# Or use the Makefile
make clean && make
```

### Issue: Compilation warnings about implicit declarations

**Cause**: Using non-standard functions without proper includes.

**Solution**:
Add these to the top of inventory.c if missing:
```c
#define _GNU_SOURCE
#include <strings.h>  // For strcasestr
```

### Issue: "fatal error: stropts.h: No such file or directory"

**Cause**: stropts.h is deprecated and removed in newer glibc versions.

**Solution**: This file isn't actually needed for this project. If you see this error, it may indicate a modified version of the code.

## Runtime Issues

### Issue: Segmentation fault on startup

**Possible Causes and Solutions**:

1. **Corrupted data file**:
```bash
# Backup and remove the data file
mv inventory.dat inventory.dat.backup
# Restart the application
./inventory
```

2. **Terminal size too small**:
```bash
# Resize terminal to at least 80x24
resize -s 24 80
```

3. **Incompatible terminal emulator**:
```bash
# Try a different terminal or set TERM variable
export TERM=xterm-256color
./inventory
```

### Issue: Cannot write to files (Permission denied)

**Cause**: Running from a directory without write permissions.

**Solution**:
```bash
# Run from home directory or create a dedicated directory
mkdir ~/inventory-data
cd ~/inventory-data
inventory  # If installed system-wide
# OR
~/path/to/inventory  # If running locally
```

### Issue: "Error opening file!" when trying to view items

**Cause**: No inventory.dat file exists yet.

**Solution**: This is normal for first run. Add some items first, then view them.

## Display Problems

### Issue: Colors not showing correctly

**Cause**: Terminal doesn't support colors or TERM variable incorrect.

**Solutions**:

1. **Check color support**:
```bash
echo $TERM
# Should show something like: xterm-256color or screen-256color
```

2. **Set TERM variable**:
```bash
# For most terminals
export TERM=xterm-256color

# For tmux users
export TERM=screen-256color

# For older systems
export TERM=xterm
```

3. **Add to shell config** (~/.bashrc or ~/.zshrc):
```bash
export TERM=xterm-256color
```

### Issue: Display appears garbled or corrupted

**Cause**: Terminal resize during operation or incompatible terminal.

**Solutions**:

1. **Press Ctrl+L** to redraw screen
2. **Restart application**:
```bash
# Exit with 'E' option or Ctrl+C
# Restart
./inventory
```

3. **Check terminal compatibility**:
```bash
# Test ncurses directly
tput colors  # Should output 8 or 256
```

### Issue: Arrow keys produce characters like ^[[A

**Cause**: Terminal not in proper mode or TERM variable incorrect.

**Solution**:
```bash
# Reset terminal
reset

# Set proper TERM
export TERM=xterm

# Restart application
./inventory
```

### Issue: Text overlaps or wraps incorrectly

**Cause**: Terminal window too narrow.

**Solution**: Resize terminal to at least 80 columns wide:
```bash
# Check current size
tput cols
tput lines

# Should be at least 80 columns × 24 lines
```

## Data Issues

### Issue: Lost all inventory data

**Prevention**:
```bash
# Always backup before major operations
# From the application, use "Backup Data" option
# Or manually:
cp inventory.dat inventory.dat.backup
cp audit.log audit.log.backup
cp config.dat config.dat.backup
```

**Recovery**:
```bash
# If backup exists
cp inventory.dat.backup inventory.dat

# If using git
git checkout inventory.dat  # Restore from last commit

# If no backup exists, data may be unrecoverable
# Check for auto-backups:
ls -la inventory_backup_*.dat
```

### Issue: Duplicate IDs after import

**Cause**: Imported CSV has conflicting IDs with existing items.

**Solution**:
1. Export current inventory
2. Clean the database: `rm inventory.dat config.dat`
3. Import the merged/cleaned CSV
4. Or edit CSV to use higher ID numbers before import

### Issue: CSV import fails silently

**Causes and Solutions**:

1. **Wrong CSV format**:
```csv
# Correct format (with header):
ID,Name,Category,Quantity,Price,LowStockThreshold
1,"USB Cable","Electronics",50,5.99,10
2,"Notebook","Office",100,2.50,20
```

2. **Encoding issues**:
```bash
# Convert to UTF-8
iconv -f ISO-8859-1 -t UTF-8 input.csv > output.csv
```

3. **Line ending issues**:
```bash
# Convert Windows line endings to Unix
dos2unix inventory.csv
# Or
sed -i 's/\r$//' inventory.csv
```

### Issue: Audit log growing too large

**Solution**:
```bash
# Archive old logs
mv audit.log audit.log.$(date +%Y%m%d)
gzip audit.log.*

# Or trim to last 1000 lines
tail -n 1000 audit.log > audit.log.tmp
mv audit.log.tmp audit.log
```

## Arch Linux Specific

### Issue: Package installation fails with makepkg

**Cause**: Missing dependencies or incorrect PKGBUILD.

**Solution**:
```bash
# Install base development tools
sudo pacman -S base-devel

# Update package database
sudo pacman -Sy

# Try again
makepkg -si
```

### Issue: "ERROR: One or more files did not pass the validity check!"

**Cause**: Checksum mismatch in PKGBUILD.

**Solution**:
```bash
# Update checksums
updpkgsums

# Or skip checksum verification (development only)
makepkg -si --skipinteg
```

### Issue: Binary not in PATH after installation

**Cause**: /usr/local/bin not in PATH (if using `make install`).

**Solution**:
```bash
# Check PATH
echo $PATH

# Add to PATH if needed (add to ~/.bashrc or ~/.zshrc)
export PATH="/usr/local/bin:$PATH"

# Reload shell config
source ~/.bashrc
```

### Issue: Conflicts with existing package

**Cause**: Another package provides a binary named "inventory".

**Solution**:
```bash
# Check what provides it
pacman -Qo /usr/bin/inventory

# Rename this binary during installation
sudo make install PREFIX=/usr/local/inventory-system

# Then run with full path or create alias
alias inventory='/usr/local/inventory-system/bin/inventory'
```

### Issue: Library version mismatch

**Cause**: Built with different ncurses version than installed.

**Solution**:
```bash
# Check ncurses version
pacman -Q ncurses

# Rebuild from source
make clean
make

# Or rebuild package
makepkg -f
```

## Performance Issues

### Issue: Slow with large inventories (>1000 items)

**Cause**: Linear search and bubble sort algorithms.

**Solutions**:
1. **Use search/filter** instead of viewing all items
2. **Export to CSV** and use spreadsheet software for analysis
3. **Consider database version** (future feature)

**Workaround**:
```bash
# Split inventory into multiple files by category
# Use separate inventory.dat files in different directories
mkdir electronics office hardware
cd electronics && inventory  # Separate instance
```

## Debug Mode

To get more information about issues:

```bash
# Compile with debug symbols
make debug

# Run with strace to see system calls
strace ./inventory 2> debug.log

# Run with gdb for detailed debugging
gdb ./inventory
(gdb) run
# When it crashes:
(gdb) backtrace
```

## Getting Help

If your issue isn't covered here:

1. **Check GitHub Issues**: Search for similar problems
2. **Check audit.log**: May contain error messages
3. **Collect information**:
   ```bash
   # System info
   uname -a
   gcc --version
   ncurses6-config --version  # or ncurses5-config
   echo $TERM
   
   # File permissions
   ls -la inventory.dat audit.log config.dat
   ```
4. **Open an issue** with the collected information

## Preventive Measures

1. **Regular backups**: Use the built-in backup feature
2. **Keep logs**: Don't delete audit.log
3. **Version control**: Keep inventory.dat in git (if appropriate)
4. **Test imports**: Use small test CSV files first
5. **Monitor disk space**: Ensure sufficient space for backups

## Emergency Recovery

If the application is completely broken:

```bash
# Nuclear option - start fresh
mv inventory.dat inventory.dat.corrupted
mv audit.log audit.log.old
mv config.dat config.dat.old

# Rebuild application
make clean && make

# Try to recover from corrupted file using strings
strings inventory.dat.corrupted > recovered.txt
# Manually recreate items from recovered.txt
```

---

**Last Updated**: February 2026  
**Application Version**: 2.0.0
