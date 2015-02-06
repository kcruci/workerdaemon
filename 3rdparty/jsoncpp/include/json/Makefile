
################################################################################
C      = gcc
CXX     = g++
RANLIB  = ranlib
AR      = ar
RM      = rm -f
################################################################################
C_FLAGS    =  -Wall -Werror -O
DEBUGFLAGS = -g -DDEBUG -ggdb
CC_FLAGS   = $(C_FLAGS)
###############################################################################

#if (test -d	../lib) ; then echo `pwd` ;	else mkdir ../lib ; fi

###############################################################################

#得到CPP/OBJ文件的名称
BASECPP   = $(wildcard *.cpp )
BASEOBJ   = $(patsubst %.cpp, .obj/%.o, $(BASECPP))


CC_FLAGS       	= $(C_FLAGS)
INC				=	-I../


################################################################################


define buildinfo
	@echo -e "$(CXX) -c $(CC_FLAGS)  $< \e[36;1m -o $@ \e[0m\n"
endef

define building
	@(errmsg=$$( $(CXX) $(INC) -c $(CC_FLAGS) $< -o $@  2>&1); r=$$?; \
		if [[ ! -z $$errmsg ]]; then \
			echo -ne "$(CXX) $(INC) -c $(CC_FLAGS) $< -o $@\n\n"\
			echo -ne "\e[31;1m $$errmsg\n \e[0m";\
			exit $$r;\
		fi\
	)
endef

define exe_building2
	@echo -e "$(CXX) $(CC_FLAGS)  $*.cpp \e[36;1m -o $* \e[0m"
    @(errmsg=$$( $(CXX) $(CC_FLAGS)  $*.cpp $(INC_BASE) $(LIB_BASE) -o $* 2>&1); r=$$?; \
	        if [[ ! -z $$errmsg ]]; then \
				echo -ne "$(CXX) $(CC_FLAGS)  $*.cpp $(INC_BASE) $(LIB_BASE) -o $*\n\n"\
				echo -ne "\e[31;1m $$errmsg\n \e[0m";\
				exit $$r;\
			fi\
	)
endef

define obj_building2
	@echo -e "$(CXX) -c $(CC_FLAGS) $< \e[36;1m -o $@ \e[0m\n"
	@(errmsg=$$( $(CXX) $(INC) -c $(CC_FLAGS) $< -o $@ 2>&1 ); r=$$?; \
		if [[ ! -z $$errmsg ]]; then \
			echo -ne "$(CXX) $(INC) -c $(CC_FLAGS) $< -o  $@\n\n"\
			echo -ne "\e[31;1m $$errmsg\n \e[0m";\
			exit $$r;\
		fi\
	)
endef

################################################################################



#输出文件
BASELIB	       = ../../lib/libjsoncpp.a



all: $(BASELIB)

$(BASELIB) : $(BASEOBJ)
	$(AR) rc $(BASELIB)	$(BASEOBJ)

.obj/%.o : %.cpp
	$(call buildinfo)
	$(call building)

debug:
	$(MAKE)	$(MFLAGS) C_FLAGS="$(C_FLAGS) $(DEBUGFLAGS)" all

clean:
	$(RM) $(BASELIB)  $(BASEOBJ)


