# -*- coding: utf-8 -*-
# 
# Copyright (c) 2009 Leo Zheng <zym361@gmail.com>, Kov Chai <tchaikov@gmail.com>
# *
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
import os
from os import path
import gtk
import gtk.glade as glade
import ibus
import gettext
import locale

GETTEXT_PACKAGE="ibus-sunpinyin"
_ = lambda msg: gettext.gettext(msg)

GLADE_FILE = path.join(path.dirname(__file__), "setup.glade")
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
        return self.config.set_value(section, key, type(self.default)(v))

   
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
        self.read_config()

    def init(self):
        pass
    
    def read_config(self):
        """update user inferface with ibus.config
        """
        self.v = self.read()
        self.__set_value(self.v)

    def write_config(self):
        v = self.save_ui_setting()
        self.write(v)
        
    def save_ui_setting(self):
        """save user interface settings into self.v
        """
        self.v = self.__get_value()
        return self.v
    
    def is_changed(self):
        return self.v != self.__get_value()

    def __get_value(self):
        try:
            return self.widget.get_value()
        except:
            return self.widget.get_active()

    def __set_value(self, v):
        try:
            self.widget.set_value(v)
        except:
            self.widget.set_active(v)
            
class CheckBoxOption(TrivalOption):
    def __init__(self, name, default, owner):
        super(CheckBoxOption, self).__init__(name, default, owner)

class ComboBoxOption(TrivalOption):
    def __init__(self, name, default, options, owner):
        try:
            default = int(default)
        except ValueError:
            default = options.index(default)
        super(ComboBoxOption, self).__init__(name, default, owner)
        self.options = options
        
    def init(self):
        model = gtk.ListStore(str)
        for v in self.options:
            model.append([str(v)])
        self.widget.set_model(model)

    def __get_value(self):
        active = self.widget.get_active()
        try:
            # if the options are numbers, save the liternal of active option as
            # a number
            return int(self.options[active])
        except ValueError:
            # otherwise save its index
            return active
    
    def __set_value(self, v):
        try:
            # if the options are just numbers, we treat 'self.v' as the literal
            # of option
            dummy = int(self.options[0])
            active = self.options.index(v)
        except ValueError:
            active = v
        self.widget.set_active(active)
        
class RadioOption(Option):
    """option represented using multiple Raidio buttons
    """
    def __init__(self, name, default, options, owner):
        super(RadioOption, self).__init__(name, default)
        self.options = options
        self.xml = owner

    def init_ui(self):
        self.read_config()
        
    def read_config(self):
        self.v = self.read()
        name = SEPARATOR.join([self.name, self.v])
        button = self.xml.get_widget(name)
        assert button is not None, "button: %r not found" % name
        button.set_active(True)

    def write_config(self):
        active_opt = None
        for opt in self.options:
            radio_name = SEPARATOR.join([self.name, opt])
            radio = self.xml.get_widget(radio_name)
            if radio.get_active():
                active_opt = opt
                break
        assert active_opt is not None
        self.write(active_opt)

class MappingInfo:
    def __init__(self, name, mapping):
        self.name = name
        self.mapping = mapping
        
class MappingOption(object):
    """an option which presents some sort of mapping, e.g. fuzzy pinyin mapping

    it is not directly related to a config option like TrivalOption does, but
    we always have a checkbox in UI for each of it so user can change it easily.
    """
    def __init__(self, name, mappings, owner):
        self.name = name
        self.widget = owner.get_widget(name)
        self.mappings = mappings
        
    def get_mappings(self):
        if self.widget.get_active():
            return [':'.join(self.mappings)]
        else:
            return []

    def set_active(self, enabled):
        self.widget.set_active(enabled)

    def get_active(self):
        return self.widget.get_active()
    
    is_enabled = property(get_active, set_active)

    def key(self):
        return self.mappings[0]
    
class MultiMappingOption(Option):
    def __init__(self, name, options, default=[]):
        Option.__init__(self, name, default)
        self.options = options
        self.saved_pairs = default
        
    def read_config(self):
        if not self.saved_pairs:
            self.saved_pairs = self.read()
        keys = set([pair.split(':')[0] for pair in self.saved_pairs])
        for opt in self.options:
            opt.is_enabled = (opt.key() in keys)
            # throw away unknown pair
    
    def write_config(self):
        # ignore empty settings
        if self.saved_pairs:
            self.write(self.saved_pairs)
        
    def save_ui_setting(self):
        self.saved_pairs = sum([opt.get_mappings() for opt in self.options
                                if opt.is_enabled], [])
        return self.saved_pairs
    
    def set_active_all(self, enabled):
        for opt in self.options:
            opt.is_enabled = enabled
            
class MultiCheckDialog (object):
    """ a modal dialog box with 'choose all' and 'choose none' button
    
    TODO: another option is to use radio button
    """
    def __init__ (self, ui_name, config_name, mappings, option_klass=MappingOption):
        self.ui_name = ui_name
        self.config_name = config_name
        self.mappings = mappings
        self.option_klass = option_klass
        self.saved_settings = []
        self.mapping_options = None
        
    def get_setup_name(self):
        """assuming the name of dialog looks like 'dlg_fuzzy_setup'
        """
        return '_'.join(['dlg', self.ui_name, 'setup'])
    
    def __init_ui(self):
        dlg_name = self.get_setup_name()
        self.__xml = glade.XML(GLADE_FILE, dlg_name)
        self.__dlg = self.__xml.get_widget(dlg_name)
        assert self.__dlg is not None, "dialog %s not found in %s" % (dlg_name, GLADE_FILE)
        handlers = {'_'.join(["on", self.ui_name, "select_all_clicked"]) : self.on_button_check_all_clicked,
                    '_'.join(["on", self.ui_name, "unselect_all_clicked"]) : self.on_button_uncheck_all_clicked,
                    '_'.join(["on", self.ui_name, "ok_clicked"]) : self.on_button_ok_clicked,
                    '_'.join(["on", self.ui_name, "cancel_clicked"]) : self.on_button_cancel_clicked}
        self.__xml.signal_autoconnect(handlers)

        options = [self.option_klass(m.name, m.mapping, self.__xml) 
                   for m in self.mappings]
        self.mapping_options = MultiMappingOption(self.config_name, options, self.saved_settings)

    def dummy(self):
        """a dummy func, i don't initialize myself upon other's request.
        instead, i will do it by myself.
        """
        pass

    init_ui = read_config = dummy
    
    def run(self):
        self.__init_ui()
        self.__read_config()
        self.__dlg.run()
        
    def __read_config(self):
        self.mapping_options.read_config()
        
    def __save_ui_settings(self):
        """save to in-memory storage, will flush to config if not canceled in main_window
        """
        self.saved_settings = self.mapping_options.save_ui_setting()

    def write_config(self):
        if self.mapping_options is not None:
            self.mapping_options.write_config()
            
    def on_button_check_all_clicked(self, button):
        self.mapping_options.set_active_all(True)
        
    def on_button_uncheck_all_clicked(self, button):
        self.mapping_options.set_active_all(False)
    
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
        mappings = [MappingInfo('QuanPin/Fuzzy/ShiSi', ('sh','s')),
                    MappingInfo('QuanPin/Fuzzy/ZhiZi', ('zh','z')),
                    MappingInfo('QuanPin/Fuzzy/ChiCi', ('ch','c')),
                    MappingInfo('QuanPin/Fuzzy/ShiSi', ('sh','s')),
                    MappingInfo('QuanPin/Fuzzy/AnAng', ('an','ang')),
                    MappingInfo('QuanPin/Fuzzy/OnOng', ('on','ong')),
                    MappingInfo('QuanPin/Fuzzy/EnEng', ('en','eng')),
                    MappingInfo('QuanPin/Fuzzy/InIng', ('in','ing')),
                    MappingInfo('QuanPin/Fuzzy/EngOng', ('eng','ong')),
                    MappingInfo('QuanPin/Fuzzy/IanIang', ('ian','iang')),
                    MappingInfo('QuanPin/Fuzzy/UanUang', ('uan','uang')),
                    MappingInfo('QuanPin/Fuzzy/NeLe', ('n','l')),
                    MappingInfo('QuanPin/Fuzzy/FoHe', ('f','h')),
                    MappingInfo('QuanPin/Fuzzy/LeRi', ('l','r')),
                    MappingInfo('QuanPin/Fuzzy/KeGe', ('k','g'))]
        MultiCheckDialog.__init__(self,
                                  ui_name = 'fuzzy',
                                  config_name = 'QuanPin/Fuzzy/Pinyins',
                                  mappings = mappings)
        
class CorrectionSetupDialog (MultiCheckDialog):
    def __init__(self):
        mappings = [MappingInfo('QuanPin/AutoCorrection/GnNg', ('gn','ng')),
                    MappingInfo('QuanPin/AutoCorrection/UenUn', ('uen','un')),
                    MappingInfo('QuanPin/AutoCorrection/ImgIng', ('img','ing')),
                    MappingInfo('QuanPin/AutoCorrection/IouIu', ('iou','iu')),
                    MappingInfo('QuanPin/AutoCorrection/UeiUi', ('uei','ui'))]
        MultiCheckDialog.__init__(self,
                                  ui_name = 'correction',
                                  config_name = 'QuanPin/AutoCorrection/Pinyins',
                                  mappings = mappings)

class PunctMapping(MappingOption):
    def __init__(self, name, mappings, owner):
        MappingOption.__init__(self, name, mappings, owner)
        if mappings:
            self.widget.set_sensitive(True)
            self.init_keys_values(mappings)
        else:
            self.widget.set_sensitive(False)
            
    def init_keys_values(self, mappings):
        self.keys = [m[0] for m in mappings]
        values_with_closing = [v or k for k, v in mappings]
        self.values = []
        for v in values_with_closing:
            try:
                self.values.append(v[0])
            except:
                self.values.append(v)
        self.keys.reverse()
        self.values.reverse()

    def get_mappings(self):
        if self.widget.get_active():
            pairs = []
            for k,vs in self.mappings:
                try:
                    for v in vs:
                        pairs.append(':'.join([k,v]))
                except:
                    v = vs
                    if v is None:
                        continue
                    pairs.append(':'.join([k,v]))
            return pairs
        else:
            return []

    def set_active(self, enabled):
        if not self.mappings: return
        if enabled:
            self.widget.set_label('\n'.join(self.values))
        else:
            self.widget.set_label('\n'.join(self.keys))
        self.widget.set_active(enabled)

    is_enabled = property(MappingOption.get_active, set_active)
    
    def key(self):
        for k, v in self.mappings:
            if v is not None:
                return k
        else:
            return None

class PunctMappingSetupDialog (MultiCheckDialog):
    # TODO: the UI should looks like a virtual keyboard,
    #       user are allowed to choose the mappings to all punctuation keys.
    def __init__(self):
        mappings = [MappingInfo('togglebutton1', [('`',None), ('~',u'～')]),
                    MappingInfo('togglebutton2', []),
                    MappingInfo('togglebutton3', [('2',None), ('@',u'＠')]),
                    MappingInfo('togglebutton4', [('3',None), ('#',u'＃')]),
                    MappingInfo('togglebutton5', [('4',None), ('$',u'¥' )]),
                    MappingInfo('togglebutton6', [('5',None), ('%',u'％')]),
                    MappingInfo('togglebutton7', [('6',None), ('^',u'…')]),
                    MappingInfo('togglebutton8', [('7',None), ('&',u'＆')]),
                    MappingInfo('togglebutton9', [('8',None), ('*',u'＊')]),
                    MappingInfo('togglebutton10', [('9',None), ('*',u'（')]),
                    MappingInfo('togglebutton11', [('0',None), ('*',u'）')]),
                    MappingInfo('togglebutton12', [('-',u'－'), ('_',u'——')]),
                    MappingInfo('togglebutton13', [('=',u'＝'), ('+',u'＋')]),
                    MappingInfo('togglebutton14', [('\\',None), ('|',u'‖')]),
                    MappingInfo('togglebutton27', [('[',u'〔'), ('{',u'｛')]),
                    MappingInfo('togglebutton28', [(']',u'〕'), ('}',u'｝')]),
                    MappingInfo('togglebutton39', []),
                    MappingInfo('togglebutton40', []),
                    MappingInfo('togglebutton50', [(',',None), ('<',u'〈')]),
                    MappingInfo('togglebutton51', [('.',u'·'), ('>',u'〉')]),
                    MappingInfo('togglebutton52', [('/',u'／'), ('?',None)])]
                    #'\'',(u'‘',u'’'),
        MultiCheckDialog.__init__(self, ui_name="punctmapping",
                                  config_name="General/PunctMapping/Mappings",
                                  mappings=mappings,
                                  option_klass=PunctMapping)

class MainWindow ():
    def __init__ (self):
        self.__bus = ibus.Bus()
        self.__config = self.__bus.get_config()
        
        
    def run(self):
        self.__init_ui("main_window")
        self.__read_config()
        gtk.main()
        
    def __init_ui(self, name):
        self.__init_gettext()
        glade_file = path.join(path.dirname(__file__), GLADE_FILE)
        self.__xml = glade.XML (glade_file, name)
        self.__init_options()
        self.window = self.__xml.get_widget(name)
        self.__xml.signal_autoconnect(self)
        self.window.show_all()

    def __init_gettext(self):
        locale.setlocale(locale.LC_ALL, "")
        localedir = os.getenv("IBUS_LOCALEDIR")
        gettext.bindtextdomain(GETTEXT_PACKAGE, localedir)
        gettext.bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8")
        glade.bindtextdomain(GETTEXT_PACKAGE, localedir)
        glade.textdomain(GETTEXT_PACKAGE)

    def __init_options(self):
        self.__fuzzy_setup = FuzzySetupDialog()
        self.__correction_setup = CorrectionSetupDialog()
        self.__punctmapping_setup = PunctMappingSetupDialog()
        
        self.__options = [
            TrivalOption("General/MemoryPower", 3, self.__xml),
            TrivalOption("General/PageSize", 10, self.__xml),
            
            RadioOption("General/InitialStatus/Mode", 'Chinese', ['Chinese', 'English'], self.__xml),
            RadioOption("General/InitialStatus/Punct", 'Full', ['Full', 'Half'], self.__xml),
            RadioOption("General/InitialStatus/Letter", 'Half', ['Full', 'Half'], self.__xml),
            RadioOption("General/Charset", 'GBK', ['GB2312', 'GBK', 'GB18030'], self.__xml),
            CheckBoxOption("General/PunctMapping/Enabled", False, self.__xml),
                                
            RadioOption("Keyboard/ModeSwitch", 'Shift', ['Shift', 'Control'], self.__xml),
            RadioOption("Keyboard/PunctSwitch", 'None', ['ControlComma',
                                                         'ControlPeriod',
                                                         'None'], self.__xml),
            CheckBoxOption("Keyboard/Page/MinusEquals", False, self.__xml),
            CheckBoxOption("Keyboard/Page/Brackets", False, self.__xml),
            CheckBoxOption("Keyboard/Page/CommaPeriod", False, self.__xml),
            
            RadioOption("Pinyin/Scheme", 'QuanPin', ['QuanPin', 'ShuangPin'], self.__xml),
            ComboBoxOption("Pinyin/ShuangPinType", 'MS2003', ['MS2003',
                                                              'ABC',
                                                              'ZiRanMa',
                                                              'Pinyin++',
                                                              'ZiGuang',
                                                              'XiaoHe'], self.__xml),
            CheckBoxOption("QuanPin/Fuzzy/Enabled", False, self.__xml),
            CheckBoxOption("QuanPin/AutoCorrection/Enabled", False, self.__xml),
            CheckBoxOption("QuanPin/FuzzySegs/Enabled", False, self.__xml),
            CheckBoxOption("QuanPin/InnerFuzzy/Enabled", False, self.__xml),
            
            self.__fuzzy_setup,
            self.__correction_setup,
            self.__punctmapping_setup,
        ]

    def __get_option(self, name):
        for opt in self.__options:
            if opt.name == name:
                return opt
        else:
            return None
        
    def __read_config(self):
        for opt in self.__options:
            opt.init_ui()
            opt.read_config()
        self.on_chk_fuzzy_enabled_toggled(None)
        self.on_chk_correction_enabled_toggled(None)
        self.on_chk_punctmapping_enabled_toggled(None)
        self.on_radio_shuangpin_toggled(None)
        
    def __write_config(self):
        for opt in self.__options:
            opt.write_config()

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
        
    def on_chk_correction_enabled_toggled(self, button):
        self.__update_enabling_button("QuanPin/AutoCorrection/Enabled",
                                      "button_correction_setup")

    def on_chk_smartseg_enabled_toggled(self, button):
        self.__update_enabling_button("QuanPin/FuzzySegs/Enabled",
                                      "QuanPin/InnerFuzzy/Enabled")

    def on_button_correction_setup_clicked(self, button):
        self.__correction_setup.run()
        
    def on_chk_punctmapping_enabled_toggled(self, button):
        self.__update_enabling_button("General/PunctMapping/Enabled",
                                      "button_punctmapping_setup")
    
    def on_button_punctmapping_setup_clicked(self, button):
        self.__punctmapping_setup.run()
    
    def on_main_ok_clicked(self, button):
        self.__write_config()
        self.__quit()
        
    def on_main_apply_clicked(self, button):
        self.__write_config()

    def on_main_cancel_clicked(self, button):
        self.__quit()

    def __quit(self):
        gtk.main_quit()
    
if __name__ == "__main__":
    MainWindow().run()
