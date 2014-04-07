# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.


PREFIX = /usr
USR_PREFIX = /usr
LOCAL_PREFIX = $(USR_PREFIX)/local
BIN = /bin
SBIN = /sbin
INSTALL_BIN = $(SBIN)
DEV = /dev
DATA = /share
LICENSES = $(DATA)/licenses
COMMAND = cerberus
PKGNAME = cerberus

EXTRA_CPP_FLAGS = 
# see configurable-definitions

_LB = $(LOCAL_PREFIX)$(BIN)
_UB = $(USR_PREFIX)$(BIN)
_SB = $(BIN)
_LS = $(LOCAL_PREFIX)$(SBIN)
_US = $(USR_PREFIX)$(SBIN)
_SS = $(SBIN)

AUTH = pam
TTY_GROUP = tty
DEFAULT_HOME = /
DEFAULT_SH = sh
DEFAULT_SHELL = $(BIN)/$(DEFAULT_SH)
DEFAULT_TERM = dumb
VCS = $(DEV)/vcs
VCSA = $(DEV)/vcsa
PATH = $(_LB):$(_UB):$(_SB)
PATH_ROOT = $(_LS):$(_LB):$(_US):$(_UB):$(_SS):$(_SB)

auth_none = 0
auth_crypt = 1
auth_pam = 2

H = \#
VCS_LEN = $(shell vcs="$(VCS)" ; echo "$${$(H)vcs}")
VCSA_LEN = $(shell vcsa="$(VCSA)" ; echo "$${$(H)vcsa}")

STR_DEFS = TTY_GROUP DEFAULT_HOME DEFAULT_SH DEFAULT_SHELL DEFAULT_TERM PATH PATH_ROOT VCS VCSA
VRB_DEFS = VCS_LEN VCSA_LEN

STR_CPPFLAGS = $(foreach D, $(STR_DEFS), -D'$(D)="$($(D))"')
VRB_CPPFLAGS = $(foreach D, $(VRB_DEFS), -D'$(D)=$($(D))') -DAUTH=$(auth_$(AUTH))

OPTIMISE = -Os
CPPFLAGS = $(EXTRA_CPP_FLAGS) $(STR_CPPFLAGS) $(VRB_CPPFLAGS)
CFLAGS = -std=gnu99 -Wall -Wextra
LDFLAGS =
ifneq ($(AUTH),none)
LDFLAGS += -lpassphrase
endif
ifeq ($(AUTH),crypt)
LDFLAGS += -lcrypt
endif
ifeq ($(AUTH),pam)
LDFLAGS += -lpam
endif

CC_FLAGS = $(CPPFLAGS) $(CFLAGS) $(OPTIMISE)
LD_FLAGS = $(LDFLAGS) $(CFLAGS) $(OPTIMISE)

SRC = cerberus quit security login
ifneq ($(AUTH),none)
SRC += auth/$(AUTH)
endif
OBJ = $(foreach S, $(SRC), obj/$(S).o)


.PHONY: default
default: command info

.PHONY: all
all: command doc

.PHONY: command
command: cerberus


.PHONY: cerberus
cerberus: bin/cerberus

bin/cerberus: $(OBJ)
	@mkdir -p bin
	$(CC) $(LD_FLAGS) -o "$@" $^

obj/cerberus.o: $(foreach H, $(SRC), src/$(H).h) src/auth.h
obj/%.o: src/%.c src/%.h src/config.h
	@mkdir -p "$(shell dirname "$@")"
	$(CC) $(CC_FLAGS) -o "$@" -c "$<"


.PHONY: doc
doc: info pdf ps dvi

.PHONY: info
info: cerberus.info
%.info: info/%.texinfo
	makeinfo "$<"

.PHONY: pdf
pdf: cerberus.pdf
%.pdf: info/%.texinfo info/fdl.texinfo
	mkdir -p obj
	cd obj ; yes X | texi2pdf ../$<
	mv obj/$@ $@

.PHONY: dvi
dvi: cerberus.dvi
%.dvi: info/%.texinfo info/fdl.texinfo
	mkdir -p obj
	cd obj ; yes X | $(TEXI2DVI) ../$<
	mv obj/$@ $@

.PHONY: ps
ps: cerberus.ps
%.ps: info/%.texinfo info/fdl.texinfo
	mkdir -p obj
	cd obj ; yes X | texi2pdf --ps ../$<
	mv obj/$@ $@


.PHONY: install
install: install-base install-info

.PHONY: install-all
install-all: install-base install-doc

.PHONY: install-base
install-base: install-command install-license

.PHONY: install-command
install-command: bin/cerberus
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(INSTALL_BIN)"
	install  -m755 -- bin/cerberus "$(DESTDIR)$(PREFIX)$(INSTALL_BIN)/$(COMMAND)"

.PHONY: install-license
install-license:
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"
	install  -m644 -- COPYING LICENSE "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"

.PHONY: install-doc
install-doc: install-info install-pdf install-ps install-dvi

.PHONY: install-info
install-info: cerberus.info
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(DATA)/info"
	install  -m644 -- "$<" "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info"

.PHONY: install-pdf
install-pdf: cerberus.pdf
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(DATA)/doc"
	install  -m644 -- "$<" "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).pdf"

.PHONY: install-ps
install-ps: cerberus.ps
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(DATA)/doc"
	install  -m644 -- "$<" "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).ps"

.PHONY: install-dvi
install-dvi: cerberus.dvi
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(DATA)/doc"
	install  -m644 -- "$<" "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).dvi"


.PHONY: uninstall
uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)$(INSTALL_BIN)/$(COMMAND)"
	-rm -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)/COPYING"
	-rm -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)/LICENSE"
	-rmdir -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"
	-rm -- "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info"
	-rm -- "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).pdf"
	-rm -- "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).ps"
	-rm -- "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).dvi"



.PHONY: clean
clean:
	-rm -r bin obj cerberus.{info,pdf,ps,dvi}

