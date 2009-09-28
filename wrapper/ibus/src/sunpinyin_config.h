/*
 * Copyright (c) 2009 Kov Chai <tchaikov@gmail.com>
 *
 * The contents of this file are subject to the terms of either the GNU Lesser
 * General Public License Version 2.1 only ("LGPL") or the Common Development and
 * Distribution License ("CDDL")(collectively, the "License"). You may not use this
 * file except in compliance with the License. You can obtain a copy of the CDDL at
 * http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
 * http://www.opensource.org/licenses/lgpl-license.php. See the License for the 
 * specific language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the full text of the License in the License file as well as the
 * following notice:
 * 
 * NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
 * (CDDL)
 * For Covered Software in this distribution, this License shall be governed by the
 * laws of the State of California (excluding conflict-of-law provisions).
 * Any litigation relating to this License shall be subject to the jurisdiction of
 * the Federal Courts of the Northern District of California and the state courts
 * of the State of California, with venue lying in Santa Clara County, California.
 * 
 * Contributor(s):
 * 
 * If you wish your version of this file to be governed by only the CDDL or only
 * the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
 * include this software in this distribution under the [CDDL or LGPL Version 2.1]
 * license." If you don't indicate a single choice of license, a recipient has the
 * option to distribute your version of this file under either the CDDL or the LGPL
 * Version 2.1, or to extend the choice of license to its licensees as provided
 * above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
 * Version 2 license, then the option applies only if the new code is made subject
 * to such option by the copyright holder. 
 */

#ifndef SUNPINYIN_CONFIG_H
#define SUNPINYIN_CONFIG_H

#include <map>
#include <string>
#include <ibus.h>
#include <imi_options.h>
#include <imi_option_event.h>

class EngineImpl;

class SunPinyinConfig
{
    typedef std::map<std::string,
        CSunpinyinSessionFactory::EPyScheme> SchemeNames;
    typedef std::map<std::string,
        EShuangpinType> TypeNames;

    SchemeNames        m_scheme_names;
    TypeNames          m_type_names;

    static IBusConfig *m_config;
    
public:
    SunPinyinConfig();
    ~SunPinyinConfig();
    
    bool get(const char* key, bool val);
    void set(const char* key, bool val);
    
    unsigned get(const char* key, unsigned val);
    void set(const char* key, unsigned val);
    
    std::string get(const char *key, const std::string& default_val);
    void set(const char* key, const std::string& val);

    CSunpinyinSessionFactory::EPyScheme get_py_scheme(CSunpinyinSessionFactory::EPyScheme);
    void set_py_scheme(CSunpinyinSessionFactory::EPyScheme);

    EShuangpinType get_shuangpin_type(EShuangpinType);
    void set_shuangpin_type(EShuangpinType);
    
    /**
     * gets called in ibus_sunpinyin_init() so that SunPinyinEngine can read 
     * configuration when it starts up
     */
    static void set_config(IBusConfig *);

    /**
     * register on_config_value_changed() as the signal handler of value-changed,
     */
    void listen_on_changed();

private:    
    /**
     * called by ibus when a value changed in config
     */
    static void on_config_value_changed(IBusConfig *config,
                                        const gchar *section,
                                        const gchar *name,
                                        GValue *value,
                                        gpointer user_data);

private:
    std::string get_scheme_name(CSunpinyinSessionFactory::EPyScheme scheme);
    CSunpinyinSessionFactory::EPyScheme get_scheme(const std::string& name);
    std::string get_type_name(EShuangpinType);
    EShuangpinType get_type(const std::string& name);
};

#endif // SUNPINYIN_CONFIG_H