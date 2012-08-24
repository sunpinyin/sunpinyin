// -*- mode: c++ -*-
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

#ifndef SUNPY_IMI_CLASSIC_VIEW_H
#define SUNPY_IMI_CLASSIC_VIEW_H

#include "portability.h"

#include "imi_view.h"

class CIMIClassicView : public CIMIView
{
public:
    CIMIClassicView();
    virtual ~CIMIClassicView();

    virtual void attachIC(CIMIContext* pIC);
    virtual unsigned clearIC(void);

    virtual bool onKeyEvent(const CKeyEvent&);

    virtual void updateWindows(unsigned mask);

    virtual void getPreeditString(IPreeditString& ps);
    virtual void getCandidateList(ICandidateList& cl, int start, int size);

    virtual int  onCandidatePageRequest(int pgno, bool relative);
    virtual int  onCandidateSelectRequest(int index);

    size_t candidateListSize() const
    { return m_candiList.size() + m_sentences.size() + m_tails.size(); }

    void makeSelection(int candiIdx, unsigned& mask);
    void deleteCandidate(int candiIdx, unsigned& mask);

private:
    static size_t top_candidate_threshold;

    unsigned m_cursorFrIdx;
    unsigned m_candiFrIdx;
    unsigned m_candiPageFirst;

    CCandidateList m_uiCandidateList;
    CPreEditString m_uiPreeditString;

    CCandidates m_candiList;
    std::vector<std::pair<int, wstring> > m_sentences;
    std::vector<std::pair<wstring, CCandidates> > m_tails;

    void _insert(unsigned keyvalue, unsigned& mask);
    void _erase(bool backward, unsigned& mask);

    void _getCandidates();

    void _commitChar(TWCHAR ch);
    void _commitString(const wstring& wstr);
    void _doCommit(bool bConvert = true);

    unsigned _moveLeft(unsigned& mask, bool searchAgain = true);
    unsigned _moveLeftSyllable(unsigned& mask, bool searchAgain = true);
    unsigned _moveHome(unsigned& mask, bool searchAgain = true);

    unsigned _moveRight(unsigned& mask);
    unsigned _moveRightSyllable(unsigned& mask);
    unsigned _moveEnd(unsigned& mask);
};

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4
