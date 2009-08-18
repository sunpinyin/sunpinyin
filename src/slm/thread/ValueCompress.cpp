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
#include "config.h"
#endif

#include <stdio.h>

#include "ValueCompress.h"

#ifdef HAVE_LIMIT_H
#include <limit.h>
#endif

#ifndef DBL_MAX
#define DBL_MAX (1E+300)
#endif

struct TVCArrItem {
    float       m_val;
    unsigned    m_heapIdx;

    TVCArrItem(float val=0.0, unsigned idx=0)
        : m_val(val), m_heapIdx(idx) { }
};

struct TVCHeapItem {
    unsigned    m_first;
    unsigned    m_last;
    unsigned    m_count;
    double      m_appval;
    double      m_sum;
    double      m_dist;

    bool
    operator< (const TVCHeapItem& b) const
        { return m_appval < b.m_appval; }

    TVCHeapItem(unsigned first=0, unsigned last=0, unsigned count=0,
                double val=0.0, double sum=0.0, double dist=0.0)
        : m_first(first), m_last(last), m_count(count),
          m_appval(val), m_sum(sum), m_dist(dist) { }
};

typedef std::vector<TVCArrItem>     CVCArr;
typedef std::vector<TVCHeapItem>    CVCHeap;


static void
BubbleUpVal(CVCHeap& heap, CVCArr& arr, int idx)
{
    while (idx > 0) {
        int parent= (idx-1)/2;
        if (heap[idx] < heap[parent])
            break;
        for (int h=heap[idx].m_first, t=heap[idx].m_last; h < t; ++h)
            arr[h].m_heapIdx = parent;
        for (int h=heap[parent].m_first, t=heap[parent].m_last; h < t; ++h)
            arr[h].m_heapIdx = idx;
        TVCHeapItem hitem = heap[parent];
        heap[parent] = heap[idx];
        heap[idx] = hitem;
        idx = parent;
    }
}

static void
IronDownVal(CVCHeap& heap, CVCArr& arr, int idx,  int bottom)
{
    int left;
    while ((left = 2 * idx + 1) < bottom) {
        int max = idx;
        if (heap[max] < heap[left])
            max = left;
        if (left+1 < bottom && heap[max] < heap[left+1])
            max = left + 1;
        if (max == idx) break;

        for (int h=heap[idx].m_first, t=heap[idx].m_last; h < t; ++h)
            arr[h].m_heapIdx = max;
        for (int h=heap[max].m_first, t=heap[max].m_last; h < t; ++h)
            arr[h].m_heapIdx = idx;
        TVCHeapItem hitem = heap[max];
        heap[max] = heap[idx];
        heap[idx] = hitem;

        idx = max;
    }
}


/**
 * Bubble idx up according to distance
 */
static void
BubbleUp(CVCHeap& heap, CVCArr& arr, int idx)
{
    while (idx > 0) {
        int parent= (idx-1)/2;
        if (heap[parent].m_dist <= heap[idx].m_dist)
            break;
        for (int h=heap[idx].m_first, t=heap[idx].m_last; h < t; ++h)
            arr[h].m_heapIdx = parent;
        for (int h=heap[parent].m_first, t=heap[parent].m_last; h < t; ++h)
            arr[h].m_heapIdx = idx;
        TVCHeapItem hitem = heap[parent];
        heap[parent] = heap[idx];
        heap[idx] = hitem;
        idx = parent;
    }
}

/**
 * Iron idx down, but do not let it lower than bottom (< bottom)
 */
static void
IronDown(CVCHeap& heap, CVCArr& arr, int idx,  int bottom)
{
    int left;
    while ((left = 2 * idx + 1) < bottom) {
        int min = idx;
        if (heap[left].m_dist < heap[min].m_dist)
            min = left;
        if (left+1 < bottom && heap[left+1].m_dist < heap[min].m_dist)
            min = left + 1;
        if (min == idx) break;

        for (int h=heap[idx].m_first, t=heap[idx].m_last; h < t; ++h)
            arr[h].m_heapIdx = min;
        for (int h=heap[min].m_first, t=heap[min].m_last; h < t; ++h)
            arr[h].m_heapIdx = idx;
        TVCHeapItem hitem = heap[min];
        heap[min] = heap[idx];
        heap[idx] = hitem;

        idx = min;
    }
}

void
CValueCompressor::operator()(std::map<float, int>& values,
                             std::map<float, int>& map,
                             std::vector<float>& table,
                             unsigned N) const
{
    CVCArr  arr;
    CVCHeap heap;

    std::map<float, int>::const_iterator itv  = values.begin();
    std::map<float, int>::const_iterator itve = values.end();
    for (; itv != itve; ++itv) {
        arr.push_back(TVCArrItem(itv->first, arr.size()));
        double sum = double(itv->first);
        if (itv->second > 0)
            sum *= itv->second;
        heap.push_back(TVCHeapItem(heap.size(), heap.size()+1, itv->second, itv->first, sum));
    }

    for (int i=0, sz=heap.size()-1; i < sz; ++i) {
        if (heap[i].m_count == 0 || heap[i+1].m_count == 0) {
            heap[i].m_dist = DBL_MAX;
        } else {
            heap[i].m_dist = heap[i+1].m_appval - heap[i].m_appval;
        }
        BubbleUp(heap, arr, i);
    }
    if (heap.size() > 0) {
        heap[heap.size()-1].m_dist = DBL_MAX;
        BubbleUp(heap, arr, heap.size()-1);
    }

    int cur, prev, next, hiprev, hinext;

    while (heap.size() > N) {
        cur = heap[0].m_first;
        if (cur == 0) {
            prev = hiprev = -1;
        } else {
            hiprev = arr[cur-1].m_heapIdx;
            prev = heap[hiprev].m_first;
        }
        next = heap[0].m_last;
        hinext = arr[next].m_heapIdx;

        for (int h=cur; h < next; ++h)
            arr[h].m_heapIdx = hinext;
        double newval = (heap[0].m_sum + heap[hinext].m_sum)/
                            (heap[0].m_count + heap[hinext].m_count);
        if (hiprev >= 0)
            heap[hiprev].m_dist += (newval - heap[0].m_appval);
        heap[hinext].m_first = heap[0].m_first;
        heap[hinext].m_count += heap[0].m_count;
        heap[hinext].m_sum += heap[0].m_sum;
        heap[hinext].m_dist += (heap[hinext].m_appval - newval);
        heap[hinext].m_appval = newval;

        if (hiprev > hinext)
            cur = hiprev, hiprev = hinext, hinext = cur;
        IronDown(heap, arr, hinext, heap.size());
        if (hiprev > 0)
            IronDown(heap, arr, hiprev, heap.size());

        heap[0] = heap[heap.size()-1];
        for (int h=heap[0].m_first, t = heap[0].m_last; h < t; ++h)
            arr[h].m_heapIdx = 0;
        heap.pop_back();
        IronDown(heap, arr, 0, heap.size());
    }

    for (int i=1, sz=heap.size(); i < sz; ++i)
        BubbleUpVal(heap, arr, i);
    for (int i = heap.size()-1; i > 0; --i) {
        for (int h=heap[0].m_first, t=heap[0].m_last; h < t; ++h)
            arr[h].m_heapIdx = i;
        for (int h=heap[i].m_first, t=heap[i].m_last; h < t; ++h)
            arr[h].m_heapIdx = 0;
        TVCHeapItem hitem = heap[0];
        heap[0] = heap[i];
        heap[i] = hitem;
        IronDownVal(heap, arr, 0, i);
    }

    map.clear();
    for (int i = 0, sz = arr.size(); i < sz; ++i)
        map[arr[i].m_val] = arr[i].m_heapIdx;

    table.clear();
    table.reserve(heap.size());
    for (int i = 0, sz = heap.size(); i < sz; ++i)
        table.push_back(float(heap[i].m_appval));

    /*
    for (int i = 0, sz = heap.size(); i < sz; ++i) {
        printf("%12lf:\n", heap[i].m_appval);
        for (int h = heap[i].m_first, t = heap[i].m_last; h < t; ++h) {
            printf("    %12f\n", arr[h].m_val);
            if (arr[h].m_heapIdx != i) {
                printf("error, non-consistence found\n");
                return;
            }
        }
        printf("\n");
    }
    */
}


void
CValueCompressor::operator()(std::map<float, float>& eff2val,
          std::map<float, int>& values,
          std::map<float, int>& v2idx,
          std::vector<float>& table,
          unsigned N) const
{
    std::map<float, int> tmp_map;
    this->operator()(values, tmp_map, table, N);

    v2idx.clear();
    std::map<float, int>::iterator itm = tmp_map.begin();
    std::map<float, int>::iterator itme = tmp_map.end();
    for (; itm != itme; ++itm) {
        v2idx[eff2val[itm->first]] = itm->second;
    }

/* // Can not be maped back, because some value could not be in the eff2val maps
    std::vector<float>::iterator itt = table.begin();
    std::vector<float>::iterator itte = table.end();
    for (; itt != itte; ++itt)
        *itt = eff2val[*itt];
*/
}





