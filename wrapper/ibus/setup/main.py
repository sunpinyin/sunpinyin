# 
# Copyright (c) 2009 Leo Zheng <zym361@gmail.com>, Kov Chai <tchaikov@gmail.com>
#  *
# The contents of this file are subject to the terms of either the GNU Lesser
# General Public License Version 2.1 only ("LGPL") or the Common Development and
# Distribution License ("CDDL")(collectively, the "License"). You may not use this
# file except in compliance with the License. You can obtain a copy of the CDDL at
# http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
# http://www.opensource.org/licenses/lgpl-license.php. See the License for the 
# specific language governing permissions and limitations under the License. When
# distributing the software, include this License Header Notice in each file and
# include the full text of the License in the License file as well as the
# following notice:
# 
# NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
# (CDDL)
# For Covered Software in this distribution, this License shall be governed by the
# laws of the State of California (excluding conflict-of-law provisions).
# Any litigation relating to this License shall be subject to the jurisdiction of
# the Federal Courts of the Northern District of California and the state courts
# of the State of California, with venue lying in Santa Clara County, California.
# 
# Contributor(s):
# 
# If you wish your version of this file to be governed by only the CDDL or only
# the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
# include this software in this distribution under the [CDDL or LGPL Version 2.1]
# license." If you don't indicate a single choice of license, a recipient has the
# option to distribute your version of this file under either the CDDL or the LGPL
# Version 2.1, or to extend the choice of license to its licensees as provided
# above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
# Version 2 license, then the option applies only if the new code is made subject
# to such option by the copyright holder. 
#

import sys
from os import path
import gtk
import gtk.glade as glade
import ibus
import gettext

GETTEXT_PACKAGE="sunpinyin"
_ = lambda msg: gettext.dgettext(msg)
GLADE_FILE = "setup.glade"
SEPARATOR = "/"

class Logger:
    @staticmethod
    def pr(message):
        print >> sys.stderr, message
        
class Option(object):
    """Option serves as an interface of ibus.config

    it is used to synchronize the configuration with setting on user interface
    """
    config = ibus.Bus().get_config()
    
    def __init__(self, name, default):
        self.name = name
        self.default = default
    
    def read(self):
        section, key = self.__get_config_name()
        return self.config.get_value(section, key, self.default)

    def write(self, v):
        section, key = self.__get_config_name()
        return self.config.set_value(section, key, v)

   
    def __get_config_name(self):
        keys = self.name.rsplit(SEPARATOR ,1)
        if len(keys) == 2:
            return SEPARATOR.join(("engine/SunPinyin", keys[0])), keys[1]
        else:
            assert len(keys) == 1
            return "engine/SunPinyin", keys[0]

class TrivalOption(Option):
    """option represented using a simple gtk widget
    """
    def __init__(self, name, default, owner):
        super(TrivalOption, self).__init__(name, default)
        self.xml = owner
        self.widget = owner.get_widget(name)
        assert self.widget is not None, "%s not found in glade" % name

    def init_ui(self):
        self.init()
        self.load_config()

    def init(self):
        pass
    
    def load_config(self):
        """update user inferface with ibus.config
        """
        self.v = self.read()
        self.widget.set_active(self.v)

    def store_config(self):
        v = self.save()
        self.write(v)
        
    def save_ui_setting(self):
        """save user interface settings into self.v
        """
        self.v = self.widget.get_active()
        return self.v
    
    def is_changed(self):
        return self.v != self.widget.get_active()
 
class CheckBoxOption(TrivalOption):
    def __init__(self, name, default, owner):
        super(CheckBoxOption, self).__init__(name, default, owner)

class ComboBoxOption(TrivalOption):
    def __init__(self, name, default, options, owner):
        super(ComboBoxOption, self).__init__(name, default, owner)
        self.options = options
        
    def init(self):
        model = gtk.ListStore(str)
        for v in self.options:
            model.append([_(str(v))])
        self.widget.set_model(model)
        
    def load_config(self):
        self.v = self.read()
        self.widget.set_active(self.options.index(self.v))
      
class RadioOption(Option):
    """option represented using multiple Raidio buttons
    """
    def __init__(self, name, default, options, owner):
        super(RadioOption, self).__init__(name, default)
        self.options = options
        self.xml = owner

    def init_ui(self):
        self.load_config()
        
    def load_config(self):
        self.v = self.read()
        name = SEPARATOR.join([self.name, self.v])
        button = self.xml.get_widget(name)
        assert button is not None, "button: %r not found" % name
        button.set_active(True)

    def store_config(self):
        active_opt = None
        for opt in self.options:
            radio_name = SEPARATOR.join(self.name, opt)
            radio = self.xml.get_widget(radio_name)
            if radio.get_active():
                active_opt = opt
                break
        assert active_opt is not None
        self.write(active_opt)
        
class OptionInfo(object):
    def __init__(self, name, default):
        self.name = name
        self.default = default
        
class MultiCheckDialog (object):
    """ a modale dialog box with 'choose all' and 'choose none' button
    
    TODO: another option is to use radio button
    """
    def __init__ (self, name, options):
        self.name = name
        dlg_name = self.get_setup_name()
        self.__xml = glade.XML(GLADE_FILE, dlg_name)
        Logger.pr("loading glade::%s" % dlg_name)
        self.__dlg = self.__xml.get_widget(dlg_name)
        assert self.__dlg is not None, "dialog %s not found in %s" % (dlg_name, GLADE_FILE)
        self.__options = [CheckBoxOption(opt.name, opt.default, self.__xml) 
                          for opt in options]

    def get_setup_name(self):
        """assuming the name of dialog looks like 'dlg_fuzzy_setup'
        """
        return '_'.join(['dlg', self.name, 'setup'])
    
    def __init_ui(self):
        handlers = {'_'.join(["on", self.name, "select_all_clicked"]) : self.on_button_check_all_clicked,
                    '_'.join(["on", self.name, "unselect_all_clicked"]) : self.on_button_uncheck_all_clicked,
                    '_'.join(["on", self.name, "ok_clicked"]) : self.on_button_ok_clicked,
                    '_'.join(["on", self.name, "cancel_clicked"]) : self.on_button_cancel_clicked}
        self.__xml.signal_autoconnect(handlers)

    def dummy(self):
        """a dummy func, i don't initialize myself upon other's request.
        instead, i will do it by myself.
        """
        pass

    init_ui = load_config = dummy
    
    def run(self):
        self.__init_ui()
        self.__load_config()
        self.__dlg.run()
        
    def __load_config(self):
        for opt in self.__options:
            opt.load_config()

    def __save_ui_settings(self):
        for opt in self.__options:
            opt.save_ui_setting()
            
    def __set_active_all(self, is_active):
        for opt in self.__options:
            opt.widget.set_active(is_active)

    def store_config(self):
        """write to in-memory storage, will flush to config if not 'cancel'ed in main_window
        """
        for opt in self.__options:
            opt.store_config()
            
    def on_button_check_all_clicked(self, button):
        self.__set_active_all(True)
        
    def on_button_uncheck_all_clicked(self, button):
        self.__set_active_all(False)
    
    def on_button_ok_clicked(self, button):
        """update given options with settings in UI, these settings will be
        written to config if user push 'OK' or 'Apply' in the main window
        """
        self.__save_ui_settings()
        self.__dlg.destroy()
        
    def on_button_cancel_clicked(self, button):
        self.__dlg.destroy()

class FuzzySetupDialog (MultiCheckDialog):
    def __init__(self):
        options = [OptionInfo("QuanPin/Fuzzy/ShiSi", False),
                   OptionInfo("QuanPin/Fuzzy/ZhiZi", False),
                   OptionInfo("QuanPin/Fuzzy/ChiCi", False),
                   OptionInfo("QuanPin/Fuzzy/ShiSi", False),
                   OptionInfo("QuanPin/Fuzzy/AnAng", False),
                   OptionInfo("QuanPin/Fuzzy/OnOng", False),
                   OptionInfo("QuanPin/Fuzzy/EnEng", False),
                   OptionInfo("QuanPin/Fuzzy/InIng", False),
                   OptionInfo("QuanPin/Fuzzy/EngOng", False),
                   OptionInfo("QuanPin/Fuzzy/IanIang", False),
                   OptionInfo("QuanPin/Fuzzy/UanUang", False),
                   OptionInfo("QuanPin/Fuzzy/NeLe", False),
                   OptionInfo("QuanPin/Fuzzy/FoHe", False),
                   OptionInfo("QuanPin/Fuzzy/LeRi", False),
                   OptionInfo("QuanPin/Fuzzy/KeGe", False)]
        super(FuzzySetupDialog, self).__init__("fuzzy", options)

class CorrectingSetupDialog (MultiCheckDialog):
    def __init__(self):
        options = [OptionInfo("QuanPin/AutoCorrecting/IgnIng", False),
                   OptionInfo("QuanPin/AutoCorrecting/UenUn", False),
                   OptionInfo("QuanPin/AutoCorrecting/ImgIng", False),
                   OptionInfo("QuanPin/AutoCorrecting/IouIu", False),
                   OptionInfo("QuanPin/AutoCorrecting/UeiUi", False)]
        super(CorrectingSetupDialog, self).__init__("correcting", options)

class PunctMappingSetupDialog (MultiCheckDialog):
    def __init__(self):
        options = []
        super(PunctMappingSetupDialog, self).__init__("punctmapping", options)
    
class MainWindow ():
    def __init__ (self):
        self.__bus = ibus.Bus()
        self.__config = self.__bus.get_config()
        
        
    def run(self):
        self.__init_ui("main_window")
        self.__load_config()
        gtk.main()
        
    def __init_ui(self, name):
        glade_file = path.join(path.dirname(__file__), GLADE_FILE)
        self.__xml = glade.XML (glade_file, name)
        self.__init_options()
        self.window = self.__xml.get_widget(name)
        self.__xml.signal_autoconnect(self)
        self.window.show_all()

    def __init_gettext(self):
        local.setlocal(local.LC_ALL, "")
        localdir = os.getenv("IBUS_LOCALEDIR")
        gettext.bindtextdomain(GETTEXT_PACKAGE, localedir)
        gettext.bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8")
        glade.bindtextdomain(GETTEXT_PACKAGE, localedir)
        glade.textdomain(GETTEXT_PACKAGE)

    def __init_options(self):
        self.__fuzzy_setup = FuzzySetupDialog()
        self.__correcting_setup = CorrectingSetupDialog()
        self.__punctmapping_setup = PunctMappingSetupDialog()
        
        self.__options = [
            ComboBoxOption("General/MemoryPower", 3, range(10), self.__xml),
            RadioOption("General/InitialStatus/Mode", 'Chinese', ['Chinese', 'English'], self.__xml),
            RadioOption("General/InitialStatus/Punct", 'Full', ['Full', 'Half'], self.__xml),
            RadioOption("General/InitialStatus/Letter", 'Full', ['Full', 'Half'], self.__xml),
            RadioOption("General/Charset", 'GBK', ['GB2312', 'GBK', 'GB18030'], self.__xml),
            CheckBoxOption("General/PunctMapping/Enabled", False, self.__xml),
            
            ComboBoxOption("General/PageSize", 10, range(7, 11), self.__xml),
            
            CheckBoxOption("Keyboard/Page/MinusEquals", False, self.__xml),
            CheckBoxOption("Keyboard/Page/Brackets", False, self.__xml),
            CheckBoxOption("Keyboard/Page/CommaPeriod", False, self.__xml),
            CheckBoxOption("Keyboard/FullLetter/ShiftSemi", False, self.__xml),
            CheckBoxOption("Keyboard/ModeSwitch/Shift", False, self.__xml),
            CheckBoxOption("Keyboard/ModeSwitch/Control", False, self.__xml),
            
            RadioOption("Pinyin/Scheme", 'QuanPin', ['QuanPin', 'ShuangPin'], self.__xml),
            ComboBoxOption("Pinyin/ShuangPinType", 'Pinyin++', ['MS2003',
                                                                'ABC',
                                                                'ZiRanMa',
                                                                'Pinyin++',
                                                                'ZiGuang'], self.__xml),
            CheckBoxOption("QuanPin/Fuzzy/Enabled", False, self.__xml),
            CheckBoxOption("QuanPin/AutoCorrecting/Enabled", False, self.__xml),
            self.__fuzzy_setup,
            self.__correcting_setup,
            self.__punctmapping_setup,
        ]

    def __get_option(self, name):
        for opt in self.__options:
            if opt.name == name:
                return opt
        else:
            return None
        
    def __load_config(self):
        for opt in self.__options:
            opt.init_ui()
            opt.load_config()
        self.on_chk_fuzzy_enabled_toggled(None)
        self.on_chk_correcting_enabled_toggled(None)
        self.on_chk_punctmapping_enabled_toggled(None)
        self.on_radio_shuangpin_toggled(None)
        
    def __store_config(self):
        for opt in self.__options:
            opt.store_config()

    def __update_enabling_button(self, checkbox_name, button_name):
        """enable a setup button when checked, disable it otherwise
        """
        checkbox = self.__xml.get_widget(checkbox_name)
        assert checkbox is not None, "checkbox: %s not found" % checkbox_name
        button = self.__xml.get_widget(button_name)
        assert button is not None, "button: %s not found" % button_name
        button_enabled = checkbox.get_active()
        button.set_sensitive(button_enabled)

    def on_radio_shuangpin_toggled(self, button):
        radio = self.__xml.get_widget("Pinyin/Scheme/ShuangPin")
        enabled = radio.get_active()
        combo = self.__xml.get_widget("Pinyin/ShuangPinType")
        combo.set_sensitive(enabled)
        
    def on_chk_fuzzy_enabled_toggled(self, button):
        self.__update_enabling_button("QuanPin/Fuzzy/Enabled",
                                      "button_fuzzy_setup")
        
    def on_button_fuzzy_setup_clicked(self, button):
        self.__fuzzy_setup.run()
        
    def on_chk_correcting_enabled_toggled(self, button):
        self.__update_enabling_button("QuanPin/AutoCorrecting/Enabled",
                                      "button_autocorrect_setup")
        
    def on_button_correcting_setup_clicked(self, button):
        self.__correcting_setup.run()
        
    def on_chk_punctmapping_enabled_toggled(self, button):
        self.__update_enabling_button("General/PunctMapping/Enabled",
                                      "button_punctmapping_setup")
    
    def on_button_punctmapping_setup_clicked(self, button):
        self.__punctmapping_setup.run()
    
    def on_main_ok_clicked(self, button):
        self.__save_config()
        self.__quit()
        
    def on_main_apply_clicked(self, button):
        self.__save_config()

    def on_main_cancel_clicked(self, button):
        self.__quit()

    def __quit(self):
        gtk.main_quit()
    
if __name__ == "__main__":
    MainWindow().run()
