bin=www

# Set the following to '0' to disable log messages:
debug=1

CFLAGS += -Wall -g -DDEBUG=$(debug) 
LDFLAGS +=

src=www.c
obj=$(src:.c=.o)

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@


www.o: www.c

clean:
	rm -f $(bin) $(obj)


# Tests --

test: $(bin) ./tests/run_tests
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	git submodule update --init --remote

testclean:
	rm -rf tests
