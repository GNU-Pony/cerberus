OPTIMISE = -Os
CPPFLAGS = 
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

