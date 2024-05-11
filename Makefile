EXECUTABLE = beehive

# The following line looks for a project's main() in files named project*.cpp,
# executable.cpp (substituted from EXECUTABLE above), or main.cpp
PROJECTFILE = $(or $(wildcard project*.cpp $(EXECUTABLE).cpp), main.cpp)
# If main() is in another file delete line above, edit and uncomment below
#PROJECTFILE = mymainfile.cpp

# designate which compiler to use
CXX         = g++

# rule for creating objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

# list of test drivers (with main()) for development
TESTSOURCES = $(wildcard test*.cpp)

# list of sources used in project
SOURCES     = $(wildcard *.cpp)
SOURCES     := $(filter-out $(TESTSOURCES), $(SOURCES))
# list of objects used in project
OBJECTS     = $(SOURCES:%.cpp=%.o)

# Default Flags
CXXFLAGS = -std=c++17 -Wconversion -Wall -Wextra -pedantic -framework CoreFoundation -framework CoreGraphics -framework ImageIO -framework CoreServices

# Build all executables
all: debug release valgrind

debug:
	make beehive_debug

release:
	make beehive_release

valgrind:
	make beehive_valgrind

# make debug - will compile sources with $(CXXFLAGS) -g3 and -fsanitize
#              flags also defines DEBUG and _GLIBCXX_DEBUG
beehive_debug: beehive.cpp screencapture.cpp
	$(CXX) $(CXXFLAGS) -g3 -DDEBUG -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG $(SOURCES) -o $(EXECUTABLE)_debug

beehive_release: beehive.cpp screencapture.cpp
	$(CXX) $(CXXFLAGS) -O3 $(SOURCES) -o $(EXECUTABLE)_release

# make valgrind - will compile sources with $(CXXFLAGS) -g3 suitable for
#                 CAEN or WSL (DOES NOT WORK ON MACOS).
beehive_valgrind: CXXFLAGS += -g3
beehive_valgrind: beehive.cpp screencapture.cpp
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(EXECUTABLE)_valgrind

# make clean - remove .o files, executables, tarball
clean:
	rm -Rf *.dSYM
	rm -f $(OBJECTS) $(EXECUTABLE) $(EXECUTABLE)_debug
	rm -f $(EXECUTABLE)_valgrind
.PHONY: clean
