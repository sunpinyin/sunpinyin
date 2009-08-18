include $(top_srcdir)/doc/rules.mk

man_MANS = \
	genpyt.1 ids2ngram.1 mmseg.1 \
	slmbuild.1 slmprune.1 slmthread.1 slmseg.1 slminfo.1 \
	tslminfo.1 tslmpack.1 tslmendian.1 idngram_merge.1 getWordFreq.1

EXTRA_DIST = $(man_MANS)

CLEANFILES = $(man_MANS)

%.1: %.pod
	pod2man --center=" " --release=" " $^ > $@ 2>/dev/null
