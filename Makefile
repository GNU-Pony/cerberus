BIN = /bin
DEV = /dev

EXTRA_CPP_FLAGS = 
# -DOWN_VCS -DOWN_VCSA -DUSE_TTY_GROUP

TTY_GROUP = tty
DEFAULT_HOME = /
DEFAULT_SHELL = $(BIN)/sh
DEFAULT_TERM = dumb
VCS = $(DEV)/vcs
VCSA = $(DEV)/vcsa

H = \#
VCS_LEN = $(shell vcs="$(VCS)" ; echo "$${$(H)vcs}")
VCSA_LEN = $(shell vcsa="$(VCSA)" ; echo "$${$(H)vcsa}")
VCS_VCSA_LEN = $(shell (echo $(VCS_LEN) ; echo $(VCSA_LEN)) | sort -n | tail -n 1)

STR_DEFS = TTY_GROUP DEFAULT_HOME DEFAULT_SHELL DEFAULT_TERM
INT_DEFS = VCS_LEN VCSA_LEN VCS_VCSA_LEN

OPTIMISE = -Os
CPPFLAGS = $(EXTRA_CPP_FLAGS) $(foreach D, $(INT_DEFS), -D'$(D)=$($(D))') $(foreach D, $(STR_DEFS), -D'$(D)="$($(D))"')
LDFLAGS = 
CFLAGS = -std=gnu99 -Wall -Wextra

CC_FLAGS = $(CPPFLAGS) $(CFLAGS) $(OPTIMISE)
LD_FLAGS = $(LDFLAGS) $(CFLAGS) $(OPTIMISE)

SRC = cerberus passphrase quit security
OBJ = $(foreach S, $(SRC), obj/$(S).o)


.PHONY: all
all: bin/cerberus


bin/cerberus: $(OBJ)
	@mkdir -p bin
	$(CC) $(LD_FLAGS) -o "$@" $^


obj/%.o: src/%.c src/%.h
	@mkdir -p obj
	$(CC) $(CC_FLAGS) -o "$@" -c "$<"


.PHONY: clean
clean:
	-rm -r bin obj

