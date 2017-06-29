# ==================================================================
#  VTK Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/vtk/bin/vtkH5detect" ]; then
  echo 'Installing VTK-lib...';
  git clone https://gitlab.kitware.com/vtk/vtk
  cd vtk
  git checkout v8.0.0
  SOURCE_DIR=`pwd`
  BUILD_DIR=${HOME}/vtk

  echo VTK_OURCE_DIR = ${SOURCE_DIR}
  echo VTK_BUILD_DIR = ${BUILD_DIR}

  mkdir -p "${BUILD_DIR}"

  cd ${BUILD_DIR}
  echo `pwd`
  cmake ${SOURCE_DIR}
  make -j4
fi
