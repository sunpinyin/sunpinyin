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

    COptionEvent(const std::string& name, const std::string& val,
                 unsigned type = TYPE_SHARED|TYPE_GLOBAL);
    COptionEvent(const std::string& name, unsigned val,
                 unsigned type = TYPE_SHARED|TYPE_GLOBAL);
    COptionEvent(const std::string& name, bool  val,
                 unsigned type = TYPE_SHARED|TYPE_GLOBAL);
    COptionEvent(const std::string& name, const std::vector<std::string>& val,
                 unsigned type = TYPE_SHARED|TYPE_GLOBAL);
    bool is_shared() const { return type & TYPE_SHARED; }
    bool is_global() const { return type & TYPE_GLOBAL; }
    unsigned get_unsigned() const;
    bool get_bool() const;
    std::string get_string() const;
    std::vector<std::string> get_string_list() const;
    
    /* TODO */
    std::vector<bool> get_bool_list() const;
    std::vector<int> get_int_list() const;
    
    unsigned type;
    std::string name;
    
    struct variant_
    {
        variant_(unsigned);
        variant_(const std::string&);
        variant_(bool);
        variant_(const std::vector<std::string>&);
        struct val_
        {
            unsigned                 d_unsigned;
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
