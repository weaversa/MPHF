EXTRAS = Makefile LICENSE.md README.md LICENSE.xxHash	\
test/test.c

HEADERS = lib/c_list_types/include/c_list_types.h	\
include/mphf_hashes.h include/mphf.h include/xxhash.h

SOURCES = src/list_types.c src/mphf_hashes.c src/mphf_build.c	\
src/mphf_query.c src/external_solve.c src/xxhash.c

OBJECTS = $(SOURCES:src/%.c=obj/%.o)

MPHFCNFLIB = mphfcnf
CC = gcc
#DBG = -g -Wall -fstack-protector-all -pedantic
OPT = -march=native -O3 -DNDEBUG -ffast-math -fomit-frame-pointer -finline-functions
INCLUDES = -Iinclude
LIBS = -l$(MPHFCNFLIB) -lm
LDFLAGS = -Llib
CFLAGS = -std=gnu99 $(DBG) $(OPT) $(INCLUDES) -fopenmp
AR = ar r
RANLIB = ranlib

all: depend lib/lib$(MPHFCNFLIB).a

depend: .depend
.depend: $(SOURCES)
	@echo "Building dependencies" 
ifneq ($(wildcard ./.depend),)
	@rm -f "./.depend"
endif
	@$(CC) $(CFLAGS) -MM $^ > .depend
# Make .depend use the 'obj' directory
	@sed -i.bak -e :a -e '/\\$$/N; s/\\\n//; ta' .depend
	@sed -i.bak 's/^/obj\//' .depend
	@rm -f .depend.bak
-include .depend

$(OBJECTS): obj/%.o : src/%.c Makefile
	@echo "Compiling "$<""
	@[ -d obj ] || mkdir -p obj
	@$(CC) $(CFLAGS) -c $< -o $@

lib/lib$(MPHFCNFLIB).a: $(OBJECTS) Makefile
	@echo "Creating "$@""
	@[ -d lib ] || mkdir -p lib
	@rm -f $@
	@$(AR) $@ $(OBJECTS)
	@$(RANLIB) $@

test/test: test/test.c lib/lib$(MPHFCNFLIB).a
	$(CC) $(CFLAGS) $(LDFLAGS) test/test.c -o test/test $(LIBS)

clean:
	rm -rf *~ */*~ $(OBJECTS) ./.depend test/test *.dSYM test/test.dSYM lib/lib$(MPHFCNFLIB).a obj

edit:
	emacs -nw $(SOURCES) $(HEADERS) $(EXTRAS)
