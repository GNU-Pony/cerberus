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


.PHONY: all
all: cerberus doc


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
doc: info

.PHONY: info
info: cerberus.info.gz

%.info: info/%.texinfo
	makeinfo "$<"

%.gz: %
	gzip -9 < "$<" > "$@"


.PHONY: install
install: bin/cerberus cerberus.info.gz
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(INSTALL_BIN)"
	install  -m755 -- bin/cerberus "$(DESTDIR)$(PREFIX)$(INSTALL_BIN)/$(COMMAND)"
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"
	install  -m644 -- COPYING LICENSE "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"
	install -dm755 -- "$(DESTDIR)$(PREFIX)$(DATA)/info"
	install  -m644 -- cerberus.info.gz "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info.gz"


.PHONY: uninstall
uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)$(INSTALL_BIN)/$(COMMAND)"
	-rm -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)/COPYING"
	-rm -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)/LICENSE"
	-rmdir -- "$(DESTDIR)$(PREFIX)$(LICENSES)/$(PKGNAME)"
	-rm -- "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info.gz"



.PHONY: clean
clean:
	-rm -r bin obj cerberus.info.gz

