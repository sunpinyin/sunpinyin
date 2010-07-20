configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sunpinyin.xml.in
               ${CMAKE_CURRENT_BINARY_DIR}/sunpinyin.xml)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sunpinyin.xml
    DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ibus/component")
