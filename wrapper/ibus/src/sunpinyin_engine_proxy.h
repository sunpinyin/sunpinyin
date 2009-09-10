#ifndef SUNPINYIN_ENGINE_PROXY_H
#define SUNPINYIN_ENGINE_PROXY_H

#include <ibus.h>
#include "config.h"

extern "C"
{
    void ibus_sunpinyin_engine_init(IBusEngine *engine);
    void ibus_sunpinyin_engine_destroy(IBusEngine *engine);
#if defined(WITH_IBUS_1_1_0)
    gboolean ibus_sunpinyin_engine_process_key_event(IBusEngine *engine,
                                                     guint keyval,
                                                     guint state);
#elif defined(WITH_IBUS_1_2_0)
    gboolean ibus_sunpinyin_engine_process_key_event(IBusEngine *engine,
                                                     guint keyval,
                                                     guint keycode,
                                                     guint modifiers);
#else
    #error "Unsuppported IBus version."
#endif
    void ibus_sunpinyin_engine_focus_in(IBusEngine *engine);
    void ibus_sunpinyin_engine_focus_out(IBusEngine *engine);
    void ibus_sunpinyin_engine_reset(IBusEngine *engine);
    void ibus_sunpinyin_engine_enable(IBusEngine *engine);
    void ibus_sunpinyin_engine_disable(IBusEngine *engine);
    void ibus_sunpinyin_engine_focus_in(IBusEngine *engine);
    void ibus_sunpinyin_engine_focus_out(IBusEngine *engine);
    void ibus_sunpinyin_engine_page_up(IBusEngine *engine);
    void ibus_sunpinyin_engine_page_down(IBusEngine *engine);
    void ibus_sunpinyin_engine_property_activate (IBusEngine *engine,
                                                  const gchar *prop_name,
                                                  guint prop_state);
    void ibus_sunpinyin_engine_candidate_clicked(IBusEngine *engine,
                                                 guint index,
                                                 guint button,
                                                 guint state);
}

#endif // SUNPINYIN_ENGINE_PROXY_H
