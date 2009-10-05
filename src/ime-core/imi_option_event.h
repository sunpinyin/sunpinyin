#ifndef _IMI_SESSION_H
#define _IMI_SESSION_H

#include <list>
#include <string>
#include <vector>

#include "utils.h"

class COptionEventBus;

struct COptionEvent
{
    enum Type {
        TYPE_SHARED = (1 << 0),
        TYPE_GLOBAL = (1 << 1)
    };
    template<typename ValueType>
    COptionEvent(const std::string& k, const ValueType& v,
                 int t = TYPE_SHARED|TYPE_GLOBAL)
        : type(t), name(k), value(v) 
    {}
    bool is_shared() const { return type & TYPE_SHARED; }
    bool is_global() const { return type & TYPE_GLOBAL; }
    int get_int() const;
    bool get_bool() const;
    std::string get_string() const;
    std::vector<std::string> get_string_list() const;
    
    /* TODO */
    std::vector<bool> get_bool_list() const;
    std::vector<int> get_int_list() const;
    
    int type;
    std::string name;
    
    struct variant_
    {
        variant_(int);
        variant_(const std::string&);
        variant_(bool);
        variant_(const std::vector<std::string>&);
        struct val_
        {
            int                      d_int;
            std::string              d_string;
            bool                     d_bool;
            std::vector<std::string> d_strings;
        } data;
        enum {
            VAL_UNSIGNED,
            VAL_UNSIGNED_LIST,
            VAL_STRING,
            VAL_STRING_LIST,
            VAL_BOOL,
            VAL_BOOL_LIST
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
    virtual bool onConfigChanged(const COptionEvent& event) { return false; }
    void addRef();
    void release();
    
private:
    int m_refcnt;
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
