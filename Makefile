
all:
	$(MAKE) -C src all

check: all
	$(MAKE) -C tests

clean:
	$(MAKE) -C src clean

.PHONY: all check clean
