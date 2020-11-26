#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>


// Déclaration de la structure de bloc libre

struct fb;



// Déclaration des méthodes

// CODE



// Structure de bloc libre (fb_t)

typedef struct fb {
	size_t taille; // Stocke la taille utilisable du bloc
	struct fb *prochain; // Pointeur vers le prochain bloc libre
} bloc_libre;

/* NOTA BENE : Un bloc occupé ne possède pas structure, car son adresse est retenue par l'utilisateur.
 * Néanmoins, il possède une taille stockée 64 bits avant le pointeur vers la mémoire de ce bloc.
 */



// Déclaration du pointeur qui stocke toute la mémoire allouée.
void *memoire;



/* 
 * 
 * 
 * 
 */



int main() {
  return 0;
}
