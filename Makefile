
include make.settings.local

###############################################################################

include $(STXXL_PATH)/pmstxxl.mk

OPENMP_OPTIONS ?= -fopenmp

STXXL_COMPILER_OPTIONS =  $(STXXL_CPPFLAGS)
STXXL_COMPILER_OPTIONS += $(OPENMP_OPTIONS)
STXXL_COMPILER_OPTIONS += -DSORT_OPTIMAL_PREFETCHING
STXXL_COMPILER_OPTIONS += -DUSE_MALLOC_LOCK
# STXXL_COMPILER_OPTIONS += -DCOUNT_WAIT_TIME
STXXL_COMPILER_OPTIONS += -DSTXXL_FREE_EXTMEMORY_ON_VECTOR_RESIZE
STXXL_COMPILER_OPTIONS += -DMERGE_PREFETCH_NEW
STXXL_COMPILER_OPTIONS += -DSTXXL_PARALLEL_MODE_EXPLICIT 
STXXL_COMPILER_OPTIONS += -I include
STXXL_COMPILER_OPTIONS += -std=c++0x
#STXXL_COMPILER_OPTIONS += -ggdb
STXXL_COMPILER_OPTIONS += $(OPTIONS)
STXXL_COMPILER_OPTIONS += $(OPTIMIZATION_DEBUG)


STXXL_LINKER_OPTIONS =  $(STXXL_LDLIBS)
STXXL_LINKER_OPTIONS += $(OPENMP_OPTIONS) -lm -lnlopt

###############################################################################

.PHONY: all

all: pscore

###############################################################################

#demsort: Makefile make.settings.local demsort.o env.o ${STXXL_PATH}/lib/libstxxl.a
#	${CXX} demsort.o env.o -o $@ ${STXXL_LINKER_OPTIONS}
ONAMES_PSCORE = 			fastscore.assoc \
					balanced_load	\
					processdir      \
					prepare_space	\
					pack_comm	\
					strspec		\
					fix_borders	\
					processalign	\
					lex-score	\
					anyoption   \
					lex-map		\
					pscore.mpi  \
					fastscore	\
					tdiscounter \
					out.mpi

mk_OFILES            = $(addsuffix .o,$(1))

OFILES_PSCORE	     = $(call mk_OFILES,$(ONAMES_PSCORE))


pscore: Makefile make.settings.local  $(OFILES_PSCORE)  ${STXXL_PATH}/lib/libstxxl.a
	${CXX} $(OFILES_PSCORE) -o $@ ${STXXL_LINKER_OPTIONS}

%.o: %.cpp Makefile make.settings.local
	${CXX} -c -o $@ $< ${STXXL_COMPILER_OPTIONS} -DDEMSORT_LIBRARY

###############################################################################

.PHONY: clean

RM = rm -f

DEP = makefile.dep

clean:
	$(RM) $(DEP)
	$(RM) *.o

distclean: clean
	$(RM) pscore
###############################################################################

FILES = $(wildcard *.cpp)

$(DEP): Makefile make.settings.local
	echo "Creating makefile.dep"
	echo '# DO NOT EDIT THIS FILE -- it is automatically generated' > $@
	echo '' >> $@

	for file in $(FILES); \
	do \
		${CXX} -MM -c $$file ${STXXL_COMPILER_OPTIONS}; \
		echo ' '; \
	done | \
	awk 'BEGIN {RS=" "} (! /^\//) {if($$0!="\\\n"){\
	        if($$0 ~ /:/) {\
	                sub(/\n/,"",$$0);\
		        printf "\n%s \\\n", $$0\
	        } else {\
	                if($$0 ~ /\n/){\
	                        print "",$$0\
	                } else {\
	                        print "",$$0,"\\"\
	                }\
	        }\
	}}' - >> $@

-include $(DEP)

###############################################################################

HEADERS  = $(wildcard *.h)

%.hct.cpp:
	echo '#include "$*.h"' > $@

header-compile-test: $(HEADERS:.h=.hct.o)

header-compile-test-clean:
	$(RM) *.hct.*