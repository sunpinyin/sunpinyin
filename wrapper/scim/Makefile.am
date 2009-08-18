#
# Copyright (c) 2007 Kov Chai <tchaikov@gmail.com>
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

SUNPINYIN_SRC_DIR 	= $(top_srcdir)/src
SLM_SRC_DIR 		= $(top_srcdir)/src/slm
LEXICON_SRC_DIR 	= $(top_srcdir)/src/lexicon

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[^_].*"'

IME_SO_DIR = $(libdir)/scim-1.0/1.4.0/IMEngine

noinst_HEADERS  	= imi_scimwin.h \
			  sunpinyin_imengine_config_keys.h \
			  sunpinyin_imengine.h \
			  sunpinyin_keycode.h \
			  sunpinyin_lookup_table.h \
			  sunpinyin_private.h \
			  sunpinyin_utils.h

# 
# SunPinyin
# 
noinst_LTLIBRARIES = libsunpinyin.la

libsunpinyin_la_SOURCES = \
        $(SLM_SRC_DIR)/slm.cpp \
        $(LEXICON_SRC_DIR)/pytrie.cpp \
	$(SUNPINYIN_SRC_DIR)/portability.cpp \
        $(SUNPINYIN_SRC_DIR)/imi_context.cpp \
        $(SUNPINYIN_SRC_DIR)/imi_data.cpp \
        $(SUNPINYIN_SRC_DIR)/lattice_states.cpp \
        $(SUNPINYIN_SRC_DIR)/imi_view.cpp  \
	$(SUNPINYIN_SRC_DIR)/imi_uiobjects.cpp \
        $(SUNPINYIN_SRC_DIR)/imi_view_modern.cpp \
        $(SUNPINYIN_SRC_DIR)/imi_view_classic.cpp \
	$(SUNPINYIN_SRC_DIR)/imi_winHandler.cpp \
	$(SUNPINYIN_SRC_DIR)/ic_history.cpp \
	$(SUNPINYIN_SRC_DIR)/imi_options.cpp

libsunpinyin_la_CXXFLAGS = -I$(top_srcdir) -I$(SUNPINYIN_SRC_DIR) \
                           -I$(top_builddir) -I$(top_builddir)/src \
                           -I$(LEXICON_SRC_DIR) -I$(SLM_SRC_DIR) \
                           @SCIM_CFLAGS@ \
                           -D_USE_RAW_PROBABILITY -DSCIM_INTERFACE \
                           -DENABLE_DEBUG

libsunpinyin_la_LDFLAGS = -no-undefined -avoid-version \
                          $(LIBTOOL_EXPORT_OPTIONS) \
			  @LTLIBINTL@

libsunpinyin_la_LIBADD = -lm

# 
# scim-sunpinyin

# 
# input method module
#
moduledir            = @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES   = sunpinyin.la

sunpinyin_la_SOURCES =  sunpinyin_utils.cpp \
		        imi_scimwin.cpp \
		        sunpinyin_imengine.cpp \
			sunpinyin_lookup_table.cpp

sunpinyin_la_CXXFLAGS   = -I$(SUNPINYIN_SRC_DIR) \
                          -I$(LEXICON_SRC_DIR) -I$(SLM_SRC_DIR) \
                          @SCIM_CFLAGS@ \
                          -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_SUNPINYIN_LOCALEDIR=\"$(datadir)/locale\" \
			  -DSCIM_SUNPINYIN_DATADIR=\"@SCIM_SUNPINYIN_DATADIR@\" \
                          -DSCIM_SUNPINYIN_ICON_FILE=\"@SCIM_ICONDIR@/sunpinyin_logo.xpm\" \
			  -DENABLE_DEBUG

sunpinyin_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  $(LIBTOOL_EXPORT_OPTIONS) \
			  @SCIM_LIBS@ \
			  @LTLIBINTL@

sunpinyin_la_LIBADD	 = libsunpinyin.la

# 
# Setup UI
# 
if SCIM_BUILD_SETUP
SETUP_MODULE = sunpinyin-imengine-setup.la
endif

setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(SETUP_MODULE)

sunpinyin_imengine_setup_la_SOURCES = sunpinyin_imengine_setup.cpp

sunpinyin_imengine_setup_la_CXXFLAGS= @SCIM_GTKUTILS_CFLAGS@ \
                                   @SCIM_CFLAGS@ \
			  	   -DSCIM_SUNPINYIN_LOCALEDIR=\"$(datadir)/locale\"

sunpinyin_imengine_setup_la_LDFLAGS = -avoid-version \
                                   -rpath $(setupmoduledir) \
                                   -module \
                                   $(LIBTOOL_EXPORT_OPTIONS) \
                                   @SCIM_GTKUTILS_LIBS@ \
                                   @LTLIBINTL@ \
                                   @SCIM_LIBS@

#
# data files installation
#
scim_sunpinyin_datadir   = @SCIM_SUNPINYIN_DATADIR@
scim_sunpinyin_data_DATA = $(top_builddir)/data/lm_sc.t3g \
                           $(top_builddir)/data/pydict_sc.bin

icondir                  = $(SCIM_ICONDIR)
icon_DATA                = $(top_builddir)/data/sunpinyin_logo.xpm
