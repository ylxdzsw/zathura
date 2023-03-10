# Maintainer: Johannes Löthberg <johannes@kyriasis.com>
# Maintainer: Robin Candau <antiz@archlinux.org>
# Contributor: Daniel Wallace <danielwallace at gtmanfred dot com>
# Contributor: Sergej Pupykin <pupykin.s+arch@gmail.com>
# Contributor: ML <neldoreth>

pkgname=zathura-fork
pkgver=0.5.2
pkgrel=2

pkgdesc="Minimalistic document viewer"
url="https://pwmt.org/projects/zathura/"
arch=('x86_64')
license=('custom')
provides=('zathura')
conflicts=('zathura')

makedepends=('meson' 'ninja')
depends=('girara>=0.2.7' 'sqlite' 'desktop-file-utils' 'file' 'libsynctex')
optdepends=('zathura-djvu: DjVu support'
            'zathura-pdf-poppler: PDF support using Poppler'
            'zathura-pdf-mupdf: PDF support using MuPDF'
            'zathura-ps: PostScript support'
            'zathura-cb: Comic book support')

source=(git+https://github.com/ylxdzsw/zathura)

sha256sums=('SKIP')

build() {
  cd zathura
  meson setup --prefix /usr --libexecdir lib --sbindir bin --buildtype plain --wrap-mode nodownload -D b_lto=true -D b_pie=true build

  cd build
  ninja
}

check() {
  cd zathura/build

  ninja test
}

package() {
  cd zathura/build
  DESTDIR="$pkgdir" ninja install

  install -D -m664 ../LICENSE "$pkgdir"/usr/share/licenses/$pkgname/LICENSE
}
