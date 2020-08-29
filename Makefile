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
ETC = /etc
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
CERBERUSRC = $(ETC)/cerberusrc

auth_none = 0
auth_crypt = 1
auth_pam = 2

H = \#
VCS_LEN = $(shell vcs="$(VCS)" ; echo "$${$(H)vcs}")
VCSA_LEN = $(shell vcsa="$(VCSA)" ; echo "$${$(H)vcsa}")

STR_DEFS = TTY_GROUP DEFAULT_HOME DEFAULT_SH DEFAULT_SHELL DEFAULT_TERM PATH PATH_ROOT VCS VCSA CERBERUSRC
VRB_DEFS = VCS_LEN VCSA_LEN

STR_CPPFLAGS = $(foreach D, $(STR_DEFS), -D'$(D)="$($(D))"')
VRB_CPPFLAGS = $(foreach D, $(VRB_DEFS), -D'$(D)=$($(D))') -DAUTH=$(auth_$(AUTH))

OPTIMISE = -Os
STD=gnu99
WARN = -Wall -Wextra -Wdouble-promotion -Wformat=2 -Winit-self -Wmissing-include-dirs            \
       -Wtrampolines -Wfloat-equal -Wshadow -Wmissing-prototypes -Wmissing-declarations          \
       -Wredundant-decls -Wnested-externs -Winline -Wno-variadic-macros -Wsync-nand              \
       -Wunsafe-loop-optimizations -Wcast-align -Wstrict-overflow -Wdeclaration-after-statement  \
       -Wundef -Wbad-function-cast -Wcast-qual -Wwrite-strings -Wlogical-op -Waggregate-return   \
       -Wstrict-prototypes -Wold-style-definition -Wpacked -Wvector-operation-performance        \
       -Wunsuffixed-float-constants -Wsuggest-attribute=const -Wsuggest-attribute=noreturn       \
       -Wsuggest-attribute=pure -Wsuggest-attribute=format -Wnormalized=nfkc -Wconversion        \
       -fstrict-aliasing -fstrict-overflow -fipa-pure-const -ftree-vrp -fstack-usage             \
       -funsafe-loop-optimizations
# excluded:  -pedantic
CPPFLAGS_ = $(EXTRA_CPP_FLAGS) $(STR_CPPFLAGS) $(VRB_CPPFLAGS)
CFLAGS_ = -std=$(STD) $(WARN)
LDFLAGS_ =
ifneq ($(AUTH),none)
LDFLAGS_ += -lpassphrase
endif
ifeq ($(AUTH),crypt)
LDFLAGS_ += -lcrypt
endif
ifeq ($(AUTH),pam)
LDFLAGS_ += -lpam
endif

CC_FLAGS = $(CPPFLAGS_) $(CFLAGS_) $(OPTIMISE) $(CPPFLAGS) $(CFLAGS)
LD_FLAGS = $(LDFLAGS_) $(CFLAGS_) $(OPTIMISE) $(LDFLAGS)

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
	@mkdir -p obj/pdf
	cd obj/pdf ; yes X | texi2pdf ../../$<
	mv obj/pdf/$@ $@

.PHONY: dvi
dvi: cerberus.dvi
%.dvi: info/%.texinfo info/fdl.texinfo
	@mkdir -p obj/dvi
	cd obj/dvi ; yes X | $(TEXI2DVI) ../../$<
	mv obj/dvi/$@ $@

.PHONY: ps
ps: cerberus.ps
%.ps: info/%.texinfo info/fdl.texinfo
	@mkdir -p obj/ps
	cd obj/ps ; yes X | texi2pdf --ps ../../$<
	mv obj/ps/$@ $@


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
	-rm -f -- "$(DESTDIR)$(PREFIX)$(INSTALL_BIN)/$(COMMAND)"
	-rm -f -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)/COPYING"
	-rm -f -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)/LICENSE"
	-rmdir -f -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"
	-rm -f -- "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info"
	-rm -f -- "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).pdf"
	-rm -f -- "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).ps"
	-rm -f -- "$(DESTDIR)$(PREFIX)$(DATA)/doc/$(PKGNAME).dvi"



.PHONY: clean
clean:
	-rm -rf -- bin obj cerberus.info cerberus.pdf cerberus.ps cerberus.dvi

