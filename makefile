all: Main

Main:
	cd src; g++ -g main.cpp routines.cpp -o pa3; mv pa3 ..

clean:
	rm pa3;