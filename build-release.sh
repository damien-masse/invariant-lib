# ==================================================================
#  invariant-lib - build script
# ==================================================================

#!/bin/bash

SCRIPT="$(readlink -f "$0")"
INVARIANTLIB_DIR="$(dirname "$SCRIPT")"

cd ..
sh $INVARIANTLIB_DIR/cmake/build_Ibex4pyIbex.sh # Benoit Desrochers version
sh $INVARIANTLIB_DIR/cmake/build_VTK.sh
sh $INVARIANTLIB_DIR/cmake/build_ppl.sh
sh $INVARIANTLIB_DIR/cmake/build_netcdf.sh
cd $INVARIANTLIB_DIR

mkdir build/build-release -p
cd build/build-release
cmake -DCMAKE_INSTALL_PREFIX=${HOME} \
	  -DIBEX_ROOT=${HOME}/ibex \
	  -DVTK_ROOT=${HOME}/vtk \
	  -DPPL_ROOT=${HOME}/ppl \
	  -DGMP_ROOT=${HOME}/gmp \
	  -DNETCDF_ROOT=${HOME}/netcdf \
	  -DCMAKE_BUILD_TYPE=RELEASE \
	  -DBUILD_TESTS=OFF \
	  -DWITH_PYTHON=ON \
	  -DWITH_3D=ON \
	  -DWITH_GRAPHIZ=OFF \
	  -DWITH_NETCDF=ON \
  	  -DWITH_PYIBEX_VERSION=ON \
  	  -DWITH_PROJ4=ON\
  	  -DWITH_EXAMPLES=ON \
	  -DWITH_PPL=ON \
	  $INVARIANTLIB_DIR
if [ -z "$1" ]; then
	make -j4
else
	make -j4 "$1"
fi
cd ..
