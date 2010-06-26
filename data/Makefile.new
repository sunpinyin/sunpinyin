LM_SC_ARPA = ../raw/lm_sc.t3g.arpa
DICTFILE = ../raw/dict.utf8
PYTRIE_LOG = pydict_sc.log.utf8
LM_SC_BIN = lm_sc.t3g

all: pydict_sc.bin pydict_sc.bin.le pydict_sc.bin.be lm_sc.t3g.le lm_sc.t3g.be
	@echo done

$(LM_SC_BIN): $(LM_SC_ARPA) $(DICTFILE)
	./tslmpack $(LM_SC_ARPA) $(DICTFILE) $@

pydict_sc.bin: $(DICTFILE) $(LM_SC_BIN)
	./genpyt -i $(DICTFILE) -o $@ -l $(PYTRIE_LOG) -s $(LM_SC_BIN)

pydict_sc.bin.le: $(DICTFILE) $(LM_SC_BIN)
	./genpyt -e le -i $(DICTFILE) -o $@ -l $(PYTRIE_LOG) -s $(LM_SC_BIN)

pydict_sc.bin.be: $(DICTFILE) $(LM_SC_BIN)
	./genpyt -e be -i $(DICTFILE) -o $@ -l $(PYTRIE_LOG) -s $(LM_SC_BIN)

lm_sc.t3g.le: lm_sc.t3g
	./tslmendian -e le -i $(LM_SC_BIN) -o $@

lm_sc.t3g.be: lm_sc.t3g
	./tslmendian -e be -i $(LM_SC_BIN) -o $@

clean:
	@rm -f pydict_sc.bin pydict_sc.bin.le pydict_sc.bin.be lm_sc.t3g lm_sc.t3g.le lm_sc.t3g.be pydict_sc.log.utf8
	@echo cleaned
