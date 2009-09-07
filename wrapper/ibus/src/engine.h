#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>

#define IBUS_TYPE_SUNPINYIN_ENGINE \
    (ibus_sunpinyin_engine_get_type ())

GType   ibus_sunpinyin_engine_get_type    (void);

#endif  /* __ENGINE_H__ */
