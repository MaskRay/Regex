CFLAGS += -std=c99

main: main.o parse.o capture.o compile.o vm.o
main.o parse.o capture.o compile.o vm.o: regex.h
