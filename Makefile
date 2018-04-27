CC=gcc
CFLAGS=-g -Wall -Werror
LIBS=-lpthread
EXE=arcport
OBJ=queue.o arcport.o

all: $(EXE)
	@echo "***************************"
	@echo "Compilation successfully!"
	@echo ""

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(EXE): $(OBJ)
	$(CC) $^ $(LIBS) -o $@

clean:
	rm -f $(EXE) $(OBJ)
	@echo "***************************"
	@echo "Deleted files!"
	@echo  ""
