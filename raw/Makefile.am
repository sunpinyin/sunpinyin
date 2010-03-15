EXTRA_DIST = dict.utf8 lm_sc.t3g.arpa.tar.bz2


noinst_PROGRAMS = tslminfo
noinst_DATA = lm_sc.t3g.arpa dict.utf8

CLEANFILES =  $(noinst_DATA)

SOURCE_DIR = $(top_builddir)/@SOURCE_DIR@
SLM_SRC_DIR = $(top_builddir)/@SLM_SRC_DIR@

LM_URL=http://open-gram.googlecode.com/files

tslminfo_SOURCES = \
	${SOURCE_DIR}/portability.cpp ${SOURCE_DIR}/portability.h ${SOURCE_DIR}/host_os.h \
	${SLM_SRC_DIR}/slm.cpp ${SLM_SRC_DIR}/slm.h \
	${SLM_SRC_DIR}/sim_slm.h \
	${SLM_SRC_DIR}/tslminfo/tslminfo.cpp

lm_sc.t3g.arpa: lm_sc.t3g.arpa.tar.bz2 
	tar -jxf lm_sc.t3g.arpa.tar.bz2
	touch $@

lm_sc.t3g.arpa.tar.bz2:
	${WGET} ${LM_URL}/$@

dict.utf8: dict.utf8.tar.bz2
	tar -jxf $^
	touch $@

dict.utf8.tar.bz2:
	${WGET} ${LM_URL}/$@

MAINTAINERCLEANFILES = lm_sc.t3g.arpa.tar.bz2 dict.utf8.tar.bz2
