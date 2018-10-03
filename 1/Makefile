CC = mpicc
CXX = mpicxx
CFLAGS = -Wall -O3 -std=gnu99
CXXFLAGS = -Wall -O3 -std=c++11

STUDENTID = $(USER:p%=%)
TARGETS1 = HW1_$(STUDENTID)_basic
SOURCES1 = HW1_$(STUDENTID)_basic.cc
TARGETS2 = HW1_$(STUDENTID)_advanced
SOURCES2 = HW1_$(STUDENTID)_advanced.cc
TARGETS3 = HW1_$(STUDENTID)_basic_calc
SOURCES3 = HW1_$(STUDENTID)_basic_calc.cc
TARGETS4 = HW1_$(STUDENTID)_advance_cacl
SOURCES4 = HW1_$(STUDENTID)_advance_cacl.cc


OBJECTS = $(SOURCES1:.cc=.o)
OBJECTS = $(SOURCES2:.cc=.o)

#.PHONY: all
all: $(SOURCES) $(TARGETS)

$(TARGET1): $(OBJECT1)
	$(CXX) $(CXXFLAGS) $(OBJECT1) -o $@
$(TARGET2): $(OBJECT2)
	$(CXX) $(CXXFLAGS) $(OBJECT2) -o $@
$(TARGET3): $(OBJECT3)
	$(CXX) $(CXXFLAGS) $(OBJECT3) -o $@
$(TARGET4): $(OBJECT4)
	$(CXX) $(CXXFLAGS) $(OBJECT4) -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
#.PHONY: clean
clean: 
	rm -f $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) *.o 