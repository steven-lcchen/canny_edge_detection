.PHONY: all clean

all: test_CmdLineParser test_threshold test_canny 
 
# Test command line parser
test_CmdLineParser: test_CmdLineParser.o
	./compile.sh -o $@ $^

# Test otsu threshold algorithm
test_threshold: test_threshold.o otsu_threshold.o LabelConnected.o
	./compile.sh -o $@ $^

# Test Canny edge detection
test_canny: test_canny.o MyColorToGray.o MedianFilter.o BoxFilter.o MyCanny.o LabelConnected.o
	./compile.sh -o $@ $^

# Compile source codes
%.o: %.cpp define.hpp
	./compile.sh -c -o $@ $<

clean:
	\rm -f *.o 
