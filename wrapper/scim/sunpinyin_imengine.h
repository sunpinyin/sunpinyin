/*
 * Copyright (c) 2007 Kov Chai <tchaikov@gmail.com>
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

#ifndef SUNPINYIN_IMENGINE_H
#define SUNPINYIN_IMENGINE_H

using namespace scim;

class CHotkeyProfile;

class SunPyFactory : public IMEngineFactoryBase
{
    ConfigPointer       m_config;
    bool                m_valid;
    WideString          m_name;
    Connection          m_reload_signal_connection;
    CHotkeyProfile     *m_hotkey_profile;

    friend class SunPyInstance;
    
public:
    SunPyFactory (const ConfigPointer &config);

    virtual ~SunPyFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
    
public:
    bool valid () const { return m_valid; }
    void reload_config(const ConfigPointer& config);
    
private:
    bool init ();
    bool load_user_config ();
};

class SunPyInstance : public IMEngineInstanceBase
{
    SunPyFactory        *m_factory;
    CIMIView            *m_pv;
    CScimWinHandler     *m_wh;
    CHotkeyProfile      *m_hotkey_profile;
    SunLookupTable      *m_lookup_table;
    
    Connection           m_reload_signal_connection;
    bool                 m_focused;
    
  public:
    SunPyInstance(SunPyFactory *factory,
                  CHotkeyProfile *hotkey_profile,
                  const String& encoding, int id);
    virtual ~SunPyInstance();
    /**
     * - mode switch key
     * - toggle full width punctuation mode
     * - toggle full width letter mode
     * - chinese/english switch
     * - caret left/right/home/end
     * - candidate table cursor_up/cursor_down/page_up/page_down/number_select
     * - backspace/delete
     * - space/enter
     */
    virtual bool process_key_event (const KeyEvent& key);
    virtual void move_preedit_caret (unsigned int pos);
    virtual void select_candidate (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up ();
    virtual void lookup_table_page_down ();
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();
    /**
     * update the configuration of the input method
     */
    virtual void trigger_property (const String &property);

public:
    /**
     * expose this inherited protected method
     * so that CScimWinHandler can call it
     */
    using IMEngineInstanceBase::commit_string;
    
    void refresh_status_property(bool cn);
    void refresh_fullsymbol_property(bool full);
    void refresh_fullpunc_property(bool full);
    void redraw_preedit_string(const IPreeditString* ppd);
    void redraw_lookup_table(const ICandidateList* pcl);

private:
    void create_session(CHotkeyProfile*);
    void destroy_session();
    
    void init_lookup_table_labels ();
    void reload_config (const ConfigPointer &config);
    void refresh_all_properties ();
    void initialize_all_properties();
    
    AttributeList build_preedit_attribs(const IPreeditString* ppd);

    void lookup_page_up();
    void lookup_page_down();
};

// emacs: -*- c++-mode -*-
#endif//SUNPINYIN_IMENGINE_H
