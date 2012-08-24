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

#include <assert.h>
#include <stdlib.h>
#include "userdict.h"


bool
CUserDict::load(const char  *fname)
{
    int rc = sqlite3_open(":memory:", &m_db);

    if (rc != SQLITE_OK) {
        sqlite3_close(m_db);
        return false;
    }

    m_fname = strdup(fname);
    rc = _copyDb(Load);

    return _createTable() && _createIndexes();
}


void
CUserDict::free()
{
    if (m_fname) {
        _copyDb(Save);
        ::free(m_fname);
        m_fname = NULL;
    }

    if (m_db) {
        sqlite3_close(m_db);
        m_db = NULL;
    }
}


unsigned
CUserDict::addWord(CSyllables &syllables, const wstring& word)
{
    assert(m_db != NULL);
    assert(syllables.size() >= 2 && syllables.size() <= MAX_USRDEF_WORD_LEN);

    sqlite3_stmt *stmt;
    const char *sql_str =
        "INSERT INTO dict (len, i0, f0, t0, i1, f1, t1, i2, f2, t2, i3, f3, t3, i4, f4, t4, i5, f5, t5, utf8str) \
         VALUES           (?,   ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?,  ?);";
    const char *tail;

    sqlite3_prepare(m_db, sql_str, strlen(sql_str), &stmt, &tail);

    int i = 1;
    sqlite3_bind_int(stmt, i++, syllables.size());

    CSyllables::iterator it = syllables.begin();
    CSyllables::iterator ite = syllables.end();
    for (; it != ite; ++it) {
        sqlite3_bind_int(stmt, i++, it->initial);
        sqlite3_bind_int(stmt, i++, it->final);
        sqlite3_bind_int(stmt, i++, it->tone);
    }

    while (i <= MAX_USRDEF_WORD_LEN * 3 + 1)
        sqlite3_bind_int(stmt, i++, 0);

    char buf[MAX_USRDEF_WORD_LEN * 6 + 1];
    WCSTOMBS(buf, word.c_str(), sizeof(buf) - 1);
    sqlite3_bind_text(stmt, i, (const char*)buf, strlen(buf), NULL);

    unsigned ret = (SQLITE_DONE == sqlite3_step(stmt)) ?
                   INI_USRDEF_WID + sqlite3_last_insert_rowid(m_db) :
                   0;

    sqlite3_finalize(stmt);
    _copyDb(Save);
    return ret;
}


void
CUserDict::removeWord(unsigned wid)
{
    assert(m_db != NULL);
    char    *zErr = NULL;
    char sql[256] = "DELETE FROM dict WHERE id=";

    if (wid > INI_USRDEF_WID) {
        sprintf(sql, "%s%d;", sql, (wid - INI_USRDEF_WID));
        sqlite3_exec(m_db, sql, NULL, NULL, &zErr);

        m_dict.erase(m_dict.find(wid - INI_USRDEF_WID));
    }
}


void
CUserDict::getWords(CSyllables &syllables,
                    std::vector<CPinyinTrie::TWordIdInfo> &result)
{
    assert(m_db != NULL);

    char *sql_str;
    const char *tail;
    std::string i_conditions, f_conditions, t_conditions;
    int length = syllables.size();
    sqlite3_stmt *stmt;
    int rc;
    char buf[256];

    if (length > MAX_USRDEF_WORD_LEN)
        return;

    for (int i = 0; i < length; i++) {
        sprintf(buf, " and i%d=%d", i, syllables[i].initial);
        i_conditions += buf;

        if (!syllables[i].final)
            continue;

        sprintf(buf, " and f%i=%i", i, syllables[i].final);
        f_conditions += buf;

        if (!syllables[i].tone)
            continue;

        sprintf(buf, " and t%i=%i", i, syllables[i].tone);
        t_conditions += buf;
    }

    sql_str = sqlite3_mprintf(
        "SELECT id, utf8str FROM dict WHERE len=%i%q%q%q;",
        length,
        i_conditions.c_str(),
        f_conditions.c_str(),
        t_conditions.c_str());

    rc = sqlite3_prepare(m_db, sql_str, strlen(sql_str), &stmt, &tail);
    if (rc != SQLITE_OK) {
        sqlite3_free(sql_str);
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(m_db));
        return;
    }

    unsigned id = 0;
    TWCHAR cwstr[MAX_USRDEF_WORD_LEN + 1];
    const unsigned char     *utf8str = NULL;
    CPinyinTrie::TWordIdInfo word;

    while (SQLITE_ROW == sqlite3_step(stmt)) {
        id = sqlite3_column_int(stmt, 0);
        utf8str = sqlite3_column_text(stmt, 1);

        if (id >= MAX_USRDEF_WID - INI_USRDEF_WID)
            continue;

        memset(&cwstr[0], 0, sizeof(cwstr));
        MBSTOWCS(cwstr, (const char*)utf8str, MAX_USRDEF_WORD_LEN);

        word.m_id = id + INI_USRDEF_WID;
        word.m_bSeen = 1;
        result.push_back(word);

        m_dict.insert(std::make_pair(id, wstring(cwstr)));
    }

    sqlite3_free(sql_str);
    sqlite3_finalize(stmt);
}


const TWCHAR*
CUserDict::operator [](unsigned wid)
{
    assert(m_db != NULL);

    sqlite3_stmt *stmt = NULL;
    int rc = SQLITE_OK;
    const char *tail;
    char sql_str[256];

    if (wid <= INI_USRDEF_WID || wid > MAX_USRDEF_WID)
        return NULL;

    wid -= INI_USRDEF_WID;

    std::map<unsigned, wstring>::const_iterator it = m_dict.find(wid);
    if (it != m_dict.end())
        return it->second.c_str();

    sprintf(sql_str, "SELECT utf8str FROM dict WHERE id=%d;", wid);

    rc = sqlite3_prepare(m_db, sql_str, strlen(sql_str), &stmt, &tail);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(m_db));
        return NULL;
    }

    const TWCHAR *ret = NULL;
    const unsigned char *utf8str = NULL;
    TWCHAR cwstr[MAX_USRDEF_WORD_LEN + 1];
    if (SQLITE_ROW == sqlite3_step(stmt)) {
        utf8str = sqlite3_column_text(stmt, 0);
        MBSTOWCS(cwstr, (const char*)utf8str, MAX_USRDEF_WORD_LEN);
        wstring wstr(cwstr);
        m_dict.insert(std::make_pair(wid, wstr));
        ret = wstr.c_str();
    }

    sqlite3_finalize(stmt);
    return ret;
}

int
CUserDict::_copyDb(DBCopyDirection direction)
{
    sqlite3 *disk_db;
    int rc = sqlite3_open(m_fname, &disk_db);

    if (rc == SQLITE_OK) {
        sqlite3 *dst = direction == Load ? m_db : disk_db;
        sqlite3 *src = direction == Save ? m_db : disk_db;
        sqlite3_backup *backup = sqlite3_backup_init(dst, "main", src, "main");
        if (backup) {
            sqlite3_backup_step(backup, -1);
            sqlite3_backup_finish(backup);
        }
        rc = sqlite3_errcode(dst);
    }

    sqlite3_close(disk_db);
    return rc;
}

bool
CUserDict::_createTable()
{
    assert(m_db != NULL);

    char *zErr = NULL;
    int rc = SQLITE_OK;
    const char *sql_str =
        "CREATE TABLE IF NOT EXISTS dict( \
         id INTEGER PRIMARY KEY, len INTEGER, \
         i0 INTEGER, i1 INTEGER, i2 INTEGER, i3 INTEGER, i4 INTEGER, i5 INTEGER, \
         f0 INTEGER, f1 INTEGER, f2 INTEGER, f3 INTEGER, f4 INTEGER, f5 INTEGER, \
         t0 INTEGER, t1 INTEGER, t2 INTEGER, t3 INTEGER, t4 INTEGER, t5 INTEGER, \
         utf8str TEXT, UNIQUE (i0, i1, i2, i3, i4, i5, utf8str));";

    rc = sqlite3_exec(m_db, sql_str, NULL, NULL, &zErr);
    if (rc != SQLITE_OK) {
        if (zErr != NULL) {
            fprintf(stderr, "SQL error: %s\n", zErr);
            sqlite3_free(zErr);
        }
        return false;
    }

    return true;
}

bool
CUserDict::_createIndexes()
{
    assert(m_db != NULL);

    char *zErr = NULL;
    int rc = SQLITE_OK;
    const char * sql_str =
        "CREATE INDEX IF NOT EXISTS index_0 ON dict (len, i0, i1, i2, i3, i4, i5);";

    rc = sqlite3_exec(m_db, sql_str, NULL, NULL, &zErr);
    if (rc != SQLITE_OK) {
        if (zErr != NULL) {
            fprintf(stderr, "SQL error: %s\n", zErr);
            sqlite3_free(zErr);
        }
        return false;
    }

    return true;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4
