.PHONY: build

build: simple jseval mt19937 rand

.PHONY: simple

simple:
	$(MAKE) -C simple

.PHONY: jseval

jseval:
	$(MAKE) -C jseval

.PHONY: mt19937

mt19937:
	$(MAKE) -C mt19937

.PHONY: rand

rand:
	$(MAKE) -C rand

.PHONY: rand_so

rand_so:
	$(MAKE) -C rand_so

.PHONY: run

run:
	$(MAKE) -C simple run
	$(MAKE) -C jseval run
	$(MAKE) -C mt19937 run
	$(MAKE) -C rand run
