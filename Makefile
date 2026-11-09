CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -pedantic
OBJDIR := obj
SOURCES := main.c arquivo.c query.c item.c lista.c lista_seq.c lista_cruz.c estrutura.c
OBJECTS := $(SOURCES:%.c=$(OBJDIR)/%.o)
TARGET := trabalho

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -lm

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -Iheaders -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
