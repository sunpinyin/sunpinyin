# CMakeLists.txt: cmake config file for ibus-sunpinyin

find_package(GTK2 REQUIRED gtk)
find_package(iBus 1.2 REQUIRED)
find_package(SunPinyin 2.0 REQUIRED)

include_directories(${GTK2_INCLUDE_DIRS} ${iBus_INCLUDE_DIRS} ${SunPinyin_INCLUDE_DIRS})
set(LIBS ${LIBS} ${GTK2_LIBRARIES} ${iBus_LIBRARIES} ${SunPinyin_LIBRARIES})
set(ENGINE_SRCS
    main.cpp
    engine.cpp 
    pointer.h ibus_common.h
    imi_ibus_win.cpp imi_ibus_win.h
    sunpinyin_config_keys.h
    sunpinyin_config.cpp sunpinyin_config.h
    sunpinyin_engine.cpp sunpinyin_engine.h
    sunpinyin_engine_proxy.cpp sunpinyin_engine_proxy.h
    sunpinyin_lookup_table.cpp sunpinyin_lookup_table.h
    sunpinyin_property.cpp sunpinyin_property.h)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
add_executable(ibus-engine-sunpinyin ${ENGINE_SRCS})
target_link_libraries(ibus-engine-sunpinyin ${LIBS})

install(TARGETS ibus-engine-sunpinyin
    RUNTIME DESTINATION ${LIBEXEC_DIR})
