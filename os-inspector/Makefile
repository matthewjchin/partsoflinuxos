# Set the following to '0' to disable log messages:
debug=1

inspector: inspector.c
	gcc -g -Wall -DDEBUG=$(debug) $< -o $@

clean:
	rm -f inspector


# Tests --

test: inspector ./tests/run_tests
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	git submodule update --init --remote

testclean:
	rm -rf tests
