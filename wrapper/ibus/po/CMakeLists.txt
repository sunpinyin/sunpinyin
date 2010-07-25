include(Gettext)

set(POT_SRCS
    ../src/main.cpp
    ../src/sunpinyin_property.cpp
    ../setup/setup.glade)
set(LANGS zh_CN)
set(POT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${GETTEXT_PACKAGE}.pot)

if (GETTEXT_FOUND)
    GETTEXT_CREATE_POT(${POT_FILE} OPTION ${XGETTEXT_OPTIONS} SRC ${POT_SRCS})
    GETTEXT_CREATE_TRANSLATIONS(${POT_FILE} ALL ${LANGS}
                                COMMENT "Creating translations.")
endif()

