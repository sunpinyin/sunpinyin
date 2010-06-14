LM_URL=http://open-gram.googlecode.com/files
WGET=wget
TAR=tar

all: lm_sc.t3g.arpa dict.utf8
	@echo done

lm_sc.t3g.arpa: stamp-lm

stamp-lm: lm_sc.t3g.arpa.tar.bz2
	$(TAR) -jxf $^
	touch $@

lm_sc.t3g.arpa.tar.bz2:
	$(WGET) $(LM_URL)/$@

dict.utf8: stamp-dict

stamp-dict: dict.utf8.tar.bz2
	$(TAR) -jxf $^
	touch $@

dict.utf8.tar.bz2:
	$(WGET) $(LM_URL)/$@

clean:
	@rm -f stamp-dict stamp-lm lm_sc.t3g.arpa dict.utf8
	@echo cleaned
