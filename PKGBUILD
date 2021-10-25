# Maintainer: fenuks

_pkgname=cantata
pkgname=${_pkgname}-git
pkgver=2.4.2.r7557.8eb3a9651
pkgrel=1
pkgdesc="Qt5 Graphical MPD Client "
arch=('x86_64' 'aarch64')
url="https://github.com/fenuks/cantata"
license=('GPL3')
depends=(
  libcdio-paranoia
  libmtp
  libmusicbrainz5
  mpg123
  qt5-multimedia
  qt5-svg
  taglib
  libebur128
  ffmpeg
)
makedepends=('cmake')
provides=("$_pkgname")
conficts=("$_pkgname")
source=("$pkgname::git+file://$(pwd)")
sha1sums=('SKIP')

pkgver() {
    cd "${srcdir}/${pkgname}"
    printf "%s.r%s.%s" "$(git tag | tail -n 1 | sed 's/^v//')" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
    cd "${srcdir}/${pkgname}"
    mkdir -p build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
}

build() {
  cd "${srcdir}/${pkgname}/build"
  make
}

package() {
  cd "${srcdir}/${pkgname}/build"
  make DESTDIR="${pkgdir}" install
}

