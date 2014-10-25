/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
 *
 * The contents of this file are subject to the terms of either the GNU Lesser
 * General Public License Version 2.1 only ("LGPL") or the Common Development and
 * Distribution License ("CDDL")(collectively, the "License"). You may not use this
 * file except in compliance with the License. You can obtain a copy of the CDDL at
 * http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
 * http://www.opensource.org/licenses/lgpl-license.php. See the License for the
 * specific language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the full text of the License in the License file as well as the
 * following notice:
 *
 * NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
 * (CDDL)
 * For Covered Software in this distribution, this License shall be governed by the
 * laws of the State of California (excluding conflict-of-law provisions).
 * Any litigation relating to this License shall be subject to the jurisdiction of
 * the Federal Courts of the Northern District of California and the state courts
 * of the State of California, with venue lying in Santa Clara County, California.
 *
 * Contributor(s):
 *
 * If you wish your version of this file to be governed by only the CDDL or only
 * the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
 * include this software in this distribution under the [CDDL or LGPL Version 2.1]
 * license." If you don't indicate a single choice of license, a recipient has the
 * option to distribute your version of this file under either the CDDL or the LGPL
 * Version 2.1, or to extend the choice of license to its licensees as provided
 * above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
 * Version 2 license, then the option applies only if the new code is made subject
 * to such option by the copyright holder.
 */

#ifdef MACOSX
#include <Python/Python.h>
#else
#include <Python.h>
#endif

#include <signal.h>
#include <sstream>

#include "portability.h"
#include "imi_plugin.h"

CIMIPlugin::CIMIPlugin(TPluginTypeEnum pluginType)
    : m_pluginType(pluginType)
{}

CIMIPlugin::~CIMIPlugin()
{}

class CIMIPythonPlugin : public CIMIPlugin
{
public:
    CIMIPythonPlugin(std::string filename);
    virtual ~CIMIPythonPlugin();

    virtual std::string getName() { return m_name; }
    virtual std::string getAuthor() { return m_author; }
    virtual std::string getDescription() { return m_description; }

    virtual TPluginCandidates provide_candidates(const TPluginPreedit& str,
                                                 int* waitTime);
    virtual TPluginCandidate  translate_candidate(const TPluginCandidate& candi,
                                                  int* waitTime);
private:
    PyObject* m_module;
    PyObject* m_provide_method;
    PyObject* m_trans_method;

    std::string m_name;
    std::string m_author;
    std::string m_description;
};

CIMIPythonPlugin::CIMIPythonPlugin(std::string filename)
    : CIMIPlugin(CIMI_PLUGIN_PYTHON), m_module(NULL), m_provide_method(NULL),
      m_trans_method(NULL)
{
    // filename always ends with .py
    std::string module_name = filename.substr(0, filename.length() - 3);
    CIMIPluginManager& manager = AIMIPluginManager::instance();
    PyObject* dict = NULL;
    PyObject* name = NULL;
    PyObject* author = NULL;
    PyObject* description = NULL;

    m_module = PyImport_ImportModule(module_name.c_str());
    if (m_module == NULL) {
        goto error;
    }
    dict = PyModule_GetDict(m_module);
    if (dict == NULL) {
        goto error;
    }
    m_provide_method = PyDict_GetItemString(dict, "provide_candidates");
    m_trans_method = PyDict_GetItemString(dict, "translate_candidate");
    name = PyDict_GetItemString(dict, "__NAME__");
    author = PyDict_GetItemString(dict, "__AUTHOR__");
    description = PyDict_GetItemString(dict, "__DESCRIPTION__");

    if (name != NULL && PyString_Check(name)) {
        m_name = PyString_AsString(name);
    }
    if (author != NULL && PyString_Check(author)) {
        m_author = PyString_AsString(author);
    }
    if (description != NULL && PyString_Check(description)) {
        m_description = PyString_AsString(description);
    }
    return;
error:
    manager.setLastError("Error when loading Python module");
    return;
}

CIMIPythonPlugin::~CIMIPythonPlugin()
{
    Py_XDECREF(m_module);
}

static PyObject*
Py_Call1(PyObject* method, PyObject* obj)
{
    PyObject* args = PyTuple_Pack(1, obj);
    PyObject* ret = PyObject_CallObject(method, args);
    Py_XDECREF(args);
    if (ret == NULL) {
        PyErr_PrintEx(2);
    }
    return ret;
}

static const size_t TWCharBufferSize = 2048;

static wstring
PyUnicode_AsWString(PyObject* obj)
{
    TWCHAR* wide_str_buf = new TWCHAR[TWCharBufferSize];
    wstring res;
    memset(wide_str_buf, 0, sizeof(TWCHAR) * TWCharBufferSize);

    Py_ssize_t size = PyUnicode_AsWideChar((PyUnicodeObject*) obj,
                                           (wchar_t*) wide_str_buf,
                                           TWCharBufferSize);
    if (size > 0) {
        res = wstring(wide_str_buf);
    }
    delete [] wide_str_buf;
    return res;
}

static void
ExtractSequence(TPluginCandidates& result, PyObject* py_seq)
{
    Py_ssize_t len = PySequence_Length(py_seq);
    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject* tuple_item_obj = PySequence_GetItem(py_seq, i);
        if (!PyTuple_Check(tuple_item_obj)) {
            continue;
        }
        PyObject* rank_obj = PyTuple_GetItem(tuple_item_obj, 0);
        PyObject* candi_obj = PyTuple_GetItem(tuple_item_obj, 1);
        if (rank_obj == NULL || !PyInt_Check(rank_obj) || candi_obj == NULL
            || !PyUnicode_Check(candi_obj)) {
            continue;
        }

        result.push_back(TPluginCandidateItem((int) PyInt_AsLong(rank_obj),
                                              PyUnicode_AsWString(candi_obj)));
    }
}

TPluginCandidates
CIMIPythonPlugin::provide_candidates(const TPluginPreedit& str,
                                     int* waitTime)
{
    TPluginCandidates res;
    *waitTime = 0;

    if (m_provide_method == NULL) {
        *waitTime = -1;
        return res;
    }

    PyObject* str_obj = PyUnicode_FromWideChar((wchar_t*) str.c_str(),
                                               str.size());
    PyObject* ret_obj = Py_Call1(m_provide_method, str_obj);

    if (ret_obj == NULL) {
        *waitTime = -1;
    } else if (PyInt_Check(ret_obj)) {
        *waitTime = (int) PyInt_AsLong(ret_obj);
    } else if (PyTuple_Check(ret_obj) && PyTuple_Size(ret_obj) == 2) {
        PyObject* time_obj = PyTuple_GetItem(ret_obj, 0);
        PyObject* seq_obj = PyTuple_GetItem(ret_obj, 1);
        if (PyInt_Check(time_obj) && PyList_Check(seq_obj)) {
            *waitTime = (int) PyInt_AsLong(time_obj);
            ExtractSequence(res, seq_obj);
        }
    } else if (PyList_Check(ret_obj)) {
        // extract all items inside this sequence.
        ExtractSequence(res, ret_obj);
    }
    Py_XDECREF(str_obj);
    Py_XDECREF(ret_obj);
    return res;
}

TPluginCandidate
CIMIPythonPlugin::translate_candidate(const TPluginCandidate& candi,
                                      int* waitTime)
{
    TPluginCandidate res;
    *waitTime = 0;

    if (m_trans_method == NULL) {
        *waitTime = -1;
        return res;
    }

    PyObject* str_obj = PyUnicode_FromWideChar((wchar_t*) candi.c_str(),
                                               candi.size());
    PyObject* ret_obj = Py_Call1(m_trans_method, str_obj);
    if (ret_obj == NULL) {
        *waitTime = -1;
    } else if (PyInt_Check(ret_obj)) {
        *waitTime = (int) PyInt_AsLong(ret_obj);
    } else if (PyUnicode_Check(ret_obj)) {
        res = TPluginCandidate(PyUnicode_AsWString(ret_obj));
    }
    Py_XDECREF(str_obj);
    Py_XDECREF(ret_obj);
    return res;
}

static void
InitializePython()
{
    if (Py_IsInitialized())
        return;
    std::stringstream eval_str;

    // append plugin module path to default load path
    Py_Initialize();
    signal(SIGINT, SIG_DFL);

    PyRun_SimpleString("import sys");
    eval_str << "sys.path.append(r'" << getenv("HOME")
             << "/.sunpinyin/plugins/" << "')";
    PyRun_SimpleString(eval_str.str().c_str());
}

#define PLUGIN_LIST_FILE "/.sunpinyin/plugins.list";
#define PLUGIN_NAME_LEN 128

CIMIPluginManager::CIMIPluginManager()
    : m_hasError(false), m_waitTime(0)
{
    InitializePython();
}

CIMIPluginManager::~CIMIPluginManager()
{
    for (size_t i = 0;  i < m_plugins.size(); i++) {
        delete m_plugins[i];
    }
}

void
CIMIPluginManager::initializePlugins()
{
    // load configuration file which list all needed plugins
    std::string plugin_list_path = getenv("HOME");
    plugin_list_path += PLUGIN_LIST_FILE;
    FILE* fp = fopen(plugin_list_path.c_str(), "r");
    if (!fp) {
        return;
    }
    while (true) {
        char plugin_name[PLUGIN_NAME_LEN];
        memset(plugin_name, 0, PLUGIN_NAME_LEN);
        fgets(plugin_name, PLUGIN_NAME_LEN, fp);
        if (strlen(plugin_name) == 0) {
            break;
        }
        if (strlen(plugin_name) == 1) {
            continue;
        }
        plugin_name[strlen(plugin_name) - 1] = 0; // remove the \n at the end
        if (loadPlugin(plugin_name) == NULL) {
            fprintf(stderr, "Error! Cannot load plugin %s\n", plugin_name);
        }
    }
    fclose(fp);
}

TPluginTypeEnum
CIMIPluginManager::detectPluginType(std::string filename)
{
    if (filename.length() >= 3
        && filename.substr(filename.length() - 3) == ".py") {
        return CIMI_PLUGIN_PYTHON;
    } else {
        return CIMI_PLUGIN_UNKNOWN;
    }
}

CIMIPlugin*
CIMIPluginManager::loadPlugin(std::string filename)
{
    TPluginTypeEnum type = detectPluginType(filename);
    CIMIPlugin* plugin = createPlugin(filename, type);
    std::stringstream error;

    if (plugin == NULL) {
        return NULL;
    }
    if (hasLastError()) {
        delete plugin;
        return NULL;
    }

    for (size_t i = 0; i < m_plugins.size(); i++) {
        if (m_plugins[i]->getName() == plugin->getName()) {
            error << "Plugin " << plugin->getName() << " has already loaded!";
            setLastError(error.str());
            delete plugin; // Reject duplicate plugins
            return NULL;
        }
    }
    m_plugins.push_back(plugin);
    return plugin;
}

CIMIPlugin*
CIMIPluginManager::createPlugin(std::string filename,
                                TPluginTypeEnum pluginType)
{
    std::stringstream error;
    clearLastError();

    switch (pluginType) {
    case CIMI_PLUGIN_PYTHON:
        return new CIMIPythonPlugin(filename);
    case CIMI_PLUGIN_UNKNOWN:
    default:
        error << "Cannot detect type for " << filename;
        setLastError(error.str());
        return NULL;
    }
}

void
CIMIPluginManager::setLastError(std::string desc)
{
    m_hasError = true;
    m_lastError = desc;
}

void
CIMIPluginManager::clearLastError()
{
    m_hasError = false;
    m_lastError = "";
}

void
CIMIPluginManager::markWaitTime(int waitTime)
{
    if (waitTime <= 0)
        return;

    if (m_waitTime == 0) {
        m_waitTime = waitTime;
    } else if (waitTime < m_waitTime) {
        m_waitTime = waitTime;
    }
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4
