POD2MAN:=pod2man

%.1 : %.pod
	-pod2man -c "User Commands" -r "$(PACKAGE_STRING)" $< > $@
