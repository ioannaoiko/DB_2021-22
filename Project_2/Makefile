sht:
	@echo " Compile ht_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sht_main.c ./src/hash_file.c ./src/sht_file.c -lbf -o ./build/runner -O2

ht:
	@echo " Compile ht_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/hash_file.c -lbf -o ./build/runner -O2

bf:
	@echo " Compile bf_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c -lbf -o ./build/runner -O2



# paths
INCLUDE = include
SRC = src
LIB = lib
EXAMPLES = examples
BIN = bin
# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CFLAGS = -Wall -no-pie -fPIE -g -I$(INCLUDE)

# Αρχεία .o
INCL = $(INCLUDE)/bf.h $(INCLUDE)/hash_file.h $(INCLUDE)/sht_file.h
OBJS = $(EXAMPLES)/ht_main.o $(SRC)/hash_file.o $(SRC)/sht_file.o $(LIB)/libbf.so
OBJS1 = $(EXAMPLES)/ht_main.o $(SRC)/hash_file.o $(SRC)/sht_file.o


# the executable program
EXEC = ht_main
# parameters
ARGS =

$(EXEC): $(OBJS) $(INCL)
	gcc $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJS1) $(EXEC)
	rm *.db

run: $(EXEC)
	./$(EXEC) $(ARGS)

gdb: $(OBJS)
	gcc $(OBJS) -g3 -o $(EXEC)
	gdb $(EXEC)
