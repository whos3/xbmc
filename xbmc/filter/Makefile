SRCS=Filter.cpp \
     FilterRule.cpp \
     FilterRuleCombination.cpp \

LIB=filter.a

include ../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(patsubst %.S,,$(SRCS))))
