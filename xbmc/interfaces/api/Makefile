SRCS=ApplicationOperations.cpp \
     AudioLibrary.cpp \
     FileItemHandler.cpp \
     FileOperations.cpp \
     GUIOperations.cpp \
     InputOperations.cpp \
     PlayerOperations.cpp \
     PlaylistOperations.cpp \
     SystemOperations.cpp \
     VideoLibrary.cpp \
     XBMCOperations.cpp \

LIB=generic-api.a

include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
