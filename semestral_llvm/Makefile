run:
	./pjp

compile:
	g++ -o pjp *.cpp `llvm-config --libs core native engine interpreter --cxxflags --ldflags --system-libs --libs all`

clean:
	rm pjp
