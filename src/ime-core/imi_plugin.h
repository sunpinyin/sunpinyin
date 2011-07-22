// -*- mode: c++ -*-

#include <string>
#include <vector>

#include "portability.h"
#include "utils.h"

typedef wstring TPluginPreedit;
typedef wstring TPluginCandidate;

struct TPluginCandidateItem
{
    size_t m_rank;
    TPluginCandidate m_candidate;
    TPluginCandidateItem(size_t rank, TPluginCandidate candidate)
        : m_rank(rank), m_candidate(candidate) {}
};

typedef std::vector<TPluginCandidateItem> TPluginCandidates;

enum TPluginTypeEnum
{
    CIMI_PLUGIN_PYTHON,
    CIMI_PLUGIN_UNKNOWN
};

class CIMIPlugin
{
private:
    TPluginTypeEnum m_pluginType;
protected:
    CIMIPlugin(TPluginTypeEnum pluginType);
public:
    virtual ~CIMIPlugin();
    virtual TPluginCandidates provide_candidates(const TPluginPreedit& str,
                                                 int* waitTime) = 0;
    virtual TPluginCandidate  translate_candidate(const TPluginCandidate& candi,
                                                  int* waitTime) = 0;

    virtual std::string getName() = 0;
    virtual std::string getAuthor() = 0;
    virtual std::string getDescription() = 0;

    TPluginTypeEnum getPluginType() const { return m_pluginType; }
};

class CIMIPluginManager
{
public:
    CIMIPluginManager();
    virtual ~CIMIPluginManager();

    TPluginTypeEnum detectPluginType(std::string filename);
    CIMIPlugin*     loadPlugin(std::string filename);
    CIMIPlugin*     createPlugin(std::string filename,
                                 TPluginTypeEnum pluginType);
    size_t          getPluginSize() const { return m_plugins.size(); }
    CIMIPlugin*     getPlugin(size_t i) { return m_plugins[i]; }

    bool        hasLastError() const { return m_hasError; }
    std::string getLastError() const { return m_lastError; }
    void        setLastError(std::string desc);
    void        clearLastError();

    void        markWaitTime(int waitTime);
    int         getWaitTime() const { return m_waitTime; }
    void        resetWaitTime() { m_waitTime = 0; }
private:
    std::vector<CIMIPlugin*> m_plugins;

    bool        m_hasError;
    std::string m_lastError;
    int         m_waitTime;
};

typedef SingletonHolder<CIMIPluginManager> AIMIPluginManager;
