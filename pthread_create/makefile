
.PHONY:all
all:run
	@echo this my first make

run:main.o
	g++ -o run main.o -lpthread

main.o:common.h

.PHONY:clean	
clean:
	rm -rf A.o main.o run
