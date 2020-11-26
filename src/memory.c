#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>



struct fb;


int get_boolean(size_t bool_number);
size_t get_value(size_t bool_number);

void set_boolean(size_t *bool_number, int boolean);
void set_value(size_t *bool_number, size_t value);

void set_bool_number(size_t *bool_number, int boolean, size_t value);



size_t get_total_size();
size_t get_total_blocks();

void set_total_blocks(size_t value);
void set_total_size(size_t value);


int is_block_free(fb_t *block);
size_t get_block_size(fb_t *block);

void set_block_state(fb_t *block, int state);
void set_block_size(fb_t *block, size_t size);

void set_block_metadata(fb_t *block, int state, size_t size);

size_t get_total_block_size(fb_t *block);

void* get_block_memory(fb_t *block);

fb_t* get_first_block();
fb_t* get_block_at(int index);

fb_t get_next_block(fb_t *block);
fb_t* get_next_free_block(size_t min_size);

fb_t* create_block_at(int index, size_t size);
void reassemble_blocks(fb_t *block1, fb_t *block2);


/* fonctions principales de l'allocateur */
void mem_destroy();
void mem_destroy_metadata();

void mem_init(size_t size);
void* mem_alloc(size_t size);
void mem_free(void *ptr);
void* mem_realloc(void *old, size_t new_size);

void mem_show();

/* Itération sur le contenu de l'allocateur */
/* nécessaire pour le mem_shell */
void mem_show_(void (*print)(void *adr, size_t size, int free));



typedef struct fb {
	size_t metadata;
} fb_t;





void *allocated = NULL;





int main() {
	mem_init(2048);

	void *ptr1 = mem_alloc(32);
	void *ptr2 = mem_alloc(32);
	void *ptr3 = mem_alloc(32);

	mem_free(ptr2);

	void *ptr4 = mem_alloc(16);
	void *ptr5 = mem_alloc(16);

	mem_show();

	mem_destroy();
	return 0;
}





int get_boolean(size_t bool_number) {
	return bool_number >> 63;
}

size_t get_value(size_t bool_number) {
	return bool_number & (size_t) 0x7FFFFFFFFFFFFFFF;
}



void set_boolean(size_t *bool_number, int boolean) {
	if (boolean == 0)
		*bool_number &= (size_t) 0x7FFFFFFFFFFFFFFF;
	else if (boolean == 1)
		*bool_number |= (size_t) 0x8000000000000000;
}

void set_value(size_t *bool_number, size_t value) {
	if (value < (size_t) 0)
		return;
	if (get_boolean(value) == 1)
		set_boolean(bool_number, 0);
	*bool_number = *bool_number & (size_t) 0x8000000000000000 | value;
}



void set_bool_number(size_t *bool_number, int boolean, size_t value) {
	set_value(bool_number, value);
	set_boolean(bool_number, boolean);
}





size_t get_total_size() {
	return (size_t) *allocated;
}


size_t get_total_blocks() {
	return (size_t) *(allocated + sizeof(size_t));
}



void set_total_blocks(size_t value) {
	if (value >= (size_t) 0)
		*allocated = value;
}


void set_total_size(size_t value) {
	if (value >= (size_t) 1)
		*(allocated + sizeof(size_t)) = value;
}





int is_block_free(fb_t *block) {
	if (block == NULL)
		return -1;

	return get_boolean(block -> metadata);
}


size_t get_block_size(fb_t *block) {
	if (block == NULL)
		return (size_t) 0;

	return get_value(block -> metadata);
}



void set_block_state(fb_t *block, int state) {
	if (block == NULL || (state != 0 && state != 1))
		return;

	set_boolean(&(block -> metadata), state);
}


void set_block_size(fb_t *block, size_t size) {
	if (block == NULL || size < 1)
		return;

	set_value(&(block -> metadata), size);
}



void set_block_metadata(fb_t *block, int state, size_t size) {
	if (block == NULL || (state != 0 && state != 1) || size < 1)
		return;

	set_bool_number(&(block -> metadata), state, size);
}



size_t get_total_block_size(fb_t *block) {
	if (block == NULL)
		return (size_t) 0;

	return sizeof(fb_t) + get_block_size(block);
}



void* get_block_memory(fb_t *block) {
	if (block == NULL)
		return NULL;

	return block + sizeof(fb_t);
}





fb_t* get_first_block() {
	return (fb_t*) allocated + (size_t) 2 * sizeof(size_t);
}


fb_t* get_block_at(int index) {
	if (index < 0)
		return NULL;

	fb_t *current = get_first_block();

	for (int i = 0; i < index; i++) {
		if (current == NULL)
			break; // return NULL;

		current = get_next_block(current);
	}

	return current;
}


fb_t* get_next_block(fb_t *block) {
	if (block == NULL)
		return NULL;

	fb_t *next = block + get_total_block_size(block);

	if (next >= allocated + get_total_size())
		return NULL;

	return next;
}


fb_t* get_next_free_block(size_t min_size) {
	if (min_size < (size_t) 1)
		return NULL;

	fb_t *current = get_first_block();

	while (current != NULL) {
		if (is_block_free(current) == 1 && get_block_size(current) >= min_size)
			return current;
		get_next_block(current);
	}

	return NULL;
}


fb_t* create_block_at(int index, size_t size) {
	if (index < 0 || size < (size_t) 1)
		return NULL;

	fb_t *before = NULL;
	if (index > 0) {
		*before = get_block_at(index - 1);
		if (*before == NULL)
			return NULL;
	}

	fb_t *after = before == NULL ? get_first_block() : get_next_block(before);

	// Allows only if after is free and if its new size >= 1
	if (is_block_free(after) == 0 || get_block_size(after) <= sizeof(fb_t) + size)
		return NULL;

	fb_t *current = after;
	after = after + sizeof(fb_t) + size;
	printf("Current address : %p\n", current);
	printf("After address   : %p\n", after);

	// Décaler after à droite dans le block libre
	*after = *current;

	set_block_metadata(current, 0, size);
	//current -> next = after;

	set_block_size(after, get_block_size(after) - get_total_block_size(current))

	//if (before != NULL)
	//	before -> next = current;

	set_total_blocks(get_total_blocks() + 1);

	return current;
}


void reassemble_blocks(fb_t *block1, fb_t *block2) {
	if (block1 == NULL || block2 == NULL || is_block_free(block1) == 0 || is_block_free(block2) == 0 || block1 -> next != block2 -> next)
		return NULL;

	set_block_size(block1, get_block_size(block1) + get_total_block_size(block2));
	set_total_blocks(get_total_blocks() - 1);
	//block1 -> next = block2 -> next;
}





// Destroys all the memory used by the program, including allocated memory and blocks metadata
void mem_destroy() {
	if (allocated != NULL) {
		free(allocated);
		allocated = NULL;
		printf("\n##### DESTROYED #####\n\n");
	}
}


// Initializes the memory : Allocates memory and create one free block using all this memory
void mem_init(size_t size) {
	if (size <= 0) {
		printf("Cannot initialize memory with negative or null size.\n");
		return;
	}

	mem_destroy();

	allocated = malloc(size);
	set_total_size(size);
	set_total_blocks((size_t) 1);
	fb_t *first_block = allocated + (size_t) 2 * sizeof(size_t);
	*first_block = (fb_t) {(size_t) 0};
	set_block_metadata(first_block, 1, size - (sizeof(fb_t)));

	printf("\n##### INITIALIZED #####\n\n");
}


// Create an occuped block in the allocated memory and return a pointer at its location in memory
void* mem_alloc(size_t size) {
	if (size <= 0) {
		printf("Cannot allocate block of negative or null size.\n");
		return NULL;
	}

	fb_t *current = get_next_free_block(size);

	if (current == NULL) {
		printf("Cannot allocate block of size %ld.\n", size);
		return NULL;
	}

	if (current -> size > size) {
		fb_t *next = malloc(sizeof(fb_t));
		*next = (fb_t) {current -> next, current -> size - size, 1};
		current -> next = next;
		current -> size = size;
		total_blocks++;
	}

	current -> free = 0;
	return allocated + addr;
}


// 
void mem_free(void *ptr) {
	if (ptr == NULL || ptr < allocated || ptr >= allocated + total_size) {
		printf("Invalid pointer given.\n");
		return;
	}

	/* Traiter 4 cas : 
	 *   - Si la zone à libérer est entourée de zones occupées : rendre la zone libre
	 *   - Si la zone à libérer est entourée de zones libres : relier les 3 zones
	 *   - Si la zone à libérer est après une zone libre et avant une zone occupée : relier la zone à celle d'avant
	 *   - Si la zone à libérer est après une zone occupée et avant une zone libre : relier la zone à celle d'après
	 */


	// Finds block and blocks before and after it
	fb_t *before = NULL,
		 *current = first_block;
	size_t addr = 0;

	while (current != NULL && allocated + addr != ptr) {
		addr += current -> size;
		before = current;
		current = current -> next;
	}

	if (current == NULL) {
		printf("Couldn't find block at address %p\n", ptr);
		return;
	}

	if (current -> free == 1) {
		printf("Block at address %p is already free.\n", ptr);
		return;
	}
	

	// Frees block and reassembles blocks
	// Don't forget to think about cases where blocks before / after are NULL

	fb_t *after = current -> next;
	current -> free = 1;

	if (after != NULL && after -> free == 1) {
		current -> size += after -> size;
		current -> next = after -> next;
		free(after);
		total_blocks--;
	}

	if (before != NULL && before -> free == 1) {
		before -> size += current -> size;
		before -> next = current -> next;
		free(current);
		total_blocks--;
	}
}


void* mem_realloc(void *old, size_t new_size) {
	// May be incorrect (erases old data by forgetting it)
	mem_free(old);
	return mem_alloc(new_size);
}


// Show all the blocks created in the allocated memory, with information on their size, address in memory and status
void mem_show() {
	fb_t *current = first_block;

	printf("===== MEMORY START =====\n");

	int i = 0;
	size_t addr = 0;
	while (current != NULL) {
		printf("\nBlock %d/%d :\n", i+1, total_blocks);
		printf("    Size    : %ld\n", current -> size);
		printf("    Address : %p\n", allocated + addr);
		printf("    State   : %s\n", (current -> free == 1 ? "free" : "used"));

		addr += current -> size;
		current = current -> next;
		i++;
	}

	printf("\n===== MEMORY END =====\n");
}



/* Itération sur le contenu de l'allocateur */
/* nécessaire pour le mem_shell */
void mem_show_(void (*print)(void *adr, size_t size, int free)) {

}
