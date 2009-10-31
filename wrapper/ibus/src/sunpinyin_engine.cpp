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

#include "engine_impl.h"
#include "sunpinyin_engine.h"

SunPinyinEngine::SunPinyinEngine()
    : m_impl(NULL)
{}

SunPinyinEngine::~SunPinyinEngine()
{}

void
SunPinyinEngine::init ()
{
    m_impl = new EngineImpl(this);
}

void
SunPinyinEngine::destroy ()
{
    delete m_impl;
    IBUS_OBJECT_CLASS (m_parent)->destroy ((IBusObject *)this);
}

void
SunPinyinEngine::set_parent_class(IBusEngineClass *klass)
{
    m_parent = klass;
}

gboolean
SunPinyinEngine::process_key_event (guint key_val,
                                    guint key_code,
                                    guint modifiers)
{
    if (!m_impl->is_valid()) return FALSE;
    return m_impl->process_key_event(key_val, key_code, modifiers);
}

void
SunPinyinEngine::focus_in ()
{
    if (!m_impl->is_valid()) return;

    m_impl->focus_in();
    m_parent->focus_in(this);
}

void
SunPinyinEngine::focus_out ()
{
    if (!m_impl->is_valid()) return;

    m_impl->focus_out();
    m_parent->focus_out(this);
}

void
SunPinyinEngine::reset ()
{
    if (!m_impl->is_valid()) return;
    
    m_impl->reset();
    m_parent->reset(this);
}

void
SunPinyinEngine::enable ()
{
    if (!m_impl->is_valid()) return;
    
    m_impl->enable();
    m_parent->enable(this);
}

void
SunPinyinEngine::disable ()
{
    if (!m_impl->is_valid()) return;

    m_impl->disable();
    m_parent->disable(this);
}

void
SunPinyinEngine::page_up ()
{
    if (!m_impl->is_valid()) return;

    m_impl->page_up();
    m_parent->page_up(this);
}

void
SunPinyinEngine::page_down ()
{
    if (!m_impl->is_valid()) return;

    m_impl->page_down();
    m_parent->page_down(this);
}

void
SunPinyinEngine::cursor_up ()
{
    if (!m_impl->is_valid()) return;

    m_impl->cursor_up();
    m_parent->cursor_up(this);
}

void
SunPinyinEngine::cursor_down ()
{
    if (!m_impl->is_valid()) return;

    m_impl->cursor_down();
    m_parent->cursor_down(this);
}


void
SunPinyinEngine::property_activate (const std::string& property, unsigned state)
{
    if (!m_impl->is_valid()) return;

    m_impl->property_activate(property, state);
    m_parent->property_activate(this, property.c_str(), state);
}

void
SunPinyinEngine::candidate_clicked (guint index)
{
    m_impl->candidate_clicked(index);
    m_parent->candidate_clicked(this, index, 0, 0);
}
