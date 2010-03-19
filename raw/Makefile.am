EXTRA_DIST = lm_sc.t3g.arpa.tar.bz2 dict.utf8.tar.bz2
noinst_DATA = lm_sc.t3g.arpa dict.utf8

SOURCE_DIR = $(top_builddir)/@SOURCE_DIR@
SLM_SRC_DIR = $(top_builddir)/@SLM_SRC_DIR@

LM_URL=http://open-gram.googlecode.com/files

lm_sc.t3g.arpa: stamp-lm

stamp-lm: lm_sc.t3g.arpa.tar.bz2
	tar -jxf $^
	touch $@

lm_sc.t3g.arpa.tar.bz2:
	${WGET} ${LM_URL}/$@

dict.utf8: stamp-dict

stamp-dict: dict.utf8.tar.bz2
	tar -jxf $^
	touch $@

dict.utf8.tar.bz2:
	${WGET} ${LM_URL}/$@


CLEANFILES = stamp-* lm_sc.t3g.arpa dict.utf8
DISTCLEANFILES = lm_sc.t3g.arpa.tar.bz2 dict.utf8.tar.bz2
