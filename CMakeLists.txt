# CMakeLists.txt: cmake config file for SunPinyin 2 build on Mac

include(CheckIncludeFiles)
include(CheckFunctionExists)

cmake_minimum_required(VERSION 2.6)
project(SUNPINYIN)

set(CMAKE_OSX_ARCHITECTURES x86_64;i386)

if (APPLE)
    set(HOST_OS "mac")
endif (APPLE)

set(PACKAGE             sunpinyin)
set(GETTEXT_PACKAGE     ${PACKAGE})
set(PACKAGE_NAME        ${PACKAGE})
set(PACKAGE_BUGREPORT   "sunpinyin-developers@googlegroups.com")
set(PACKAGE_TARNAME     ${PACKAGE})
set(VERSION             "2.0")
set(PACKAGE_VERSION     ${VERSION})
set(PACKAGE_STRING      "${PACKAGE} ${VERSION}")

check_include_files(assert.h    HAVE_ASSERT_H)
check_include_files(dlfcn.h     HAVE_DLFCN_H)
check_include_files(fcntl.h     HAVE_FCNTL_H)
check_include_files(getopt.h    HAVE_GETOPT_H)
check_include_files(iconv.h     HAVE_ICONV_H)
check_include_files(inttypes.h  HAVE_INTTYPES_H)
check_include_files(libintl.h   HAVE_LIBINTL_H)
check_include_files(limit.h     HAVE_LIMIT_H)
check_include_files(locale.h    HAVE_LOCALE_H)
check_include_files(stdlib.h    HAVE_STDLIB_H)
check_include_files(string.h    HAVE_STRING_H)
check_include_files(strings.h   HAVE_STRINGS_H)
check_include_files(memory.h    HAVE_MEMORY_H)
check_include_files(stdint.h    HAVE_STDINT_H)
check_include_files(wchar.h     HAVE_WCHAR_H)
check_include_files(unistd.h    HAVE_UNISTD_H)
check_include_files(sys/mman.h  HAVE_SYS_MMAN_H)
check_include_files(sys/stat.h  HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)

check_function_exists(getopt_long HAVE_GETOPT_LONG)
check_function_exists(getpagesize HAVE_GETPAGESIZE)
check_function_exists(gettext     HAVE_GETTEXT)
check_function_exists(get_opt     HAVE_GET_OPT)
check_function_exists(munmap      HAVE_MUNMAP)
check_function_exists(memset      HAVE_MEMSET)
check_function_exists(setlocale   HAVE_SETLOCALE)
check_function_exists(strdup      HAVE_STRDUP)
check_function_exists(strndup     HAVE_STRNDUP)
check_function_exists(strlen      HAVE_STRLEN)
check_function_exists(strcpy      HAVE_STRCPY)
check_function_exists(log2        HAVE_LOG2)
check_function_exists(exp2        HAVE_EXP2)

configure_file(config.h.cmakein config.h ESCAPE_QUOTES)
configure_file(src/config/${HOST_OS}.h host_os.h ESCAPE_QUOTES)

set(SOURCE_DIR      "${SUNPINYIN_SOURCE_DIR}/src")
set(CORPUS_DIR      "${SUNPINYIN_SOURCE_DIR}/raw")
set(RESULT_DIR      "${SUNPINYIN_SOURCE_DIR}/data")
set(SWAP_DIR        "${SUNPINYIN_SOURCE_DIR}/swap")
set(LMTARGET        "lm_sc")

set(SLM_SRC_DIR     "${SOURCE_DIR}/slm")
set(LEXICON_SRC_DIR "${SOURCE_DIR}/lexicon")
set(PINYIN_SRC_DIR  "${SOURCE_DIR}/pinyin")

add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${PINYIN_SRC_DIR})

set(GENPYT_SOURCES 
    ${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h host_os.h
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h
	${LEXICON_SRC_DIR}/genPYT.cpp ${LEXICON_SRC_DIR}/pytrie.cpp ${LEXICON_SRC_DIR}/pytrie.h
	${LEXICON_SRC_DIR}/pytrie_gen.cpp ${LEXICON_SRC_DIR}/pytrie_gen.h
	${PINYIN_SRC_DIR}/pinyin_data.cpp ${PINYIN_SRC_DIR}/pinyin_data.h
	${PINYIN_SRC_DIR}/syllable.h)

find_library(iconv iconv)
add_executable(genpyt ${GENPYT_SOURCES})
target_link_libraries(genpyt ${iconv})

set(PINYIN_TEXTFILE "${CORPUS_DIR}/dict.utf8")
set(PYTRIE_FILE     "${RESULT_DIR}/pydict_sc.bin")
set(PYTRIE_PRINTOUT "${SWAP_DIR}/pydict_sc.log.utf8")
set(TSLM_FILE3      "${RESULT_DIR}/${LMTARGET}.t3g")

add_custom_target(lexicon ${CMAKE_CURRENT_BINARY_DIR}/genpyt ${PINYIN_TEXTFILE} ${PYTRIE_FILE} ${PYTRIE_PRINTOUT} ${TSLM_FILE3})

