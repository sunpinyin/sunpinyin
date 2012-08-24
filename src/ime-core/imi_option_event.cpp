#include <cassert>
#include <algorithm>
#include "imi_option_event.h"

COptionEvent::variant_::variant_(int v)
    : type(VAL_INTEGER)
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

COptionEvent::variant_::variant_(const std::vector<string_pair>& v)
    : type(VAL_STRING_PAIR_LIST)
{
    data.d_string_pair_list = v;
}

COptionEvent::variant_::variant_(bool v)
    : type(VAL_BOOL)
{
    data.d_bool = v;
}

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

std::vector<string_pair>
COptionEvent::get_string_pair_list() const
{
    return value.data.d_string_pair_list;
}

IConfigurable::IConfigurable()
{
    AOptionEventBus::instance().registerAsListener(this);
}

IConfigurable::~IConfigurable()
{
    AOptionEventBus::instance().unregisterAsListener(this);
}

void
COptionEventBus::registerAsListener(IConfigurable* listener)
{
    Subscribers::iterator found = find(m_listeners.begin(),
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
    Subscribers::iterator found = find(m_listeners.begin(),
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

// -*- indent-tabs-mode: nil -*- vim:et:ts=4
