# Maintainer: Geoffroy Planquart <geoffroy@aethelflaed.com>
pkgname=kompex-sqlite-wrapper
pkgver=1.10.12
pkgrel=1
pkgdesc="An open source C++ wrapper library for SQLite"
arch=('x86_64' 'i686')
url="http://sqlitewrapper.kompex-online.com/"
license=('LGPL3')
groups=()
depends=('gcc-libs')
makedepends=('git')
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
source=()
noextract=()
md5sums=()

build() {
  # Although the sources are taken from git, it's not the original repository
  # so I don't use _gitroot and _gitname to prevent pkgver from changin
  giturl='git://github.com/Aethelflaed/kompex-sqlite-wrapper.git'

  cd "$srcdir"
  msg "Connecting to GIT server...."

  if [[ -d "$pkgname" ]]; then
    cd "$pkgname" && git pull origin
    msg "The local files are updated."
  else
    git clone "$giturl" "$pkgname"
  fi

  msg "GIT checkout done or server timeout"
  msg "Starting build..."

  rm -rf "$srcdir/$pkgname-build"
  git clone "$srcdir/$pkgname" "$srcdir/$pkgname-build"
  cd "$srcdir/$pkgname-build"

  #
  # BUILD HERE
  #
  ./configure --prefix=/usr
  make
}

package() {
  cd "$srcdir/$pkgname-build"
  make DESTDIR="$pkgdir/" install
}

# vim:set ts=2 sw=2 et:
