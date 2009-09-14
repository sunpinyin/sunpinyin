import sys
from os import path
import gtk
import gtk.glade as glade
import ibus

class Setup ():
    def __init__ (self):
        self.__bus = ibus.Bus()
        self.__config = self.__bus.get_config()

    def run(self):
        self.__init_ui()
        gtk.main()

    def __init_ui(self):
        glade_file = path.join(path.dirname(__file__), "setup.glade")
        self.__xml = glade.XML (glade_file)
        self.__window = self.__xml.get_widget("window_main")
        self.__xml.signal_autoconnect(self)
        self.__window.show_all()
        
if __name__ == "__main__":
    Setup().run()

