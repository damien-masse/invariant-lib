# ==================================================================
#  VTK Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/gmp/lib/libgmp.so" ]; then
  echo 'Installing PPL-lib...';
  GMP_VERSION=6.1.2
  GMP_FILE_NAME=gmp-${VERSION}
  GMP_ARCHIVE_NAME=${FILE_NAME}.tar.xz

  wget https://gmplib.org/download/gmp/${VERSION}/${ARCHIVE_NAME}
  tar xf ${GMP_ARCHIVE_NAME}
  rm ${GMP_ARCHIVE_NAME}
  cd ${GMP_FILE_NAME}

  ./configure --prefix=$HOME/gmp --enable-cxx CXXFLAGS=-fPIC CFLAGS=-fPIC
  make
  make check
  make install
  cd ..
  rm -R ${GMP_ARCHIVE_NAME}
fi


if [ ! -e "$HOME/ppl/lib/libppl.so" ]; then
  echo 'Installing PPL-lib...';

  git clone git://git.cs.unipr.it/ppl/ppl.git
  cd ppl
  git checkout devel
  autoreconf
  ./configure --prefix=$HOME/ppl --enable-interfaces=cxx --enable-optimization=sspeed --enable-fpmath=default \
      --disable-ppl_lpsol --disable-ppl_lcdd --enable-thread-safe --enable-shared --with-cxxflags=-fPIC --with-cflags=-fPIC \
      --with-gmp=$HOME/gmp
  make -j4
  make install
fi

