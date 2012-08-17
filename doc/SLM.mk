# Sample Makefile for SLM training.

DICT_FILE = dict.utf8
CORPUS_FILE = corpus.utf8

SLM_TARGET = lm_sc
IDS_FILE = ${SLM_TARGET}.ids

BIGRAM_STAT = ${SLM_TARGET}.id2gram
SLM2_RAW_FILE = ${SLM_TARGET}.2gram
SLM2_FILE = ${SLM_TARGET}.2gm
SLM2_TEXT_FILE = ${SLM_TARGET}.2gm.arpa
TSLM2_FILE = ${SLM_TARGET}.t2g
TSLM2_TEXT_FILE = ${SLM_TARGET}.t2g.arpa

TRIGRAM_STAT = ${SLM_TARGET}.id3gram
SLM3_RAW_FILE = ${SLM_TARGET}.3gram 
SLM3_FILE = ${SLM_TARGET}.3gm 
SLM3_TEXT_FILE = ${SLM_TARGET}.3gm.arpa 
TSLM3_FILE = ${SLM_TARGET}.t3g
TSLM3_TEXT_FILE = ${SLM_TARGET}.t3g.arpa

PYTRIE2_FILE = pydict2_sc.bin
PYTRIE2_LOG_FILE = pydict2_sc.log
PYTRIE3_FILE = pydict3_sc.bin
PYTRIE3_LOG_FILE = pydict3_sc.log

all: bootstrap3

mmseg_ids: ${DICT_FILE} ${CORPUS_FILE}
	mmseg -f bin -s 10 -a 9 -d ${DICT_FILE} ${CORPUS_FILE} > ${IDS_FILE}

slm2_ids: ${DICT_FILE} ${CORPUS_FILE} ${TSLM2_FILE}
	slmseg -f bin -s 10 \
		-d ${DICT_FILE} -m ${TSLM2_FILE} ${CORPUS_FILE} > ${IDS_FILE}

slm3_ids: ${DICT_FILE} ${CORPUS_FILE} ${TSLM3_FILE}
	slmseg -f bin -s 10 \
		-d ${DICT_FILE} -m ${TSLM3_FILE} ${CORPUS_FILE} > ${IDS_FILE}

bigram_stat: ${BIGRAM_STAT_FILE}
${BIGRAM_STAT_FILE}: ${IDS_FILE}
	ids2ngram -n 2 -p 20000000 -s $@.tmp -o $@ $<
	rm -f $@.tmp

slm2_raw: ${SLM2_RAW_FILE}
${SLM2_RAW_FILE}: ${BIGRAM_STAT_FILE}
	slmbuild -n 2 -w 200000 -c 0,2 -d ABS,0.005 -d ABS -b 10 -e 9 -o $@ $<

slm2: ${SLM2_FILE}
${SLM2_FILE}: ${SLM2_RAW_FILE}
	slmprune $< $@ R 100000 200000

tslm2: ${TSLM2_FILE}
${TSLM2_FILE}:${SLM2_FILE}
	slmthread $< $@

lexicon2: ${PYTRIE2_FILE}
${PYTRIE2_FILE}: ${DICT_FILE} ${TSLM_FILE2}
	genpyt -i ${DICT_FILE} -s ${TSLM_FILE2} -l ${PYTRIE2_LOG_FILE} -o $@

slm2_info: ${SLM2_TEXT_FILE}
${SLM2_TEXT_FILE}: ${DICT_FILE} ${SLM2_FILE}
	slminfo -p -v -l ${DICT_FILE} ${SLM2_FILE} > $@

tslm2_info: ${TSLM2_TEXT_FILE}
${TSLM2_TEXT_FILE}: ${DICT_FILE} ${TSLM2_FILE}
	tslminfo -v -l ${DICT_FILE} ${TSLM2_FILE} > $@

tslm2_pack: ${DICT_FILE} ${TSLM2_TEXT_FILE}
	tslmpack ${TSLM2_TEXT_FILE} ${DICT_FILE} ${TSLM2_FILE}

trigram_stat: ${TRIGRAM_STAT_FILE}
${TRIGRAM_STAT_FILE}: ${IDS_FILE}
	ids2ngram -n 3 -p 20000000 -s $@.tmp -o $@ $<
	rm -f $@.tmp

slm3_raw: ${SLM3_RAW_FILE}
${SLM3_RAW_FILE}: ${TRIGRAM_STAT_FILE}
	slmbuild -n 3 -w 200000 -c 0,2,2 -d ABS,0.0005 -d ABS -d ABS -b 10 -e 9 \
		-o $@ $<
slm3: ${SLM3_FILE}
${SLM3_FILE}: ${SLM3_RAW_FILE}
	slmprune $< $@ R 100000 2500000 1000000

tslm3: ${TSLM3_FILE}
${TSLM3_FILE}: ${SLM3_FILE}
	slmthread $< $@

slm3_info: ${SLM3_TEXT_FILE}
${SLM3_TEXT_FILE}: ${DICT_FILE} ${SLM3_FILE}
	slminfo -p -v -l ${DICT_FILE} ${SLM3_FILE} > $@

tslm3_info: ${TSLM3_TEXT_FILE}
${TSLM3_TEXT_FILE}: ${DICT_FILE} ${TSLM3_FILE}
	tslminfo -p -v -l ${DICT_FILE} ${TSLM3_FILE} > $@

tslm3_pack: ${DICT_FILE} ${TSLM3_TEXT_FILE}
	tslmpack ${TSLM3_TEXT_FILE} ${DICT_FILE} ${TSLM3_FILE}

lexicon3: ${PYTRIE3_FILE}
${PYTRIE3_FILE}: ${DICT_FILE} ${TSLM_FILE3}
	genpyt -i ${DICT_FILE} -s ${TSLM_FILE3} -l ${PYTRIE3_LOG_FILE} -o $@

tmp_clean:
	rm -f *.tmp ${IDS_FILE}
	rm -f ${BIGRAM_STAT} ${SLM2_RAW_FILE}
	rm -f ${TRIGRAM_STAT} ${SLM3_RAW_FILE}

mmseg_bigram: mmseg_ids tslm2_info
mmseg_trigram: mmseg_ids tslm3_info
slm_bigram2: slm2_ids tslm2_info
slm_bigram3: slm3_ids tslm2_info
slm_trigram2: slm2_ids tslm3_info
slm_trigram3: slm3_ids tslm3_info

bootstrap2:
	make mmseg_bigram
	make slm_bigram2
	make lexicon2

bootstrap3:
	make mmseg_trigram
	make slm_trigram3
	make lexicon3

