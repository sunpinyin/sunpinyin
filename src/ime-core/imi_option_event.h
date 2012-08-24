// -*- mode: c++ -*-
#ifndef _IMI_SESSION_H
#define _IMI_SESSION_H

#include <list>
#include <string>
#include <vector>

#include "utils.h"

class COptionEventBus;

struct COptionEvent {
    template<typename ValueType>COptionEvent(const std::string& k,
                                             const ValueType& v)
        : name(k), value(v)
    {}

    int get_int() const;
    bool get_bool() const;
    std::string get_string() const;
    std::vector<std::string> get_string_list() const;
    std::vector<string_pair> get_string_pair_list() const;

    /* TODO:
     * string_pair get_string_pair() const;
     * std::vector<bool> get_bool_list() const;
     * std::vector<int> get_int_list() const;
     */

    int type;
    std::string name;

    struct variant_ {
        variant_(int);
        variant_(const std::string&);
        variant_(bool);
        variant_(const std::vector<std::string>&);
        variant_(const std::vector<string_pair>&);
        struct val_ {
            int d_int;
            std::string d_string;
            bool d_bool;
            std::vector<std::string> d_strings;
            std::vector<string_pair> d_string_pair_list;
        } data;
        enum {
            VAL_INTEGER,
            VAL_INTEGER_LIST,
            VAL_STRING,
            VAL_STRING_LIST,
            VAL_STRING_PAIR,
            VAL_STRING_PAIR_LIST,
            VAL_BOOL,
            VAL_BOOL_LIST,
        } type;
    } value;
};

class IConfigurable
{
public:
    IConfigurable();
    ~IConfigurable();
    /**
     * onConfigChanged will be called whenever an option is changed
     * @param event presents the changed option
     * @return true if the event is consumed, and not intented to be
     *         sent to another event listener, false otherwise.
     */
    virtual bool onConfigChanged(const COptionEvent&) { return false; }
};

class COptionEventBus
{
public:
    /**
     * listener will receive a message whenever an option is changed
     * @param listener who is interested in a change of options
     */
    void registerAsListener(IConfigurable* listener);

    /**
     * remove listener from the subscriber list
     * @param listener who is no more interested in a change of options
     */
    void unregisterAsListener(IConfigurable* listener);

    /**
     * publish an event to all listeners of this event bus
     * @param event the event which presents an option change
     */
    void publishEvent(const COptionEvent& event);
    template<class> friend class SingletonHolder;

private:
    COptionEventBus() {}

    typedef std::list<IConfigurable*> Subscribers;
    Subscribers m_listeners;
};

typedef SingletonHolder<COptionEventBus> AOptionEventBus;

#endif // _IMI_SESSION_H

// -*- indent-tabs-mode: nil -*- vim:et:ts=4
