#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="diy"
readonly ownership="Diy Upstream <kwrobot@kitware.com>"
readonly subtree="vtkm/thirdparty/$name/vtkm$name"
readonly repo="https://github.com/KitwareMedical/diy.git"
readonly tag="slicersmtk_for-vtk-m-20220914-master-2020-08-26-0f1c387"
readonly paths="
cmake
include
CMakeLists.txt
LEGAL.txt
LICENSE.txt
README.md
"

extract_source () {
    git_archive
    pushd "$extractdir/$name-reduced"
    mv include/diy include/vtkmdiy
    popd
}

. "${BASH_SOURCE%/*}/../update-common.sh"
