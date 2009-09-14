#include "sunpinyin_engine_proxy.h"
#include "sunpinyin_engine.h"
#include "sunpinyin_config.h"
#include "engine.h"

struct IBusSunPinyinEngineClass {
    IBusEngineClass parent;
};

typedef SunPinyinEngine IBusSunPinyinEngine;

/* functions prototype */
extern "C" 
{
    static void ibus_sunpinyin_engine_class_init (IBusSunPinyinEngineClass *);
    static GObject* ibus_sunpinyin_engine_constructor (GType type,
                                                       guint n_construct_params,
                                                       GObjectConstructParam *construct_params);
}

static IBusEngineClass *parent_class = NULL;

GType
ibus_sunpinyin_engine_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusSunPinyinEngineClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_sunpinyin_engine_class_init,
        NULL,
        NULL,
        sizeof (IBusSunPinyinEngine),
        0,
        (GInstanceInitFunc) ibus_sunpinyin_engine_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_ENGINE,
                                       "IBusSunPinyinEngine",
                                       &type_info,
                                       (GTypeFlags) 0);
    }

    return type;
}

// load sunpinyin configuration
void
ibus_sunpinyin_init(IBusBus *bus)
{
    IBusConfig *config = ibus_bus_get_config(bus);
    SunPinyinConfig::set_config(config);
}

void
ibus_sunpinyin_exit()
{}

extern "C" 
{
    
    // initialize the meta class object
    void
    ibus_sunpinyin_engine_class_init (IBusSunPinyinEngineClass *klass)
    {
        GObjectClass *object_class = G_OBJECT_CLASS(klass);
        IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
        IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);
        
        parent_class = (IBusEngineClass *) g_type_class_peek_parent (klass);
        
        object_class->constructor  = ibus_sunpinyin_engine_constructor;
        ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_sunpinyin_engine_destroy;
        
        engine_class->process_key_event = ibus_sunpinyin_engine_process_key_event;
        engine_class->focus_in          = ibus_sunpinyin_engine_focus_in;
        engine_class->focus_out         = ibus_sunpinyin_engine_focus_out;
        engine_class->reset             = ibus_sunpinyin_engine_reset;
        engine_class->enable            = ibus_sunpinyin_engine_enable;
        engine_class->disable           = ibus_sunpinyin_engine_disable;
        engine_class->focus_in          = ibus_sunpinyin_engine_focus_in;
        engine_class->focus_out         = ibus_sunpinyin_engine_focus_out;
        engine_class->page_up           = ibus_sunpinyin_engine_page_up;
        engine_class->page_down         = ibus_sunpinyin_engine_page_down;
        engine_class->property_activate = ibus_sunpinyin_engine_property_activate;
        engine_class->candidate_clicked = ibus_sunpinyin_engine_candidate_clicked;
    }
    
    // allocate a new ibus engine
    GObject*
    ibus_sunpinyin_engine_constructor (GType type,
                                       guint n_construct_params,
                                       GObjectConstructParam  *construct_params)
    {
        
        IBusSunPinyinEngine *engine = (IBusSunPinyinEngine *)
            G_OBJECT_CLASS (parent_class)->constructor (type,
                                                        n_construct_params,
                                                        construct_params);
        engine->set_parent_class(parent_class);
        
        const gchar *engine_name = ibus_engine_get_name ((IBusEngine *) engine);
        g_assert (engine_name);
        // 
        return (GObject *)engine;
    }
} // extern "C"

