configure_file(config.py.in config.py)
configure_file(ibus-setup-sunpinyin.in ibus-setup-sunpinyin)
install(FILES config.py setup.glade main.py
    DESTINATION "${DATA_DIR}/setup")

