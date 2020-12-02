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

/* Structure placée au début de la zone de l'allocateur

    Elle contient toutes les variables globales nécessaires au
    fonctionnement de l'allocateur.
    
    Elle peut bien évidemment être complétée
    
    On y trouve :
    	- La taille de la mémoire exploitable par l'utilisateur, définie initialement dans mem_init()
	- La stratégie à utiliser lors de l'allocation de la mémoire (pointeur vers une fonction)
	- Un pointeur vers le premier bloc libre
*/
struct allocator_header {
        size_t memory_size;
	mem_fit_function_t *fit;
	struct fb *list;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header')
 */
static void* memory_addr;

// Retourne le pointeur du début de l'allocateur (struct allocator_header) (et également le champ memory_size de ce dernier).
static inline void *get_system_memory_addr() {
	return memory_addr;
}

// Retourne un pointeur vers la structure allocator_header, contenant les métadonnées globales de l'allocateur.
static inline struct allocator_header *get_header() {
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

// Retourne la taille de la mémoire initialement demandée par l'utilisateur dans mem_init().
static inline size_t get_system_memory_size() {
	return get_header()->memory_size;
}

/* Retourne la différence entre la taille des métadonnées d'un bloc libre et celle d'un bloc occupé.
 * En effet, un bloc libre possède des métadonnées plus importantes qu'un bloc occupé, et cette différence
 * de taille peut être utilisée pour allouer un peu plus de mémoire lors d'un mem_alloc().
 */
static inline size_t get_metadata_size_gap() {
	return sizeof(struct fb) - sizeof(size_t);
}


/* Structure représentant un bloc libre.
 * Un bloc libre a une taille allouable (size) et un pointeur vers le prochain bloc libre.
 * Notons que la taille réelle allouable d'un bloc libre est légèrement supérieure à la valeur size,
 * en raison de l'écart entre la taille des métadonnées des blocs libres à celle des blocs occupés.
 */
struct fb {
	size_t size;
	struct fb* next;
};


/* Fonction permettant d'initialiser l'allocateur avec une taille initiale et un pointeur vers la zone à utiliser.
 * Cette zone devra avoir été préalablement allouée par l'utilisateur, et la taille demandée ne peut pas être supérieure
 * à la taille de la zone allouée.
 */
void mem_init(void* mem, size_t taille) {
	// Il faut que taille demandée soit un multiple de ALIGNMENT et qu'il soit supérieur à celui-ci afin d'optimiser l'allocation de la mémoire.
	if (taille < (size_t) ALIGNMENT || taille % (size_t) ALIGNMENT != 0)
		return;
	
	// On définit la variable globale memory_addr par la valeur du pointeur renseigné par l'utilisateur.
        memory_addr = mem;
	// On renseigne dans nos métadonnées globales (struct allocator_header) la taille demandée par l'utilisateur.
        *(size_t*)memory_addr = taille;  // get_header() -> memory_size = taille;
	
	// On vérifie qu'on a bien enregistré les infos (métadonnées globales) et qu'on sera capable de les récupérer par la suite.
	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());
	
	/* On fait pointer la variable list des métadonnées globales (premier bloc libre de l'allocateur)
	 * vers l'adresse se situant juste après la structure allocator_header.
	 */
	get_header()->list = get_system_memory_addr() + sizeof(struct allocator_header);
	// On crée une structure de bloc libre à cette adresse, de taille maximale afin de remplir tout l'espace demandé par l'utilisateur.
	*(get_header()->list) = (struct fb) {
		taille - sizeof(struct allocator_header) - sizeof(struct fb),
		NULL
	};
	
	// On définit la stratégie d'allocation par mem_fit_first().
	mem_fit(&mem_fit_first);
}

// Cette fonction permet d'afficher dans le shell une représentation textuelle des blocs mémoire utilisés par l'allocateur.
void mem_show(void (*print)(void *, size_t, int)) {
	// On crée un pointeur vers le premier bloc (libre ou occupé) de l'allocateur.
	void *current = get_system_memory_addr() + sizeof(struct allocator_header);
	// On crée un pointeur vers le premier bloc libre de l'allocateur.
	struct fb *free_block = get_header()->list;
	
	/* Boucle permettant de parcourir tous les blocs mémoire de l'allocateur.
	 * Elle s'arrêtera lorsque la variable current pointera vers une adresse en dehors de l'allocateur.
	 */
	while (current < get_system_memory_addr() + sizeof(struct allocator_header) + get_system_memory_size()) {
		/* Entier (booléen) valant 1 si le bloc actuel (current) est un bloc libre
		 * (s'il est à la même adresse que le bloc libre actuel : free_block).
		 * Vaut 0 sinon.
		 */
		int is_free = current == (void*)free_block ? 1 : 0;
		// Variable contenant la taille du boc actuel
		size_t size = *(size_t*)current;
		/* Cette instruction permet d'afficher une représentation textuelle du bloc actuelle, indiquant son adresse
		 * en mémoire, sa taille et s'il est libre ou non.
		 * Pour ce faire, on utilise le pointeur de la fonction passée en paramètre de mem_show().
		 */
		print(current, size, is_free);
		
		// Condition permettant d'atteindre le bloc suivant, avant d'effectuer un tour de boucle.
		
		/* Si le bloc actuel est libre, on renseigne dans free_block le prochain bloc libre,
		 * et on fait pointer current vers le prochain bloc, en lui ajoutant la taille
		 * du bloc actuel ainsi que la taille des métadonnées d'une structure de bloc libre.
		 */
		if (is_free == 1) {
			free_block = free_block->next;
			current += sizeof(struct fb) + size;
		/* Sinon, on fait pointer current vers le prochain bloc, en lui ajoutant la
		 * taille du bloc actuel ainsi que la taille des métadonnées d'un bloc occupé.
		 */
		} else
			current += sizeof(size_t) + size;
	}
}

// Fonction permettant de redéfinir la stratégie d'allocation par celle passée en paramètre (pointeur vers une fonction).
void mem_fit(mem_fit_function_t *f) {
	get_header()->fit = f;
}

/* PROBLEME CONCERNANT LA TAILLE DES ZONES LIBRES :
 * Une zone libre possède une taille, mais ne faudrait-il pas mentir sur cette taille ?
 * En effet, une zone libre possède des métadonnées plus importantes qu'une zone occupée
 * Donc, lorsque l'on transforme une zone libre en occupée, on réduira la taille des métadonnées de cette dernière
 * Ce qui veut dire que la nouvelle zone occupée pourra contenir un peu plus de données que la taille annoncée par la zone libre
 * Ce surplus de mémoire vaut exactement : memory_gap = sizeof(struct fb) - sizeof(size_t)
 *
 * Donc une solution pour gagner un peu de place (et surtout ne pas gaspiller memory_gap) serait d'ajouter ce memory_gap à la taille initiale de la zone libre
 * Mais cela complexifiera la tâche consistant à trouver la zone située après une zone libre, puisqu'il faudra retrancher memory_gap à chaque fois
 *
 * Ou alors on peut décider d'accepter à ce que l'utilisateur demande l'allocation d'une zone légèrement plus grande que la taille de la zone libre,
 * tant que ce dépassement n'est pas supérieur à memory_gap
 */

void *mem_alloc(size_t taille) {
	/* INSTRUCTIONS :
	 * L'appel de fit(get_header()->list, taille) (ligne 105) va retourner une zone libre selon la stratégie utilisée, que l'on stockera dans *fb
	 * On redéfinira fb->size = taille puis on créera une zone libre *after à l'adresse sizeof(struct fb) + taille, si 
	 * Cependant, il faut avoir la zone libre se trouvant avant *fb, notée *before, car il faudra redéfinir before->next = fb->next
	 * Ensuite, il faudra modifier le size_t taille de *fb et créer une zone libre *after juste après,
	 * s'il reste assez de place pour stocker les métadonnées de cette nouvelle zone, et un minimum de place
	 * Finalement, on retournera le pointeur vers la zone mémoire de l'utilisateur, c'est à dire (void*)(fb + sizeof(size_t))
	 */
	
	/* ... */
	__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */
	struct fb *fb=get_header()->fit(/*...*/NULL, /*...*/0);
	/* ... */
	return NULL;
}


/* Fonction permettant de libérer une zone allouée par l'utilisateur.
 * Le paramètre mem est le pointeur retourné à l'utilisateur lors du mem_alloc().
 * Ce dernier pointe vers l'adresse correspondant au début de la zone mémoire demandée préalablement par l'utilisateur.
 */
void mem_free(void* mem) {
	/* On initialise des pointeurs de structures de bloc libre qui correspondront au bloc que l'utilisateur demande de libérer,
	 * puis au blocs libres précédant et suivant ce dernier, afin de savoir s'il faut réassembler les-dits blocs.
	 */
	struct fb *before = NULL,
		  *current = (struct fb*)(mem - sizeof(size_t)),	  
		  *after = get_header()->list;
	
	// Boucle permettant de correctement définir les pointeurs vers les blocs libres précédant et suivant le bloc à libérer.
	while (after != NULL && after < current) {
		before = after;
		after = after->next;
	}
	
	/* Crée une structure de bloc libre à l'emplacement donné par l'utilisateur, en redéfinissant sa taille
	 * en fonction de l'écart de taille entre les métadonnées des blocs libres et celles des blocs occupés.
	 */
	*current = (struct fb) {
		mem_get_size(mem) - get_metadata_size_gap(),
		after
	};
	
	/* Variables décrivant si les blocs précédant et suivant le bloc actuel sont libres ou pas.
	 * Attention : les pointeurs before et after ne correspondent pas forcément à blocs cités ci-dessus,
	 * mais aux blocs LIBRES les plus proches du bloc actuel, respectivement avant et après ce dernier.
	 */
	int before_is_free = before != NULL && current == before + sizeof(struct fb) + before->size ? 1 : 0,
	    after_is_free = after != NULL && after == current + sizeof(struct fb) + current->size ? 1 : 0;
	
	// Si le bloc se situant juste après le bloc actuel est libre, on le fusionne avec le bloc actuel.
	if (after_is_free) {
		current->size += sizeof(struct fb) + after->size;
		current->next = after->next;
	}
	
	// Si le bloc se situant juste avant le bloc actuel est libre, on le fusionne avec le bloc actuel.
	if (before_is_free) {
		before->size += sizeof(struct fb) + current->size;
		before->next = current->next;
	/* Sinon, on doit quand même redéfinir le bloc libre suivant le précédent
	 * bloc libre par notre bloc actuel, pour garantir le chaînage des blocs.
	 */
	} else
		before->next = current; // Sans cette ligne, before->next pointait sur after.
}


// Fonction retournant le premier bloc libre de taille au moins égale à size, en utilisant donc la stratégie mem_fit_first.
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
// Cette fonction retourne la taille d'une zone occupée, en prenant le pointeur vers la zone mémoire de cette zone (et non vers ses métadonnées).
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return *(size_t*) (zone - sizeof(size_t));
}


/* Fonctions facultatives
 * autres stratégies d'allocation
 */

// Fonction retournant le bloc libre dont la taille est la plus proche de size (et satisfaisant size >= taille), en utilisant donc la stratégie mem_fit_best.
struct fb* mem_fit_best(struct fb *list, size_t size) {
	return NULL;
}

// Fonction retournant le bloc libre dont la taille est la plus grande (et satisfaisant size >= taille), en utilisant donc la stratégie mem_fit_worst.
struct fb* mem_fit_worst(struct fb *list, size_t size) {
	return NULL;
}
