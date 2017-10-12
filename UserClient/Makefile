#-----------------------------------
#This is a makefile template for some C/C++ objects which will work on Linux.
#-----------------------------------

#Put your include files for makefile here.
#include /*include files*/

#The include file pathes.
INCLUDEPATH=

#The libs you may need.
LIBS=

#The pathes of libs.
LIBPATH= -lpthread

#Compiler.
CC= g++

#Compiler flags and settings.
CCFLAGS= -g



#The finale target program to be made.
TARGET= client

#Intermediate files.
OBJECTS= main.o


#"-fPIC -shared" may needed if you are making a dynamic library.
TARGET: $(OBJECTS)
	$(CC) $(CCFLAGS) $^ -o $(TARGET) $(INCLUDEPATH) $(LIBPATH) $(LIBS)

#You need different pattern rules to get intermediate files of different types of C files.
#"-fPIC" may needed if you are making a dynamic library
%.o:%.cpp
	$(CC) $(CCFLAGS) -c $< $(INCLUDEPATH) -o $@

#Install your program or dynamic library.
install:
#	cp $(TARGET) $(HOME)/bin(lib)
#	cp $(TARGET) $(HOME)/src/bin(lib)

#Remove the intermediate files.
.PHONY: clean cleanobj cleanpro
clean:
	-rm -f $(TARGET) $(OBJECTS)

cleanobj:
	-rm -f $(OBJECTS)

cleanpro:
	-rm -f $(TARGET)



