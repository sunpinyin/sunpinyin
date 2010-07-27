# CMakeLists.txt: cmake config file for ibus-sunpinyin

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

cmake_minimum_required(VERSION 2.6)
project(SUNPINYIN)

set(PACKAGE             ibus-sunpinyin)
set(GETTEXT_PACKAGE     ${PACKAGE})
set(PACKAGE_NAME        ${PACKAGE})
set(PACKAGE_BUGREPORT   "sunpinyin-developers@googlegroups.com")
set(PACKAGE_TARNAME     ${PACKAGE})
set(VERSION             "2.0")
set(PACKAGE_VERSION     ${VERSION})
set(PACKAGE_STRING      "${PACKAGE} ${VERSION}")

include(Paths)

install(DIRECTORY icons
    DESTINATION "${DATA_DIR}")

add_subdirectory (src)
add_subdirectory (setup)
add_subdirectory (data)
add_subdirectory (po)
