#include <cassert>
#include <algorithm>
#include "imi_option_event.h"

COptionEvent::variant_::variant_(int v) 
    : type(VAL_UNSIGNED)
{
    data.d_int = v;
}

COptionEvent::variant_::variant_(const std::string& v)
    : type(VAL_STRING)
{
    data.d_string = v;
}

COptionEvent::variant_::variant_(const std::vector<std::string>& v)
    : type(VAL_STRING_LIST)
{
    data.d_strings = v;
}

COptionEvent::variant_::variant_(bool v)
    : type(VAL_BOOL)
{
    data.d_bool = v;
}

COptionEvent::COptionEvent(const std::string& k, const std::string& v,
                           int t) 
    : type(t), name(k), value(v)
{}

COptionEvent::COptionEvent(const std::string& k, int v,
                           int t) 
    : type(t), name(k), value(v)
{}

COptionEvent::COptionEvent(const std::string& k, bool v,
                           int t) 
    : type(t), name(k), value(v)
{}

int
COptionEvent::get_int() const
{
    return value.data.d_int;
}

bool
COptionEvent::get_bool() const
{
    return value.data.d_bool;
}

std::string
COptionEvent::get_string() const
{
    return value.data.d_string;
}

std::vector<std::string>
COptionEvent::get_string_list() const
{
    return value.data.d_strings;
}

IConfigurable::IConfigurable()
    : m_refcnt(0)
{}

IConfigurable::~IConfigurable()
{
    // assert(m_refcnt == 0);
}

void
IConfigurable::addRef()
{
    if (m_refcnt++ == 0) {
        AOptionEventBus::instance().registerAsListener(this);
    }
}

void
IConfigurable::release()
{
    if (--m_refcnt == 0) {
        AOptionEventBus::instance().unregisterAsListener(this);
    }
}

void
COptionEventBus::registerAsListener(IConfigurable* listener)
{
    Subscribers::iterator  found = find(m_listeners.begin(),
                                        m_listeners.end(),
                                        listener);
    if (found == m_listeners.end())
        m_listeners.push_back(listener);
    else
        assert(false && "already subscribed");
}

void
COptionEventBus::unregisterAsListener(IConfigurable* listener)
{
    Subscribers::iterator  found = find(m_listeners.begin(),
                                        m_listeners.end(),
                                        listener);
    if (found != m_listeners.end())
        m_listeners.erase(found);
    else
        assert(false && "not yet subscribed");
}

void
COptionEventBus::publishEvent(const COptionEvent& event)
{
    Subscribers::iterator end = m_listeners.end();
    for (Subscribers::iterator it = m_listeners.begin();
         it != end; ++it) {
        if ((*it)->onConfigChanged(event))
            break;
    }
}
