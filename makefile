all: Main

Main:
	cd src; g++ -g *.cpp -o pa3; mv pa3 ..

clean:
	rm pa3;