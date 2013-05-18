TARGET := graph3
C_SOURCES :=
CXX_SOURCES := window.cpp
SOURCES := $(C_SOURCES) $(CXX_SOURCES)
C_OBJECTS := ${C_SOURCES:.c=.o}
CXX_OBJECTS := ${CXX_SOURCES:.cpp=.o}
OBJECTS = $(C_OBJECTS) $(CXX_OBJECTS)

INCLUDE_DIRS :=
LIBRARY_DIRS :=
LIBRARIES := sfml-window GL GLU

REL_FLAGS := -O2
DBG_FLAGS := -g -DDEBUG
CPPFLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))
CFLAGS   +=
CXXFLAGS += -Wall -std=c++11 `pkg-config --cflags gtkmm-2.4`
LDFLAGS  += $(foreach librarydir,$(LIBRARY_DIRS),-L$(librarydir))
LDFLAGS  += $(foreach library,$(LIBRARIES),-l$(library)) `pkg-config --libs gtkmm-2.4`

CC := gcc
CXX := g++

all: release

release: CXXFLAGS += $(REL_FLAGS)
release: $(TARGET)

debug: CXXFLAGS += $(DBG_FLAGS)
debug: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

-include ${OBJECTS:.o=.d}

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $< > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	    sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@$(RM) $*.d.tmp

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MM $< > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	    sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@$(RM) $*.d.tmp

.PHONY: clean cleanest

clean:
	@-$(RM) $(OBJECTS) ${OBJECTS:.o=.d}

cleanest:
	@-$(RM) $(OBJECTS) $(TARGET) ${OBJECTS:.o=.d}
