import sys
from os import path
import gtk
import gtk.glade as glade
import ibus

class Setup ():
    def __init__ (self):
        self.__bus = ibus.Bus()
        self.__config = self.__bus.get_config()
        self.__options = {
            "Keyboard/Page/MinusEquals" : [False, self.__checkbutton_op],
            "Keyboard/Page/CommaPeriod" : [False, self.__checkbutton_op],
        }
            
    def run(self):
        self.__init_ui()
        gtk.main()

    def __checkbutton_op(self, name, opt, info):
        widget = self.__xml.get_widget(name)
        if widget == None:
            print >> sys.stderr, "Can not find widget %s" % name
            return False

        if opt == "read":
            info[0] = self.__read(name, info[0])
            widget.set_active(info[0])
            return True
        if opt == "write":
            info[0] = widget.get_active()
            self.__write(name, info[0])
            return True
        if opt == "check":
            return info[0] != widget.get_active()
        return False
    
    def __read(self, name, v):
        section, key = self.__get_config_name(name)
        return self.__config.get_value(section, key, v)

    def __write(self, name, v):
        section, key = self.__get_config_name(name)
        return self.__config.set_value(section, key, name, v)

    def __get_config_name(self, item_name):
        SEPARATOR = "/"
        keys = item_name.rsplit(SEPARATOR ,1)
        if len(keys) == 2:
            return SEPARATOR.join(("engine/SunPinyin", keys[0])), keys[1]
        else:
            assert len(keys) == 1
            return "engine/SunPinyin", keys[0]
            
    def __load_config(self):
        for name, info in self.__options.items():
            info[1] (name, "read", info)

    def __save_config(self):
        self.__need_reload_config = True
        for name, info in self.__options.items():
            info[1] (name, "write", info)
    
    def __init_ui(self):
        glade_file = path.join(path.dirname(__file__), "setup.glade")
        self.__xml = glade.XML (glade_file)
        self.__window = self.__xml.get_widget("window_main")
        for name, info in self.__options.items():
            info[1] (name, "init", info)
            info[1] (name, "read", info)
        self.__xml.signal_autoconnect(self)
        self.__window.show_all()

    def on_button_ok_clicked(self, button):
        self.__quit()
        
    def on_button_apply_clicked(self, button):
        self.__save_config()

    def on_button_cancel_clicked(self, button):
        self.__quit()

    def __quit(self):
        gtk.main_quit()
    
if __name__ == "__main__":
    Setup().run()

