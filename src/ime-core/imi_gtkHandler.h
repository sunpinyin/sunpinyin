// -*- mode: c++ -*-

#include <glib.h>
#include "imi_winHandler.h"
#include "imi_view.h"

static inline gboolean
UpdateCallback(gpointer ptr)
{
    CIMIView* view = (CIMIView*) ptr;
    view->getWinHandler()->doneDeferedUpdate();
    view->updateWindows();
    return FALSE;
}

// implement gtk loop mechanism
class CIMIGtkHandler : public CIMIWinHandler
{
public:
    CIMIGtkHandler() : m_sourceId(0) {}

    virtual ~CIMIGtkHandler() {
        disableDeferedUpdate();
    }

    virtual void enableDeferedUpdate(CIMIView* view, int waitTime) {
        if (m_sourceId == 0 && waitTime > 0) {
            m_sourceId = g_timeout_add_seconds(waitTime, UpdateCallback, view);
        }
    }

    virtual void disableDeferedUpdate() {
        if (m_sourceId != 0) {
            g_source_remove(m_sourceId);
            m_sourceId = 0;
        }
    }

    virtual void doneDeferedUpdate() {
        m_sourceId = 0;
    }

private:
    guint m_sourceId;
};
