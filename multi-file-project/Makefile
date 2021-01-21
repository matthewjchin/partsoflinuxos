bin=inspector

# Set the following to '0' to disable log messages:
debug=1

CFLAGS += -Wall -g
LDFLAGS +=

src=inspector.c str_fncs.c
obj=$(src:.c=.o)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) -DDEBUG=$(debug) $(obj) -o $@

inspector.o: inspector.c logger.h str_fncs.h str_fncs.c
str_fncs.o: str_fncs.h str_fncs.c logger.h

clean:
	rm -f inspector $(obj)


# Tests --

test: inspector ./tests/run_tests
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	git submodule update --init --remote

testclean:
	rm -rf tests
