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

#include "sunpinyin_engine_proxy.h"
#include "sunpinyin_engine.h"

extern "C" 
{
    void ibus_sunpinyin_engine_init(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->init();
    }

    void ibus_sunpinyin_engine_destroy(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->destroy();
    }
    
#if defined(WITH_IBUS_1_1_0)
    gboolean ibus_sunpinyin_engine_process_key_event(IBusEngine *engine,
                                                     guint keyval,
                                                     guint state)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        // XXX: use the mapped keyval as its keycode
        return thiz->process_key_event(keyval, keyval, state) ? TRUE : FALSE;
    }
#elif defined(WITH_IBUS_1_2_0)
    gboolean ibus_sunpinyin_engine_process_key_event(IBusEngine *engine,
                                                     guint keyval,
                                                     guint keycode,
                                                     guint modifiers)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        return thiz->process_key_event(keyval, keycode, modifiers) ? TRUE : FALSE;
    }
#else
    #error "Unsuppported IBus version."
#endif
    
    void ibus_sunpinyin_engine_focus_in(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->focus_in();
    }
    
    void ibus_sunpinyin_engine_focus_out(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->focus_out();
    }
    
    void ibus_sunpinyin_engine_reset(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->reset();
    }
    
    void ibus_sunpinyin_engine_enable(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->enable();
    }
    
    void ibus_sunpinyin_engine_disable(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->disable();
    }
    
    void ibus_sunpinyin_engine_page_up(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->page_up();
    }
    
    void ibus_sunpinyin_engine_page_down(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->page_down();
    }

    void ibus_sunpinyin_engine_cursor_up(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->cursor_up();
    }
    
    void ibus_sunpinyin_engine_cursor_down(IBusEngine *engine)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->cursor_down();
    }
    
    void ibus_sunpinyin_engine_property_activate (IBusEngine *engine,
                                                  const gchar *prop_name,
                                                  guint prop_state)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->property_activate(prop_name, prop_state);
    }

    void ibus_sunpinyin_engine_candidate_clicked (IBusEngine *engine,
                                                  guint index,
                                                  guint button,
                                                  guint state)
    {
        SunPinyinEngine *thiz = (SunPinyinEngine *)engine;
        thiz->candidate_clicked(index);
    }
} // extern "C"

