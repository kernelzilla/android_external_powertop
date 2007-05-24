SRC=$(wildcard *.po)
OBJ= $(SRC:.po=.mo)
INST= $(SRC:.po=.inst)

translations: $(OBJ)
	
%.mo: %.po
	-@msgfmt -o $@ -v $<

clean:
	rm -f *.mo

# hack to automate installation dynamicaly, without previous knowledge of 
# the po/mo file list (we fool make by pretending the need for .inst files).
install: $(OBJ) $(INST)

%.inst: %.mo
	mkdir -p $(DESTDIR)$(LOCALESDIR)/$*/LC_MESSAGES/
	cp -f $< $(DESTDIR)$(LOCALESDIR)/$*/LC_MESSAGES/powertop.mo

