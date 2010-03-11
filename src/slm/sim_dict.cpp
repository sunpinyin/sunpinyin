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

#include <stdio.h>
#include <stdlib.h>

#include "sim_dict.h"


void CSIMDict::freeSubTree(CSIMDict::TState& root)
{
        if (root.follow != NULL) {
                Map_Type &map = *(root.follow);
                for (Map_Type::iterator it=map.begin(), last=map.end(); it != last; ++it)
                        freeSubTree(it->second);
                delete root.follow;
                root.follow = NULL;
        }
}

const CSIMDict::TState* CSIMDict::step(const CSIMDict::TState* root, TWCHAR wch)
{
        if ((root != NULL) && (root->follow != NULL) && wch != WCH_NULL) {
                Map_Type::iterator it = root->follow->find(TSIMChar(wch));
                if (it != root->follow->end())
                        return &(it->second);
        }
        return NULL;
}

int	CSIMDict::matchLongest(const CSIMDict::TState* root, CSIMDict::PState &  result, const TWCHAR* str)
{
        int lastWordLen = 0, len = 0;
        result = root;
        while (root != NULL) {
                if (root->word_id != SIM_ID_NOT_WORD) {
                        result = root;
                        lastWordLen = len;
                }
                ++len;
                root = step(root, *str++);
        }
        return lastWordLen;
}

bool
CSIMDict::parseText(const char* filename)
{
    FILE * fp = NULL;
    static char buf[1024];
    static TWCHAR wword[sizeof(buf)];
    unsigned int id;

    try {
        if ((fp = fopen(filename, "r")) == NULL)
          return false;
        while (fgets(buf, 1024, fp) != NULL) {
            if (*buf == '\n' || *buf == '#')
                continue;

            char* p = buf;
            while (*p == ' ' || *p == '\t')
                ++p;
            char* pstart = p;
            while (*p != 0 && *p != ' ' && *p != '\t')
                ++p;
            if (*p == 0)
                continue;
            *p++ = 0;
            while (*p == ' ' || *p == '\t')
                ++p;
            if (!(*p >= '0' && *p <= '9')) continue;
            for (id=0; *p >= '0' && *p <= '9'; ++p)
                id = 10*id + (*p - '0');

            if (id < SIM_ID_REALWORD_START)
                continue;
            if (MBSTOWCS(wword, pstart, sizeof(buf)) != (size_t)-1) {
                insertWord(wword, TSIMWordId(id));
            } else {
                fprintf(stderr, "mbs to wcs conversion error for : %s %d\n", buf, id);
                exit(100);
            }
        }
        fclose(fp);
    } catch (...) {
        if (fp != NULL)
            fclose(fp);
        buf[sizeof(buf)-1] = 0;
        fprintf(stderr, "Catch exception when loading dictionary at %s, exiting...", buf);
        exit(200);
    }
    return true;
}

void CSIMDict::insertWord(const TWCHAR* wstr, TSIMWordId id)
{
        TState* ps = &m_root;
        while (*wstr) {
                TSIMChar ch(*wstr++);
                TSIMWordId nodeId = (*wstr)?SIM_ID_NOT_WORD:id;
                if (ps->follow == NULL) {
                        ps->follow = new Map_Type();
                }
                Map_Type & map = *(ps->follow);
                Map_Type::iterator it = map.find(ch);
                if (it != map.end() && nodeId != SIM_ID_NOT_WORD &&
                    it->second.word_id != SIM_ID_NOT_WORD && it->second.word_id != nodeId) {
                        throw new int(100);
                }
                if (it != map.end()){
                        if (nodeId != SIM_ID_NOT_WORD)
                                it->second.word_id = nodeId;
                        ps = &(it->second);
                } else {
                        ps = &(map[ch] = TState(nodeId));
                }
        }
}

void CSIMDict::InnerPrint(FILE* fp, wstring & wstr, const TState* pnode)
{
        if (pnode != NULL && pnode->word_id != SIM_ID_NOT_WORD) {
                char* buf = new char[wstr.size()*2+2];
                WCSTOMBS(buf, wstr.c_str(), wstr.size()*2+2);
                fprintf(fp, "%s %d\n", buf, unsigned(pnode->word_id));
                delete[] buf;
        }
        if (pnode != NULL && pnode->follow != NULL) {
                Map_Type::iterator it, ite = pnode->follow->end();
                for (it = pnode->follow->begin(); it != ite; ++it) {
                        TWCHAR wch = TWCHAR(it->first);
                        wstr.push_back(wch);
                        InnerPrint(fp, wstr, &(it->second));
                        wstr.erase(wstr.size()-1, 1);
                }
        }
}
