# 
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
# 
# Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
# 
# The contents of this file are subject to the terms of either the GNU Lesser
# General Public License Version 2.1 only ("LGPL") or the Common Development and
# Distribution License ("CDDL")(collectively, the "License"). You may not use this
# file except in compliance with the License. You can obtain a copy of the CDDL at
# http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
# http://www.opensource.org/licenses/lgpl-license.php. See the License for the 
# specific language governing permissions and limitations under the License. When
# distributing the software, include this License Header Notice in each file and
# include the full text of the License in the License file as well as the
# following notice:
# 
# NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
# (CDDL)
# For Covered Software in this distribution, this License shall be governed by the
# laws of the State of California (excluding conflict-of-law provisions).
# Any litigation relating to this License shall be subject to the jurisdiction of
# the Federal Courts of the Northern District of California and the state courts
# of the State of California, with venue lying in Santa Clara County, California.
# 
# Contributor(s):
# 
# If you wish your version of this file to be governed by only the CDDL or only
# the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
# include this software in this distribution under the [CDDL or LGPL Version 2.1]
# license." If you don't indicate a single choice of license, a recipient has the
# option to distribute your version of this file under either the CDDL or the LGPL
# Version 2.1, or to extend the choice of license to its licensees as provided
# above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
# Version 2 license, then the option applies only if the new code is made subject
# to such option by the copyright holder. 
# 
SRC_DIR = $(top_srcdir)/src
SUNPINYIN_SRC_DIR = $(top_srcdir)/src/ime-core
SLM_SRC_DIR = $(top_srcdir)/src/slm
LEXICON_SRC_DIR = $(top_srcdir)/src/lexicon
PINYIN_SRC_DIR = $(top_srcdir)/src/pinyin

GTK_CFLAGS=`pkg-config --cflags gtk+-2.0`
GTK_LIBS=`pkg-config --libs gtk+-2.0`

noinst_PROGRAMS = sunpinyin

sunpinyin_SOURCES = \
	$(SRC_DIR)/portability.cpp $(SRC_DIR)/portability.h \
        $(SLM_SRC_DIR)/slm.cpp $(SLM_SRC_DIR)/slm.h \
        $(LEXICON_SRC_DIR)/pytrie.cpp $(LEXICON_SRC_DIR)/pytrie.h \
        $(PINYIN_SRC_DIR)/pinyin_data.cpp $(PINYIN_SRC_DIR)/pinyin_data.h \
	$(PINYIN_SRC_DIR)/pinyin_seg.cpp $(PINYIN_SRC_DIR)/pinyin_seg.h \
	$(PINYIN_SRC_DIR)/shuangpin_data.cpp $(PINYIN_SRC_DIR)/shuangpin_data.h \
	$(PINYIN_SRC_DIR)/shuangpin_seg.cpp $(PINYIN_SRC_DIR)/shuangpin_seg.h \
        $(SUNPINYIN_SRC_DIR)/imi_context.cpp $(SUNPINYIN_SRC_DIR)/imi_context.h \
        $(SUNPINYIN_SRC_DIR)/imi_data.cpp $(SUNPINYIN_SRC_DIR)/imi_data.h \
        $(SUNPINYIN_SRC_DIR)/lattice_states.cpp $(SUNPINYIN_SRC_DIR)/lattice_states.h \
        $(SUNPINYIN_SRC_DIR)/imi_view.cpp $(SUNPINYIN_SRC_DIR)/imi_view.h \
	$(SUNPINYIN_SRC_DIR)/imi_uiobjects.cpp $(SUNPINYIN_SRC_DIR)/imi_uiobjects.h \
        $(SUNPINYIN_SRC_DIR)/imi_keys.h \
        $(SUNPINYIN_SRC_DIR)/imi_view_classic.cpp $(SUNPINYIN_SRC_DIR)/imi_view_classic.h \
	$(SUNPINYIN_SRC_DIR)/imi_winHandler.cpp $(SUNPINYIN_SRC_DIR)/imi_winHandler.h \
	$(SUNPINYIN_SRC_DIR)/ic_history.cpp $(SUNPINYIN_SRC_DIR)/ic_history.h \
	$(SUNPINYIN_SRC_DIR)/imi_funcobjs.cpp $(SUNPINYIN_SRC_DIR)/imi_funcobjs.h \
	$(SUNPINYIN_SRC_DIR)/imi_options.cpp $(SUNPINYIN_SRC_DIR)/imi_options.h \
	$(SUNPINYIN_SRC_DIR)/imi_option_event.cpp $(SUNPINYIN_SRC_DIR)/imi_option_keys.h \
	$(SUNPINYIN_SRC_DIR)/userdict.cpp $(SUNPINYIN_SRC_DIR)/userdict.h \
        imi_gtkwin.cpp imi_gtkwin.h sunpinyin.cpp

sunpinyin_CXXFLAGS = -DGTK_STANDALONE -D_USE_RAW_PROBABILITY -DSUNPINYIN_DATA_DIR=\"../../data\"\
                     -I$(top_srcdir) -I$(SUNPINYIN_SRC_DIR) -I$(LEXICON_SRC_DIR) -I$(SLM_SRC_DIR) -I$(PINYIN_SRC_DIR)\
                     -I$(top_builddir) -I$(top_builddir)/src \
                     $(GTK_CFLAGS) $(sqlite3_CFLAGS) 

sunpinyin_LDFLAGS = ${GTK_LIBS} $(sqlite3_LIBS)

sunpinyin_LDADD = -lm 

all-local :
	if [ -L data ] ; then unlink data; fi
	if [ -L images ]; then unlink images; fi
	ln -s ${srcdir}/../../data data
	ln -s ${srcdir}/../../data images

clean-local :
	if [ -L data ] ; then unlink data; fi
	if [ -L images ]; then unlink images; fi
	rm -f *.rpo

