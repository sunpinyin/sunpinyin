EXTRA_DIST = pinyin_data.py trie.py $(PINYIN_SRC_DIR)/quanpin_trie.h

PINYIN_SRC_DIR = $(top_srcdir)/src/pinyin

$(PINYIN_SRC_DIR)/quanpin_trie.h: pinyin_data.py
	$(PYTHON) pinyin_data.py
