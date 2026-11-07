CC := c99
C_FLAGS := -Wall -Wextra -pedantic

objects := main.o item.o lista.o lista_seq.o lista_cruz.o
target := trabalho

vpath %.h headers
vpath %.c src
vpath %.o obj

run: $(target)
	./$(target)

all: $(target)

$(target): $(objects)
	$(CC) -o $(target) $(objects) $(C_FLAGS)

$(objects): %.o: %.c obj
	$(CC) -c $< -o obj/$@

obj:
	mkdir obj

clean:
	rm -f $(target) obj/*

.PHONY: run all clean
