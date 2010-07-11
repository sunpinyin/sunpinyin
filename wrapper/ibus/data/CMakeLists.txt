# CMakeLists.txt: cmake config file for ibus-sunpinyin

configure_file(sunpinyin.xml.in sunpinyin.xml)
install(FILES sunpinyin.xml
    DESTINATION "${DATA_DIR}/ibus/component")

