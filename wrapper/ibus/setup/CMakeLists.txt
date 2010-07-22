configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ibus-setup-sunpinyin.in
               ${CMAKE_CURRENT_BINARY_DIR}/ibus-setup-sunpinyin
               @ONLY)
install(FILES setup.glade main.py
              ${CMAKE_CURRENT_BINARY_DIR}/config.py
    DESTINATION "${DATA_DIR}/setup")
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ibus-setup-sunpinyin
    DESTINATION ${LIBEXEC_DIR})
