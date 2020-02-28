all: homeworkFT homeworkFFT inputGenerator compareOutputs

compareOutputs: compareOutputs.c
	gcc -o compareOutputs compareOutputs.c -O3 -lm -Wall

inputGenerator: inputGenerator.c
	gcc -o inputGenerator inputGenerator.c -O3 -lm -Wall

homeworkFT: homeworkFT.c
	gcc -o homeworkFT homeworkFT.c -O3 -lpthread -lm -Wall

homeworkFFT: homeworkFFT.c
	gcc -g -o homeworkFFT homeworkFFT.c -O3 -lpthread -lm -Wall


check:
	make
	./homeworkFFT ft1.data.in out.txt 4
	./compareOutputs ft1.data.out out.txt

clean:
	rm homeworkFFT homeworkFT inputGenerator compareOutputs