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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "imi_view_classic.h"
#include "imi_uiobjects.h"

#include "imi_keys.h"

CIMIClassicView::CIMIClassicView()
    :CIMIView(), m_cursorFrIdx(0), m_candiFrIdx(0),
     m_candiPageFirst(0), m_numeric_mode(false)
    { }

CIMIClassicView::~CIMIClassicView()
    { }

void
CIMIClassicView::attachIC(CIMIContext* pIC)
{
    CIMIView::attachIC(pIC);
    clearIC ();
}

unsigned
CIMIClassicView::clearIC(void)
{
    if (!m_pIC->isEmpty()) {
        m_cursorFrIdx = m_candiFrIdx = m_candiPageFirst = 0;
        m_numeric_mode = false;

        m_pIC->clear ();
        m_pPySegmentor->clear ();
        m_candiList.clear ();
        m_tailSentence.clear ();
        return PREEDIT_MASK | CANDIDATE_MASK;
    }
    return 0;
}

void
CIMIClassicView::updateWindows(unsigned mask)
{
    if (!m_pWinHandler)
        return;

    if (mask & PREEDIT_MASK) {
        CPreEditString ps;
        getPreeditString (ps);
        m_pWinHandler->updatePreedit (&ps);
    }

    if ((mask & PREEDIT_MASK) || (mask & CANDIDATE_MASK)) {
        unsigned word_num = m_pIC->getBestSentence (m_tailSentence, m_candiFrIdx);
        if (word_num <= 1 || (!m_candiList.empty() && m_tailSentence == m_candiList[0].m_cwstr))
            m_tailSentence.clear ();
    }

    if (mask & CANDIDATE_MASK) {
        CCandidateList cl;
        getCandidateList (cl, m_candiPageFirst, m_candiWindowSize);
        m_pWinHandler->updateCandidates (&cl);
    }
}

bool
CIMIClassicView::onKeyEvent(const CKeyEvent& key)
{
    unsigned changeMasks = 0;

    unsigned keycode = key.code;
    unsigned keyvalue = key.value;
    unsigned modifiers = key.modifiers;
    
#ifdef DEBUG
    printf("Classic View got a key (0x%x-0x%x-0x%x)...", 
           keycode, keyvalue, modifiers);
    if (((modifiers & IM_CTRL_MASK) != 0) && (keyvalue == 'P' || keyvalue=='p'))
        m_pIC->printLattice();
#endif

    if (m_pHotkeyProfile && m_pHotkeyProfile->isModeSwitchKey(key)) {
        setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, (!m_bCN)?1:0);
        if (!m_pIC->isEmpty ()) {
            changeMasks |= CANDIDATE_MASK | PREEDIT_MASK;
            clearIC ();
        }
        
    } else if (m_pHotkeyProfile && m_pHotkeyProfile->isPunctSwitchKey(key)) {
        // On CTRL+. switch Full/Half punc
        changeMasks |= KEYEVENT_USED;
        setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC, (!m_bFullPunct)?1:0);
        
    } else if (m_pHotkeyProfile && m_pHotkeyProfile->isSymbolSwitchKey(key)) {
        // On SHIFT+SPACE switch Full/Half symbol
        changeMasks |= KEYEVENT_USED;
        setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL, (!m_bFullSymbol)?1:0);
        
    } else if (modifiers == IM_CTRL_MASK && keycode == IM_VK_LEFT)  { // move left
        if (!m_pIC->isEmpty ()) {
            changeMasks |= KEYEVENT_USED;
            _moveLeft (changeMasks);
        }
        
    } else if (modifiers == IM_CTRL_MASK && keycode == IM_VK_RIGHT) { // move right
        if (!m_pIC->isEmpty ()) {
            changeMasks |= KEYEVENT_USED;
            _moveRight (changeMasks);
        }
    } else if ( ((modifiers == 0 && keycode == IM_VK_PAGE_UP) ||
                 (m_pHotkeyProfile && m_pHotkeyProfile->isPageUpKey (key))) &&
                !m_pIC->isEmpty() ) {
        changeMasks |= KEYEVENT_USED;
        int sz = m_candiList.size() + ((m_tailSentence.size() > 0)?1:0);
        if (sz > 0 && m_candiPageFirst > 0) {
            m_candiPageFirst -= m_candiWindowSize;
            if (m_candiPageFirst < 0) m_candiPageFirst = 0;
            changeMasks |= CANDIDATE_MASK;
        }
    } else if ( ((modifiers == 0 && keycode == IM_VK_PAGE_DOWN) ||
                 (m_pHotkeyProfile && m_pHotkeyProfile->isPageDownKey (key))) &&
                !m_pIC->isEmpty() ) {
        changeMasks |= KEYEVENT_USED;
        int sz = m_candiList.size() + ((m_tailSentence.size() > 0)?1:0);
        if (sz > 0 && m_candiPageFirst + m_candiWindowSize < sz) {
            m_candiPageFirst += m_candiWindowSize;
            changeMasks |= CANDIDATE_MASK;
        }
    }
    else if (m_pHotkeyProfile && m_pHotkeyProfile->isCandiDeleteKey(key, m_candiWindowSize) &&
             !m_pIC->isEmpty ()) {
        changeMasks |= KEYEVENT_USED;
        unsigned sel = (keyvalue == '0'? 9: keyvalue-'1');        
        _deleteCandidate (sel, changeMasks);
        goto PROCESSED;

    } else if ((modifiers & (IM_CTRL_MASK | IM_ALT_MASK | IM_SUPER_MASK | IM_RELEASE_MASK)) == 0) {
        if ((keyvalue >= '0' && keyvalue <= '9') &&
                   (m_candiWindowSize >= 10 || keyvalue < ('1' + m_candiWindowSize))) { // try to make selection
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED;
                unsigned sel = (keyvalue == '0'? 9: keyvalue-'1');
                _makeSelection (sel, changeMasks);
            } else {
                m_numeric_mode = true;
            }

            goto PROCESSED;
        } 
        
        if (keyvalue == '.' && m_numeric_mode) {
            m_numeric_mode = false;
            goto PROCESSED;

        }
        
        m_numeric_mode = false;

        if (keyvalue > 0x60 && keyvalue < 0x7b) { // islower(keyvalue)
            changeMasks |= KEYEVENT_USED;
            _insert (keyvalue, changeMasks);

        } else if (keyvalue > 0x20 && keyvalue < 0x7f) { //isprint(keyvalue) && !isspace(keyvalue)
            changeMasks |= KEYEVENT_USED;
            if (m_pIC->isEmpty ()) {
                _insert (keyvalue, changeMasks);
                _doCommit ();
                clearIC ();
            } else {
                _insert (keyvalue, changeMasks);
            }
            
        } else if (keycode == IM_VK_BACK_SPACE || keycode == IM_VK_DELETE) {
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED;
                _erase (keycode == IM_VK_BACK_SPACE, changeMasks);
            }

        } else if (keycode == IM_VK_SPACE) {
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED;
                _makeSelection (0, changeMasks);
            } else {
                wstring wstr = (m_pIC->fullPuncOp()) (keyvalue);
                if (wstr.size()) {
                    _commitString (wstr);
                    changeMasks |= KEYEVENT_USED;
                }
            }

        } else if (keycode == IM_VK_ENTER) {
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED | CANDIDATE_MASK | PREEDIT_MASK;
                _doCommit (false);
                clearIC ();
            }

        } else if (keycode == IM_VK_ESCAPE) {
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED | CANDIDATE_MASK | PREEDIT_MASK;
                clearIC ();
            }

        } else if (keycode == IM_VK_LEFT) { // move left syllable
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED;
                _moveLeftSyllable (changeMasks);
            }

        } else if (keycode == IM_VK_RIGHT) { // move right syllable
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED;
                _moveRightSyllable (changeMasks);
            }

        } else if (keycode == IM_VK_HOME) { // move home
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED;
                _moveHome (changeMasks);
            }

        } else if (keycode == IM_VK_END) { // move end
            if (!m_pIC->isEmpty ()) {
                changeMasks |= KEYEVENT_USED;
                _moveEnd (changeMasks);
            }
        }
    } else {
        goto RETURN;

    } 

PROCESSED:;
    m_pHotkeyProfile->rememberLastKey(key);

RETURN:;

#ifdef DEBUG
    printf("   |-->(Mask=0x%x)\n", changeMasks);
#endif

    updateWindows (changeMasks);
    return changeMasks & KEYEVENT_USED;
}

int
CIMIClassicView::onCandidatePageRequest(int pgno, bool relative)
{
    unsigned changeMasks = 0;
    int      ncandi, lastpgidx;

    if (!m_pIC->isEmpty()) {
        changeMasks |= KEYEVENT_USED;
        int sz = m_candiList.size() + ((m_tailSentence.size() > 0)?1:0);
        if (sz > 0) {
           lastpgidx = (sz-1)/m_candiWindowSize * m_candiWindowSize;
           if (relative == true) {
                ncandi = m_candiPageFirst + pgno*m_candiWindowSize;
                if (ncandi >= sz)
                   ncandi = lastpgidx;
                if (ncandi < 0)
                    ncandi =0;
                if (ncandi != m_candiPageFirst) {
                    m_candiPageFirst = ncandi;
                    changeMasks |= CANDIDATE_MASK;
                }
            } else {
                if (pgno == -1) { //last page
                    ncandi = lastpgidx;
                } else {
                    ncandi = pgno * m_candiWindowSize;
                    if (ncandi > lastpgidx)
                        ncandi = lastpgidx;
                }
                if (ncandi != m_candiPageFirst) {
                    m_candiPageFirst = ncandi;
                    changeMasks |= CANDIDATE_MASK;
                }
            }
        }
    }

    updateWindows(changeMasks);
    return 0;
}

int
CIMIClassicView::onCandidateSelectRequest(int index)
{
    unsigned changeMasks = 0;

    if (!m_pIC->isEmpty())
        _makeSelection(index, changeMasks);

    updateWindows(changeMasks);
    return 0;
}

void
CIMIClassicView::getPreeditString(IPreeditString& ps)
{
    ps.clear();

    wstring &wstr = ps.getString ();
    IPreeditString::CCharTypeVec& charTypes = ps.getCharTypeVec ();

    m_pIC->getBestSentence (wstr, 0, m_candiFrIdx);

    int caret = wstr.size ();
    charTypes.reserve (caret);
    for (int i = 0; i < caret; ++i)
        charTypes.push_back (IPreeditString::HANZI_CHAR | IPreeditString::USER_CHOICE);

    const wstring& pystr = m_pPySegmentor->getInputBuffer ();
    std::vector<unsigned>& seg_path = m_pIC->getBestSegPath();

    if (pystr.empty())
        return;

    std::vector<unsigned>::iterator it  = seg_path.begin() + 1;
    std::vector<unsigned>::iterator ite = seg_path.end();

    CLattice& lattice = m_pIC->getLattice ();
    unsigned i = 0, l = 0;
    for (; it != ite; i=*(it++)) {
        l = *it - i;

        if (*it <= m_candiFrIdx)
            continue;

        if (i < m_cursorFrIdx && m_cursorFrIdx <= i+l)
            caret = wstr.size() +  (m_cursorFrIdx-i);

        CLatticeFrame &fr = lattice [i+l];
        int ct = IPreeditString::PINYIN_CHAR;
        if (fr.isSyllableSepFrame()) {
            ct = IPreeditString::BOUNDARY | IPreeditString::USER_CHOICE;
        } else if (fr.m_type == CLatticeFrame::ASCII) {
            ct = IPreeditString::ASCII_CHAR;
        } else if (fr.m_type == CLatticeFrame::SYMBOL) {
            ct = IPreeditString::SYMBOL_CHAR;
        }

        wstr.insert (wstr.end(), pystr.begin()+i, pystr.begin()+i+l);
        for (int c=0; c<l; ++c)
            charTypes.push_back (ct);

        if (fr.isSyllableFrame() && !fr.isSyllableSepFrame()) {
            if ( it != ite -1 && !lattice[i+l+1].isSyllableSepFrame ()) { 
                wstr.push_back (' ');
                charTypes.push_back (IPreeditString::BOUNDARY);
            }
        }
    }

    ps.setCaret (caret);
}

void
CIMIClassicView::getCandidateList(ICandidateList& cl, int start, int size)
{
    cl.clear();
    cl.reserve(size);

    int tscount = (m_tailSentence.size() > 0)? 1: 0;
    cl.setFirst (start);
    cl.setTotal (tscount + m_candiList.size());

    ICandidateList::CCandiStrings& css = cl.getCandiStrings ();
    ICandidateList::CCandiTypeVec& cts = cl.getCandiTypeVec ();

    //Loop used for future n-best sentence candidates usage
    for (; start < tscount && size > 0; ++start, --size) {
        css.push_back (m_tailSentence);
        cts.push_back (ICandidateList::BEST_TAIL);
    }

    start -= tscount;
    for (int sz=m_candiList.size(); start < sz && size > 0; ++start, --size) {
        css.push_back (m_candiList[start].m_cwstr);
        cts.push_back ((start == 0)?(ICandidateList::BEST_WORD):(ICandidateList::NORMAL_WORD));
    }
}

void
CIMIClassicView::_insert (unsigned keyvalue, unsigned &changeMasks)
{
    changeMasks |= KEYEVENT_USED;

    if (m_pPySegmentor->getInputBuffer().size() >= MAX_LATTICE_LENGTH-1)
        return;

    if (m_cursorFrIdx == m_pIC->getLastFrIdx ())
        m_pPySegmentor->push (keyvalue);
    else
        m_pPySegmentor->insertAt (m_cursorFrIdx, keyvalue);

    m_cursorFrIdx ++;

    if (m_pIC->buildLattice (m_pPySegmentor))
        _getCandidates ();

    changeMasks |= PREEDIT_MASK | CANDIDATE_MASK;
}

void
CIMIClassicView::_erase (bool backward, unsigned &changeMasks)
{
    if (backward) {
        // if possible to cancel the last selection
        if (m_backspaceCancel) {
            if (m_candiFrIdx > 0) {
                changeMasks |= CANDIDATE_MASK | PREEDIT_MASK | KEYEVENT_USED;
                m_candiFrIdx = m_pIC->cancelSelection(m_candiFrIdx, true);
                _getCandidates();
                return;
            }
        }
        if (m_cursorFrIdx == m_pIC->getLastFrIdx())
            m_pPySegmentor->pop();
        else if (m_cursorFrIdx > 0)
            m_pPySegmentor->deleteAt(m_cursorFrIdx-1, backward);
        else
            return;
        _moveLeft(changeMasks, true);
    } else {
        if (m_cursorFrIdx < m_pIC->getLastFrIdx ())
            m_pPySegmentor->deleteAt(m_cursorFrIdx-1, backward);
        else
            return;
    }
    
    if (m_pIC->buildLattice (m_pPySegmentor))
        _getCandidates ();

    changeMasks |= PREEDIT_MASK | CANDIDATE_MASK | KEYEVENT_USED;
}

void
CIMIClassicView::_getCandidates ()
{
    m_candiPageFirst = 0;
    m_pIC->getCandidates (m_candiFrIdx, m_candiList);
}

void
CIMIClassicView::_commitChar (TWCHAR ch)
{
    TWCHAR wa[2] = {ch, 0};
    m_pWinHandler->commit(wa);
}

void
CIMIClassicView::_commitString (const wstring& wstr)
{
    m_pWinHandler->commit(wstr.c_str());
}

void
CIMIClassicView::_doCommit (bool bConvert)
{
    wstring bs;

    if (bConvert) {
        m_pIC->memorize ();
        m_pIC->getBestSentence (bs);
        m_pWinHandler->commit (bs.c_str());
    } else {
        bs += m_pPySegmentor->getInputBuffer ();
        m_pWinHandler->commit (bs.c_str());
    }
}

unsigned
CIMIClassicView::_moveLeft (unsigned& mask, bool searchAgain)
{
    if (m_cursorFrIdx == 0)
        return _moveEnd (mask);

    mask |= PREEDIT_MASK;
    if (m_cursorFrIdx == m_candiFrIdx) {
        mask |= CANDIDATE_MASK;
        m_candiFrIdx = m_pIC->cancelSelection (m_candiFrIdx, searchAgain);
        _getCandidates ();
    }

    return --m_cursorFrIdx;
}

unsigned
CIMIClassicView::_moveLeftSyllable (unsigned& mask, bool searchAgain)
{
    if (m_cursorFrIdx == 0)
        return _moveEnd (mask);

    mask |= PREEDIT_MASK;

    if (m_cursorFrIdx == m_candiFrIdx) {
        mask |= CANDIDATE_MASK;
        m_candiFrIdx = m_pIC->cancelSelection (m_candiFrIdx, searchAgain);
        _getCandidates ();
    }

    std::vector<unsigned>& seg_path = m_pIC->getBestSegPath();
    std::vector<unsigned>::iterator it = std::upper_bound (seg_path.begin(), seg_path.end(), m_cursorFrIdx-1);
    return m_cursorFrIdx = *(--it);
}

unsigned
CIMIClassicView::_moveHome (unsigned& mask, bool searchAgain)
{
    if (m_cursorFrIdx == 0)
        return 0;

    mask |= PREEDIT_MASK;

    if (m_candiFrIdx != 0) {
        std::vector<unsigned>& best_path    = m_pIC->getBestPath();
        std::vector<unsigned>::iterator it  = best_path.begin();
        std::vector<unsigned>::iterator ite = best_path.end();
        CLattice& lattice = m_pIC->getLattice ();

        for (; it != ite; ++it) {
            if (lattice[*it].m_bwType & CLatticeFrame::USER_SELECTED)
                m_pIC->cancelSelection (*it, false);
        }

        mask |= CANDIDATE_MASK;
        m_candiFrIdx = 0;
        _getCandidates ();

        if (searchAgain) m_pIC->searchFrom ();
    }

    m_cursorFrIdx = 0;
    return 0;
}

unsigned
CIMIClassicView::_moveRight (unsigned& mask)
{
    if (m_cursorFrIdx < m_pIC->getLastFrIdx ()) {
        mask |= PREEDIT_MASK;
        ++ m_cursorFrIdx;
        return m_cursorFrIdx;
    }

    return _moveHome (mask);
}

unsigned
CIMIClassicView::_moveRightSyllable (unsigned& mask)
{
    if (m_cursorFrIdx < m_pIC->getLastFrIdx ()) {
        mask |= PREEDIT_MASK;

        std::vector<unsigned>& seg_path = m_pIC->getBestSegPath();
        std::vector<unsigned>::iterator it = std::upper_bound (seg_path.begin(), seg_path.end(), m_cursorFrIdx);
        m_cursorFrIdx = *it;
        return m_cursorFrIdx;
    } 

    return _moveHome (mask);
}

unsigned
CIMIClassicView::_moveEnd (unsigned& mask)
{
    if (m_cursorFrIdx < m_pIC->getLastFrIdx ()) {
        mask |= PREEDIT_MASK;
        m_cursorFrIdx = m_pIC->getLastFrIdx ();
    }

    return m_cursorFrIdx;
}

void
CIMIClassicView::_makeSelection (int candiIdx, unsigned& mask)
{
    candiIdx += m_candiPageFirst;
    if (!m_tailSentence.empty ()) --candiIdx;

    if (candiIdx < 0) {
        // commit the best sentence
        mask |= PREEDIT_MASK | CANDIDATE_MASK;
        _doCommit ();
        clearIC ();
    } else if (candiIdx < m_candiList.size ()) {
        mask |= PREEDIT_MASK | CANDIDATE_MASK;
        CCandidate& candi = m_candiList [candiIdx];
        m_pIC->makeSelection (candi);
        m_candiFrIdx = candi.m_end;
        if (m_cursorFrIdx < m_candiFrIdx) m_cursorFrIdx = m_candiFrIdx;

        CLattice& lattice = m_pIC->getLattice ();
        while (m_candiFrIdx < m_pIC->getLastFrIdx () && 
               !lattice[m_candiFrIdx+1].isUnusedFrame () &&
               !lattice[m_candiFrIdx+1].isSyllableFrame ()) {
            ++ m_candiFrIdx;
            lattice[m_candiFrIdx].m_bwType |= CLatticeFrame::IGNORED;
        }

        if (m_candiFrIdx == m_pIC->getLastFrIdx ()) {
            _doCommit ();
            clearIC ();
        } else {
            m_candiPageFirst = 0;
            _getCandidates ();
        }
    } else if (candiIdx == 0 && m_candiList.size() == 0) {
        // user might delete all the left over pinyin characters, this will
        // make m_candiList empty
        // 0 or space choices should commit previous selected candidates
        mask |= PREEDIT_MASK | CANDIDATE_MASK;
        _doCommit();
        clearIC();
    }
}
    
void
CIMIClassicView::_deleteCandidate (int candiIdx, unsigned& mask)
{
    candiIdx += m_candiPageFirst;
    if (!m_tailSentence.empty ()) --candiIdx;

    if (candiIdx < 0) {
        // try to remove candidate 0 which is a calculated sentence
        std::vector<unsigned> wids;
        m_pIC->getBestSentence (wids, m_candiFrIdx);
        m_pIC->removeFromHistoryCache(wids);
    } else {
        // remove an ordinary candidate
        CCandidate& candi = m_candiList [candiIdx];
        m_pIC->deleteCandidate(candi);
    }

    _getCandidates ();
    mask |= PREEDIT_MASK | CANDIDATE_MASK;
}
