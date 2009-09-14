#
# Copyright (c) 2009 Kov Chai <tchaikov@gmail.com>
#

# 
# SunPinyin
# 
SRC_DIR = $(top_srcdir)/src
SUNPINYIN_SRC_DIR = $(top_srcdir)/src/ime-core
SLM_SRC_DIR = $(top_srcdir)/src/slm
LEXICON_SRC_DIR = $(top_srcdir)/src/lexicon
PINYIN_SRC_DIR = $(top_srcdir)/src/pinyin
IBUS_SUNPINYIN_DATADIR=$(pkgdatadir)/data
IBUS_SUNPINYIN_ICONDIR=$(pkgdatadir)/icons

noinst_LTLIBRARIES = libsunpinyin.la

libsunpinyin_la_SOURCES = \
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
	$(SUNPINYIN_SRC_DIR)/userdict.cpp $(SUNPINYIN_SRC_DIR)/userdict.h

libsunpinyin_la_CXXFLAGS = \
	-D_USE_RAW_PROBABILITY \
	-DSUNPINYIN_DATA_DIR=\"../../data\"\
	-I$(top_srcdir) \
	-I$(SUNPINYIN_SRC_DIR) \
	-I$(LEXICON_SRC_DIR) \
	-I$(SLM_SRC_DIR) \
	-I$(PINYIN_SRC_DIR)\
	-I$(top_builddir) \
	-I$(top_builddir)/src \
	$(sqlite3_CFLAGS) 

libsunpinyin_la_LDFLAGS = \
	-no-undefined -avoid-version \
	$(LIBTOOL_EXPORT_OPTIONS) \
	@LTLIBINTL@ \
	$(sqlite3_LIBS)

libsunpinyin_la_LIBADD = -lm

# 
# ibus-sunpinyin
# 
libexec_PROGRAMS = ibus-engine-sunpinyin

ibus_engine_sunpinyin_SOURCES = \
	main.cpp \
	engine.cpp \
	engine.h \
	sunpinyin_engine.cpp \
	sunpinyin_engine.h \
	sunpinyin_engine_proxy.cpp \
	sunpinyin_engine_proxy.h \
	sunpinyin_lookup_table.cpp \
	sunpinyin_lookup_table.h \
	sunpinyin_property.cpp \
	sunpinyin_property.h \
	sunpinyin_config.cpp \
	sunpinyin_config.h \
	sunpinyin_config_keys.h \
	imi_ibus_win.cpp \
	imi_ibus_win.h \
	ibus_portable.cpp \
	ibus_portable.h \
	$(NULL)

ibus_engine_sunpinyin_CXXFLAGS = \
	@IBUS_CFLAGS@ \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-I$(top_builddir)/src \
	-I$(SUNPINYIN_SRC_DIR) \
	-I$(PINYIN_SRC_DIR) \
	-I$(LEXICON_SRC_DIR) \
	-I$(SLM_SRC_DIR) \
	-DSUNPINYIN_DATA_DIR=\"$(IBUS_SUNPINYIN_DATADIR)\" \
	-DSUNPINYIN_ICON_DIR=\"$(IBUS_SUNPINYIN_ICONDIR)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-Wall \
	$(NULL)

ibus_engine_sunpinyin_CFLAGS = \
	@IBUS_CFLAGS@ \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	$(NULL)

ibus_engine_sunpinyin_LDFLAGS = \
	@IBUS_LIBS@ \
	$(NULL)

ibus_engine_sunpinyin_LDADD = \
	libsunpinyin.la

component_DATA = \
	sunpinyin.xml \
	$(NULL)

componentdir = @datadir@/ibus/component

EXTRA_DIST = \
	sunpinyin.xml.in \
	$(NULL)

CLEANFILES = \
	sunpinyin.xml \
	$(NULL)

sunpinyin.xml: sunpinyin.xml.in
	( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

# 
# data files
# 
scim_sunpinyin_datadir   = $(IBUS_SUNPINYIN_DATADIR)
scim_sunpinyin_data_DATA = $(top_builddir)/data/lm_sc.t3g \
                           $(top_builddir)/data/pydict_sc.bin \
                           $(top_builddir)/data/quanpin.dat 

$(top_builddir)/data/quanpin.dat:
	$(python) $(top_builddir)/python/pinyin_data.py

icondir                  = $(IBUS_SUNPINYIN_ICONDIR)
icon_DATA                = $(top_builddir)/data/sunpinyin_logo.xpm \
                           $(top_builddir)/data/eng.png            \
                           $(top_builddir)/data/han.png            \
                           $(top_builddir)/data/cnpunc.png         \
                           $(top_builddir)/data/enpunc.png         \
                           $(top_builddir)/data/fullwidth.png      \
                           $(top_builddir)/data/halfwidth.png
test: ibus-engine-sunpinyin
	$(builddir)/ibus-engine-sunpinyin
