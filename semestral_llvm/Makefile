run:
	./pjp

compile:
	g++ -o pjp *.cpp `llvm-config --libs core native engine interpreter --cxxflags --ldflags --system-libs --libs all`
	mkdir obj bin

clean:
	rm pjp output.o a.out 2>/dev/null \
	rm -r bin/ obj/ 2>/dev/null

tests:
	./toBin.bash


