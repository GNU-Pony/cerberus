USR_PREFIX = /usr
LOCAL_PREFIX = $(USR_PREFIX)/local
BIN = /bin
SBIN = /sbin
DEV = /dev

EXTRA_CPP_FLAGS = 
# see configurable-definitions

_LB = $(LOCAL_PREFIX)$(BIN)
_UB = $(USR_PREFIX)$(BIN)
_SB = $(BIN)
_LS = $(LOCAL_PREFIX)$(SBIN)
_US = $(USR_PREFIX)$(SBIN)
_SS = $(SBIN)

TTY_GROUP = tty
DEFAULT_HOME = /
DEFAULT_SH = sh
DEFAULT_SHELL = $(BIN)/$(DEFAULT_SH)
DEFAULT_TERM = dumb
VCS = $(DEV)/vcs
VCSA = $(DEV)/vcsa
PATH = $(_LB):$(_UB):$(_SB)
PATH_ROOT = $(_LS):$(_LB):$(_US):$(_UB):$(_SS):$(_SB)

H = \#
VCS_LEN = $(shell vcs="$(VCS)" ; echo "$${$(H)vcs}")
VCSA_LEN = $(shell vcsa="$(VCSA)" ; echo "$${$(H)vcsa}")
VCS_VCSA_LEN = $(shell (echo $(VCS_LEN) ; echo $(VCSA_LEN)) | sort -n | tail -n 1)

STR_DEFS = TTY_GROUP DEFAULT_HOME DEFAULT_SH DEFAULT_SHELL DEFAULT_TERM PATH PATH_ROOT VCS VCSA
INT_DEFS = VCS_LEN VCSA_LEN VCS_VCSA_LEN

STR_CPPFLAGS = $(foreach D, $(STR_DEFS), -D'$(D)="$($(D))"')
INT_CPPFLAGS = $(foreach D, $(INT_DEFS), -D'$(D)=$($(D))')

OPTIMISE = -Os
CPPFLAGS = $(EXTRA_CPP_FLAGS) $(STR_CPPFLAGS) $(INT_CPPFLAGS)
LDFLAGS = 
CFLAGS = -std=gnu99 -Wall -Wextra

CC_FLAGS = $(CPPFLAGS) $(CFLAGS) $(OPTIMISE)
LD_FLAGS = $(LDFLAGS) $(CFLAGS) $(OPTIMISE)

SRC = cerberus passphrase quit security login pam
OBJ = $(foreach S, $(SRC), obj/$(S).o)


.PHONY: all
all: bin/cerberus


bin/cerberus: $(OBJ)
	@mkdir -p bin
	$(CC) $(LD_FLAGS) -o "$@" $^


obj/%.o: src/%.c src/%.h src/config.h
	@mkdir -p obj
	$(CC) $(CC_FLAGS) -o "$@" -c "$<"


.PHONY: clean
clean:
	-rm -r bin obj

