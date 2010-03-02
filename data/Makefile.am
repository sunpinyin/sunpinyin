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

INSTALL_FILES = lm_sc.t3g.be lm_sc.t3g.le pydict_sc.bin.be pydict_sc.bin.le
ICON_FILES = sunpinyin_logo.xpm \
	han.png eng.png cnpunc.png enpunc.png fullwidth.png halfwidth.png \
	han.svg eng.svg cnpunc.svg enpunc.svg fullwidth.svg halfwidth.svg \
	setup.svg

EXTRA_DIST = $(ICON_FILES)

LM_SC_ARPA = $(top_builddir)/@RAW_DATA_DIR@/lm_sc.t3g.arpa
DICTFILE = $(top_builddir)/@RAW_DATA_DIR@/dict.utf8
PYTRIE_LOG = pydict_sc.log.utf8
LM_SC_BIN = lm_sc.t3g

noinst_PROGRAMS = tslmpack genpyt

SOURCE_DIR = $(top_builddir)/@SOURCE_DIR@
SLM_SRC_DIR = $(top_builddir)/@SLM_SRC_DIR@
LEXICON_SRC_DIR = $(top_builddir)/@LEXICON_SRC_DIR@
PINYIN_SRC_DIR = $(top_builddir)/@PINYIN_SRC_DIR@

tslmpack_SOURCES = \
	${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h \
	${SLM_SRC_DIR}/thread/ValueCompress.cpp ${SLM_SRC_DIR}/thread/ValueCompress.h \
	${SLM_SRC_DIR}/tslmpack/slmpack.cpp ${SLM_SRC_DIR}/tslmpack/common.h \
	${SLM_SRC_DIR}/tslmpack/arpa_conv.cpp ${SLM_SRC_DIR}/tslmpack/arpa_conv.h \
	${SLM_SRC_DIR}/tslmpack/arpa_slm.cpp ${SLM_SRC_DIR}/tslmpack/arpa_slm.h

genpyt_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h \
	${LEXICON_SRC_DIR}/genPYT.cpp ${LEXICON_SRC_DIR}/pytrie.cpp ${LEXICON_SRC_DIR}/pytrie.h \
	${LEXICON_SRC_DIR}/pytrie_gen.cpp ${LEXICON_SRC_DIR}/pytrie_gen.h \
	${PINYIN_SRC_DIR}/pinyin_data.cpp ${PINYIN_SRC_DIR}/pinyin_data.h \
	${PINYIN_SRC_DIR}/syllable.h

genpyt_CXXFLAGS = ${DEFAULT_INCLUDES} -I${PINYIN_SRC_DIR}

$(LM_SC_BIN): tslmpack $(LM_SC_ARPA) $(DICTFILE)
	./tslmpack $(LM_SC_ARPA) $(DICTFILE) $@

pydict_sc.bin: genpyt $(DICTFILE) $(LM_SC_BIN)
	./genpyt $(DICTFILE) $@ $(PYTRIE_LOG) $(LM_SC_BIN)

noinst_DATA = $(LM_SC_BIN) pydict_sc.bin 
CLEANFILES = $(LM_SC_BIN) pydict_sc.bin tslmpack genpyt
