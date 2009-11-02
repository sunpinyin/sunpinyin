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
SOURCE_DIR = $(top_builddir)/@SOURCE_DIR@
SLM_SRC_DIR = $(top_builddir)/@SLM_SRC_DIR@
LEXICON_SRC_DIR = $(top_builddir)/@LEXICON_SRC_DIR@
PINYIN_SRC_DIR = $(top_builddir)/@PINYIN_SRC_DIR@

CORPUS_DIR = $(top_builddir)/@RAW_DATA_DIR@
SWAP_DIR = $(top_builddir)/@SWAP_DATA_DIR@
RESULT_DIR = $(top_builddir)/@RESULT_DATA_DIR@

if DEBUG
AM_CXXFLAGS = -g -O0 -DDEBUG
else
AM_CXXFLAGS =  
endif

DICTFILE = ${CORPUS_DIR}/dict.utf8
CORPUSFILE = ${CORPUS_DIR}/corpus.utf8
TEST_CORPUSFILE = ${CORPUS_DIR}/test.utf8
REAL_CORPUSFILE = ${CORPUS_DIR}/BIGCORPUS

LMTARGET = lm_sc
IDS_FILE = ${SWAP_DIR}/${LMTARGET}.ids
SWAP_FILE = ${SWAP_DIR}/swap

#FILE NAMES for BIGRAM model
IDNGRAM_FILE = ${SWAP_DIR}/${LMTARGET}.id2gram
RAW_LM_FILE = ${SWAP_DIR}/${LMTARGET}.2gram 
SLM_FILE = ${SWAP_DIR}/${LMTARGET}.2gm 
SLM_INFO_FILE = ${SWAP_DIR}/${LMTARGET}.2gm.arpa 
TSLM_FILE = ${RESULT_DIR}/${LMTARGET}.t2g
TSLM_INFO_FILE = ${SWAP_DIR}/${LMTARGET}.t2g.arpa

#FILE NAMES for TRIGRAM model
IDNGRAM_FILE3 = ${SWAP_DIR}/${LMTARGET}.id3gram
RAW_LM_FILE3 = ${SWAP_DIR}/${LMTARGET}.3gram 
SLM_FILE3 = ${SWAP_DIR}/${LMTARGET}.3gm 
SLM_INFO_FILE3 = ${SWAP_DIR}/${LMTARGET}.3gm.arpa 
TSLM_FILE3 = ${RESULT_DIR}/${LMTARGET}.t3g
TSLM_INFO_FILE3 = ${SWAP_DIR}/${LMTARGET}.t3g.arpa
TSLM_REPACKED_FILE3 = ${SWAP_DIR}/${LMTARGET}.t3g.repacked
TSLM_UNPACKED_FILE3 = ${SWAP_DIR}/${LMTARGET}.t3g.arpa.unpacked

#Lexicon FILE names (raw resource and others)
PINYIN_TEXTFILE = ${CORPUS_DIR}/dict.utf8
PINYIN_NMP_TEXTFILE = ${SWAP_DIR}/dict_nmp.utf8
PYTRIE_FILE = ${RESULT_DIR}/pydict_sc.bin
PYTRIE_PRINTOUT = ${SWAP_DIR}/pydict_sc.log.utf8

noinst_PROGRAMS = testvc slmseg mmseg ids2ngram idngram_merge slmbuild slmprune slminfo slmthread tslminfo tslmpack tslmendian getWordFreq genpyt

genpyt_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h \
	${LEXICON_SRC_DIR}/genPYT.cpp ${LEXICON_SRC_DIR}/pytrie.cpp ${LEXICON_SRC_DIR}/pytrie.h \
	${LEXICON_SRC_DIR}/pytrie_gen.cpp ${LEXICON_SRC_DIR}/pytrie_gen.h \
	${PINYIN_SRC_DIR}/pinyin_data.cpp ${PINYIN_SRC_DIR}/pinyin_data.h \
	${PINYIN_SRC_DIR}/syllable.h

genpyt_CXXFLAGS = ${DEFAULT_INCLUDES} -I${PINYIN_SRC_DIR}

slmthread_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/sim_slm.cpp ${SLM_SRC_DIR}/sim_slm.h \
	${SLM_SRC_DIR}/thread/ValueCompress.cpp ${SLM_SRC_DIR}/thread/ValueCompress.h \
	${SLM_SRC_DIR}/thread/slmthread.cpp

testvc_SOURCES = \
	${SLM_SRC_DIR}/thread/ValueCompress.cpp ${SLM_SRC_DIR}/thread/ValueCompress.h \
	${SLM_SRC_DIR}/thread/test_vc.cpp

tslminfo_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h \
	${SLM_SRC_DIR}/sim_slm.h \
	${SLM_SRC_DIR}/tslminfo/tslminfo.cpp

tslmpack_SOURCES = \
	${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h \
	${SLM_SRC_DIR}/thread/ValueCompress.cpp ${SLM_SRC_DIR}/thread/ValueCompress.h \
	${SLM_SRC_DIR}/tslmpack/slmpack.cpp ${SLM_SRC_DIR}/tslmpack/common.h \
	${SLM_SRC_DIR}/tslmpack/arpa_conv.cpp ${SLM_SRC_DIR}/tslmpack/arpa_conv.h \
	${SLM_SRC_DIR}/tslmpack/arpa_slm.cpp ${SLM_SRC_DIR}/tslmpack/arpa_slm.h

tslmendian_SOURCES = \
	${SLM_SRC_DIR}/tslmendian/slm_file.cpp ${SLM_SRC_DIR}/tslmendian/slm_file.h \
	${SLM_SRC_DIR}/tslmendian/slm_endian.cpp \
	${SLM_SRC_DIR}/slm.h

slminfo_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/sim_slm.h \
	${SLM_SRC_DIR}/slminfo/slminfo.cpp

slmprune_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/sim_slm.cpp ${SLM_SRC_DIR}/sim_slm.h \
	${SLM_SRC_DIR}/slmprune/slmprune.cpp

getWordFreq_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h \
	${SLM_SRC_DIR}/getWordFreq/getWordFreq.cpp

slmbuild_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/sim_slm.h \
	${SLM_SRC_DIR}/sim_slmbuilder.cpp ${SLM_SRC_DIR}/sim_slmbuilder.h \
	${SLM_SRC_DIR}/slmbuild/slmbuild.cpp

mmseg_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/sim_dict.cpp ${SLM_SRC_DIR}/sim_dict.h \
	${SLM_SRC_DIR}/sim_sen.cpp ${SLM_SRC_DIR}/sim_sen.h \
	${SLM_SRC_DIR}/mmseg/mmseg.cpp

slmseg_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/sim_dict.cpp ${SLM_SRC_DIR}/sim_dict.h \
	${SLM_SRC_DIR}/sim_sen.cpp ${SLM_SRC_DIR}/sim_sen.h \
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h \
	${SLM_SRC_DIR}/slmseg/slmseg.cpp

ids2ngram_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/sim_fmerge.h \
	${SLM_SRC_DIR}/ids2ngram/ids2ngram.cpp ${SLM_SRC_DIR}/ids2ngram/idngram.h ${SLM_SRC_DIR}/ids2ngram/idngram_merge.h 

idngram_merge_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/sim_fmerge.h \
	${SLM_SRC_DIR}/ids2ngram/idngram_merge.cpp ${SLM_SRC_DIR}/ids2ngram/idngram.h ${SLM_SRC_DIR}/ids2ngram/idngram_merge.h 

if DEBUG
clean-local :
	rm -f *.rpo
endif

test_corpus :
	if [ -e ${CORPUSFILE} ]; then unlink ${CORPUSFILE}; fi
	ln -s ${TEST_CORPUSFILE} ${CORPUSFILE}

real_corpus :
	if [ -e ${CORPUSFILE} ]; then unlink ${CORPUSFILE}; fi
	ln -s ${REAL_CORPUSFILE} ${CORPUSFILE}

ids :
	./mmseg -d ${DICTFILE} -f bin -s 10 -a 9 ${CORPUSFILE} >${IDS_FILE}

slmids3:
	./slmseg -d ${DICTFILE} -f bin -s 10 -m ${TSLM_FILE3} ${CORPUSFILE} >${IDS_FILE}
	cp ${TSLM_FILE3} ${TSLM_FILE3}.normal

slmids:
	./slmseg -d ${DICTFILE} -f bin -s 10 -m ${TSLM_FILE} ${CORPUSFILE} >${IDS_FILE}
	cp ${TSLM_FILE} ${TSLM_FILE}.normal

#second round bootstrap bigram
bs_bigram : slmids m2_idngram m2_slm m2_prune m2_thread m2_tslminfo

#This is the command to make a bigram model
bigram : ids m2_idngram m2_slm m2_prune m2_thread m2_tslminfo

m2_idngram : 
	./ids2ngram -n 2 -s ${SWAP_FILE} -o ${IDNGRAM_FILE} -p 5000000 ${IDS_FILE}
	rm -f ${SWAP_FILE}

m2_slm:	
	./slmbuild -n 2 -o ${RAW_LM_FILE} -w 120000 -c 0,2 -d ABS,0.005 -d ABS,0.6 -b 10 -e 9 ${IDNGRAM_FILE}

m2_prune: 
	./slmprune ${RAW_LM_FILE} ${SLM_FILE} R 40000 100000

m2_thread :
	./slmthread ${SLM_FILE} ${TSLM_FILE}

m2_tslminfo :
	./tslminfo -v -l ${DICTFILE} ${TSLM_FILE} >${TSLM_INFO_FILE}

#Use this to generate bigram non-threaded lm arpa information if needed
m2_info :
	./slminfo -p -v -l ${DICTFILE} ${SLM_FILE} >${SLM_INFO_FILE}

#second round bootstrap to make trigram model
bs_trigram : slmids3 m3_idngram m3_slm m3_prune m3_thread m3_tslminfo

#This is the command to make a trigram model
trigram : ids m3_idngram m3_slm m3_prune m3_thread m3_tslminfo

m3_idngram : 
	./ids2ngram -n 3 -s ${SWAP_FILE} -o ${IDNGRAM_FILE3} -p 5000000 ${IDS_FILE}
	rm -f ${SWAP_FILE}

m3_slm:	
	./slmbuild -n 3 -o ${RAW_LM_FILE3} -w 120000 -c 0,2,2 -d ABS,0.0005 -d ABS -d ABS,0.6 -b 10 -e 9 ${IDNGRAM_FILE3}

m3_prune: 
	./slmprune ${RAW_LM_FILE3} ${SLM_FILE3} R 100000 1250000 1000000

m3_thread :
	./slmthread ${SLM_FILE3} ${TSLM_FILE3}

m3_tslminfo :
	./tslminfo -p -v -l ${DICTFILE} ${TSLM_FILE3} >${TSLM_INFO_FILE3}

m3_tslmpack :
	./tslmpack ${TSLM_INFO_FILE3} ${DICTFILE} ${TSLM_REPACKED_FILE3}

m3_tslmunpack :
	./tslminfo -p -v -l ${DICTFILE} ${TSLM_REPACKED_FILE3} >${TSLM_UNPACKED_FILE3}

#Use this to generate trigram non-threaded lm arpa information if needed
m3_info :
	./slminfo -p -v -l ${DICTFILE} ${SLM_FILE3} >${SLM_INFO_FILE3}

#clean all intermedian file for building the model
model_clean :
	rm -f ${IDS_FILE}
	rm -f ${SWAP_FILE}
	rm -f ${IDNGRAM_FILE} ${RAW_LM_FILE}
	rm -f ${IDNGRAM_FILE3} ${RAW_LM_FILE3}

lexicon :
	./genpyt ${PINYIN_TEXTFILE} ${PYTRIE_FILE} ${PYTRIE_PRINTOUT} ${TSLM_FILE3}

lexicon2 :
	./genpyt ${PINYIN_TEXTFILE} ${PYTRIE_FILE} ${PYTRIE_PRINTOUT} ${TSLM_FILE}
