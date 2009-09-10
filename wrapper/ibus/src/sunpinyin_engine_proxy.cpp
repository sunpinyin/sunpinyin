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

