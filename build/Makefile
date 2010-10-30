SLM_SRC_DIR = ../src/slm
LEXICON_SRC_DIR = ../src/lexicon
PINYIN_SRC_DIR = ../src/pinyin

CORPUS_DIR = ../raw
SWAP_DIR = ../swap
RESULT_DIR = ../data

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

#second round bootstrap bigram from a trigram model
bs_bigram3 : slmids3 m2_idngram m2_slm m2_prune m2_thread m2_tslminfo

#This is the command to make a bigram model
bigram : ids m2_idngram m2_slm m2_prune m2_thread m2_tslminfo

m2_idngram : 
	./ids2ngram -n 2 -s ${SWAP_FILE} -o ${IDNGRAM_FILE} -p 20000000 ${IDS_FILE}
	rm -f ${SWAP_FILE}

m2_slm:	
	./slmbuild -n 2 -o ${RAW_LM_FILE} -w 200000 -c 0,2 -d ABS,0.005 -d ABS -b 10 -e 9 ${IDNGRAM_FILE}

m2_prune: 
	./slmprune ${RAW_LM_FILE} ${SLM_FILE} R 100000 200000

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
	./ids2ngram -n 3 -s ${SWAP_FILE} -o ${IDNGRAM_FILE3} -p 20000000 ${IDS_FILE}
	rm -f ${SWAP_FILE}

m3_slm:	
	./slmbuild -n 3 -o ${RAW_LM_FILE3} -w 200000 -c 0,2,2 -d ABS,0.0005 -d ABS -d ABS -b 10 -e 9 ${IDNGRAM_FILE3}

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
	./genpyt -i ${PINYIN_TEXTFILE} -o ${PYTRIE_FILE} -l ${PYTRIE_PRINTOUT} -s ${TSLM_FILE3}

lexicon2 :
	./genpyt -i ${PINYIN_TEXTFILE} -o ${PYTRIE_FILE} -l ${PYTRIE_PRINTOUT} -s ${TSLM_FILE}
