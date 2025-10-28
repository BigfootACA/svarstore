# Maintainer: BigfootACA <bigfoot@radxa.com>
# Maintainer: BigfootACA <bigfoot@classfun.cn>

pkgname=svarstore
pkgver=0.1
pkgrel=1
pkgdesc="Simple EFI variable store manager for Radxa devices"
arch=(x86_64 aarch64)
url="https://github.com/BigfootACA/svarstore"
license=(GPL-2.0-or-later)
depends=(glibc)
makedepends=(gcc git meson ninja)
options=(staticlibs)
source=(meson.build)
md5sums=(SKIP)

pkgver(){
        cd "$(dirname "$(realpath "$srcdir/meson.build")")"
        bash scripts/version.sh
}

build(){
	dir="$(dirname "$(realpath "$srcdir/meson.build")")"
	arch-meson "$dir" build
}

package() {
	dir="$(dirname "$(realpath "$srcdir/meson.build")")"
        DESTDIR="$pkgdir" ninja -C build install
}
