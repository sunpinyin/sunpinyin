#
# Copyright (c) 2009 Kov Chai <tchaikov@gmail.com>
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

# 
# SunPinyin
# 
SRC_DIR = $(top_srcdir)/src
SUNPINYIN_SRC_DIR = $(top_srcdir)/src/ime-core
SLM_SRC_DIR = $(top_srcdir)/src/slm
LEXICON_SRC_DIR = $(top_srcdir)/src/lexicon
PINYIN_SRC_DIR = $(top_srcdir)/src/pinyin
IBUS_SUNPINYIN_DATADIR = $(pkgdatadir)/data
IBUS_SUNPINYIN_ICONDIR = $(pkgdatadir)/icons
SUNPINYIN_USERDATA_DIR_PREFIX = .ibus/sunpinyin

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
	-DSUNPINYIN_USERDATA_DIR_PREFIX=\"$(SUNPINYIN_USERDATA_DIR_PREFIX)\" \
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
	-DSUNPINYIN_USERDATA_DIR_PREFIX=\"$(SUNPINYIN_USERDATA_DIR_PREFIX)\" \
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

$(top_builddir)/data/pydict_sc.bin: $(top_builddir)/data/lm_sc.t3g
	make -C $(top_builddir)/build genpyt lexicon

icondir                  = $(IBUS_SUNPINYIN_ICONDIR)
icon_DATA                = $(top_builddir)/data/sunpinyin_logo.xpm \
                           $(top_builddir)/data/eng.png            \
                           $(top_builddir)/data/han.png            \
                           $(top_builddir)/data/cnpunc.png         \
                           $(top_builddir)/data/enpunc.png         \
                           $(top_builddir)/data/fullwidth.png      \
                           $(top_builddir)/data/halfwidth.png

test: ibus-engine-sunpinyin
	$(builddir)/ibus-engine-sunpinyin --ibus
