# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-src"
  "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-build"
  "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-subbuild/math_lib-populate-prefix"
  "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-subbuild/math_lib-populate-prefix/tmp"
  "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-subbuild/math_lib-populate-prefix/src/math_lib-populate-stamp"
  "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-subbuild/math_lib-populate-prefix/src"
  "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-subbuild/math_lib-populate-prefix/src/math_lib-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-subbuild/math_lib-populate-prefix/src/math_lib-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/iwillwin/homeWorkFirst/build/_deps/math_lib-subbuild/math_lib-populate-prefix/src/math_lib-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
