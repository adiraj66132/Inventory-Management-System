# Maintainer: Your Name <your.email@example.com>
pkgname=inventory-system
pkgver=2.0.0
pkgrel=1
pkgdesc="Terminal-based inventory management system with CSV export, categories, and audit logging"
arch=('x86_64' 'i686' 'aarch64' 'armv7h')
url="https://github.com/yourusername/inventory-system"
license=('MIT')
depends=('ncurses')
makedepends=('gcc' 'make')
source=("$pkgname-$pkgver.tar.gz::https://github.com/yourusername/$pkgname/archive/v$pkgver.tar.gz")
sha256sums=('SKIP')  # Replace with actual checksum

# Uncomment for local development
# source=("inventory.c" "Makefile")
# sha256sums=('SKIP' 'SKIP')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    # For local development, comment above and uncomment below:
    # cd "$srcdir"
    
    make
}

package() {
    cd "$srcdir/$pkgname-$pkgver"
    # For local development, comment above and uncomment below:
    # cd "$srcdir"
    
    # Install binary
    install -Dm755 inventory "$pkgdir/usr/bin/inventory"
    
    # Install documentation
    install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
    
    # Install license
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
    
    # Install example files if they exist
    if [ -d "examples" ]; then
        install -Dm644 examples/*.csv "$pkgdir/usr/share/$pkgname/examples/"
    fi
}

# vim:set ts=2 sw=2 et:
