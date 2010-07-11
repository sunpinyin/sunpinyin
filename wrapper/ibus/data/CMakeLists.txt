# CMakeLists.txt: cmake config file for SunPinyin 2 build on Mac

configure_file(sunpinyin.xml.in sunpinyin.xml)
install(FILES sunpinyin.xml
    DESTINATION "${DATA_DIR}/ibus/component")

