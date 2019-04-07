SOURCES = \
	core.cvc coreio.cvc array.cvc test_core.cvc test_coreio.cvc test_array.cvc

HEADERS = civic.h

COMPILER = ../../bin/civcc

%.as: %.cvc
	$(COMPILER) $^ -o $@
%.o: %.as
	civas $^ -o $@

compile: $(SOURCES:.cvc=.o)

assemble: $(SOURCES:.cvc=.as)

all: compile

run: compile
	@echo "Testing core.cvc"
	@echo "------------------------------------"
	civvm test_core.o core.o
	@echo "------------------------------------"
	@echo "Testing coreio.cvc"
	@echo "------------------------------------"
	civvm test_coreio.o coreio.o core.o
	@echo "------------------------------------"
	@echo "Testing array.cvc"
	@echo "------------------------------------"
	civvm test_array.o array.o
	@echo "------------------------------------"

test: run

clean:
	rm -f *.o *.as
