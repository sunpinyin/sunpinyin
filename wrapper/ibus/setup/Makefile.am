setup_sunpinyin_PYTHON = \
	main.py \
	setup.glade \
	$(NULL)

nodist_setup_sunpinyin_PYTHON = config.py

setup_sunpinyindir = $(datadir)/ibus-sunpinyin/setup

libexec_SCRIPTS = ibus-setup-sunpinyin

CLEANFILES = \
	*.pyc \
	$(NULL)

EXTRA_DIST = \
	ibus-setup-sunpinyin.in \
	$(NULL)

test:
	$(ENV) DBUS_DEBUG=true \
		   LANG=en_US \
		   PYTHONPATH=$(abs_top_srcdir):$(pyexecdir) \
		   $(PYTHON) $(srcdir)/main.py

