/* On inclut l'interface publique */
#include "mem.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

/* Définition de l'alignement recherché
 * Avec gcc, on peut utiliser __BIGGEST_ALIGNMENT__
 * sinon, on utilise 16 qui conviendra aux plateformes qu'on cible
 */
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/* structure placée au début de la zone de l'allocateur

   Elle contient toutes les variables globales nécessaires au
   fonctionnement de l'allocateur

   Elle peut bien évidemment être complétée
*/
struct allocator_header {
        size_t memory_size;
	mem_fit_function_t *fit;
	struct fb *list;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void* memory_addr;

static inline void *get_system_memory_addr() {
	return memory_addr;
}

static inline struct allocator_header *get_header() {
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

static inline size_t get_system_memory_size() {
	return get_header()->memory_size;
}


struct fb {
	size_t size;
	struct fb* next;
};


void mem_init(void* mem, size_t taille)
{
        memory_addr = mem;
        *(size_t*)memory_addr = taille;  // get_header() -> memory_size = taille;
	/* On vérifie qu'on a bien enregistré les infos et qu'on
	 * sera capable de les récupérer par la suite
	 */
	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());
	
	get_header()->list = get_system_memory_addr() + sizeof(struct allocator_header);
	*(get_header()->list) = (struct fb) {
		taille - sizeof(struct allocator_header) - sizeof(struct fb),
		NULL
	};
	
	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int)) {
	void *current = get_system_memory_addr();
	struct fb *free_block = get_header()->list;
	
	while (current < get_system_memory_addr() + sizeof(struct allocator_header) + get_system_memory_size()) {
		int is_free = current == (void*)free_block ? 1 : 0; // Is current block free ?
		size_t size = *(size_t*)current; // Size of current block
		print(current, size, is_free); // Prints current block with its address, size, and state (free/occuped)
		if (is_free == 1) {
			free_block = free_block->next;
			current += sizeof(struct fb) + size;
		} else
			current += sizeof(size_t) + size;
	}
	
//	/* ... */
//	while (/* ... */ 0) {
//		/* ... */
//		print(/* ... */NULL, /* ... */0, /* ... */0);
//		/* ... */
//	}
}

void mem_fit(mem_fit_function_t *f) {
	get_header()->fit = f;
}

void *mem_alloc(size_t taille) {
	/* ... */
	__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */
	struct fb *fb=get_header()->fit(/*...*/NULL, /*...*/0);
	/* ... */
	return NULL;
}


void mem_free(void* mem) {
	// Defines free blocks before and after given occuped block (mem), and initializes a free block for mem
	struct fb *before = NULL,
		  *current = (struct fb*)(mem - sizeof(size_t)),	  
		  *after = get_header()->list;
	
	// Retrieves blocks before and after current block that are free
	while (after != NULL && after < current) {
		before = after;
		after = after->next;
	}
	
	// Switches mem to a free block
	*current = (struct fb) {
		mem_get_size(mem) + sizeof(size_t) - sizeof(struct fb),
		after
	};
	
	// Defines states of blocks before and after current block
	int before_is_free = before != NULL && current == before + sizeof(struct fb) + before->size ? 1 : 0,
	    after_is_free = after != NULL && after == current + sizeof(struct fb) + current->size ? 1 : 0;
	
	// If block after is free, reassembles current block and block after
	if (after_is_free) {
		current->size += sizeof(struct fb) + after->size;
		current->next = after->next;
	}
	
	// If block before is free, reassembles current block and block before
	if (before_is_free) {
		before->size += sizeof(struct fb) + current->size;
		before->next = current->next;
	}
}


struct fb* mem_fit_first(struct fb *list, size_t size) {
	return NULL;
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return *(size_t*) (zone - sizeof(size_t));
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb* mem_fit_best(struct fb *list, size_t size) {
	return NULL;
}

struct fb* mem_fit_worst(struct fb *list, size_t size) {
	return NULL;
}
