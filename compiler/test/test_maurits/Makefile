SOURCES = \
	core.cvc main.cvc coreio.cvc array.cvc

HEADERS = civic.h

%.as: %.cvc
	civcc $^ -o $@
%.out: %.as
	civas $^ -o $@

compile: $(SOURCES:.cvc=.out)

all: compile

run: compile
	civvm *.out

clean:
	rm -f *.out *.as
