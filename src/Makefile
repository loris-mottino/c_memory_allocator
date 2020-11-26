#déclaration des variables
CC = gcc
EXEC = prog
SRC = $(willcard*.c)
OBJ = $(SRC:.c:.o)

all : $(EXEC)

#compile tous les fichiers .c en fichier .o
%.o : %.c
	$(CC) -o $@ -c $<

#compile l'éxécuteur
$(EXEC) : $(OBJ)
	$(CC) -o $@ $^

#nettoie tous les fichiers objets
clean :
	rm -rf *o
