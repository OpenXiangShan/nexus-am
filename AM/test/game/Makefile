ARCH = $(shell ../../magic.py arch)
CXXFLAGS = $(shell ../../magic.py cxxflags) -I../../include -I../../arch/$(ARCH)/include 

all: main.o draw.o
	@echo "Done."

-include *.d

clean:
	rm *.o
