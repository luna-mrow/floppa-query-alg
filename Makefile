CC      := c99
C_FLAGS := -Wall -Wextra -pedantic
target  := trabalho

# falta os .o dos modulos novos (lista_seq, lista_ord, lista_cruz...)
objects := main.o query.o item.o lista.o arquivo.o
OBJ     := $(addprefix obj/,$(objects))

all: $(target)

$(target): $(OBJ)
	$(CC) $(C_FLAGS) -o $@ $^ -lm

obj/%.o: src/%.c | obj
	$(CC) $(C_FLAGS) -c $< -o $@

obj:
	mkdir -p obj

run: $(target)
	./$(target)

clean:
	rm -rf obj $(target)

.PHONY: all run clean
