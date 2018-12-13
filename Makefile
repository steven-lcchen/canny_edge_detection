SRC := ./src

.PHONY: all clean

all: test_CmdLineParser test_threshold test_canny 
 
# Test command line parser
test_CmdLineParser: obj/test_CmdLineParser.o
	./compile.sh -o $@ $^

# Test otsu threshold algorithm
test_threshold: obj/test_threshold.o obj/otsu_threshold.o obj/LabelConnected.o
	./compile.sh -o $@ $^

# Test Canny edge detection
test_canny: obj/test_canny.o obj/MyColorToGray.o obj/MedianFilter.o obj/BoxFilter.o obj/MyCanny.o obj/LabelConnected.o obj/otsu_threshold.o
	./compile.sh -o $@ $^

# Compile source codes
obj/%.o: $(SRC)/%.cpp ./inc/define.hpp
	./compile.sh -c -o $@ $< -Iinc

obj/test_%.o: $(SRC)/%.cpp inc/define.hpp
	./compile.sh -c -o $@ $< -Iinc

clean:
	\rm -f obj/*.o ./test_*
