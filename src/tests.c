#include "mem.h"

#include <stdio.h>
#include <stdlib.h>



#define MEMORY_SIZE 2048L

// Constantes utilisées à des fins d'affichage (car on n'a pas accès aux structures de mem.c)
#define SIZE_OF_STRUCT_ALLOCATOR_HEADER 24L
#define SIZE_OF_STRUCT_FB 16L



// Fonction permettant d'afficher la description d'un bloc, en spécifiant sa taille, son état et son adresse
void print(void *ptr, size_t size, int is_free) {
    printf(
    	"Zone mémoire %s de taille %ld (taille des métadonnées : %ld, taille %s : %ld) et d'adresse %p\n",
    	is_free == 1 ? "libre" : "occupée",
    	size,
    	is_free == 1 ? SIZE_OF_STRUCT_FB : sizeof(size_t),
    	is_free == 1 ? "non-occupée" : "utilisable",
    	size - (is_free == 1 ? SIZE_OF_STRUCT_FB : sizeof(size_t)),
    	ptr
    );
}



// Allocation de quatre zones de tailles différentes
void test_00() {
	printf("\nTest 00 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    mem_alloc(256);
    mem_alloc(512);
    mem_alloc(128);
    mem_alloc(1024);

    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 00 terminé.\n\n");
}


// Allocation de quatre zones de tailles différentes et libération d'une de ces zones
void test_01() {
	printf("\nTest 01 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    mem_alloc(256);
    void *ptr = mem_alloc(512);
    mem_alloc(128);
    mem_alloc(1024);
    mem_free(ptr);

    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 01 terminé.\n\n");
}


// Allocation de quatre zones de tailles différentes et libération de deux de ces zones afin qu'elles se ré-assemblent
void test_02() {
	printf("\nTest 02 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    mem_alloc(256);
    void *ptr1 = mem_alloc(512);
    void *ptr2 = mem_alloc(128);
    mem_alloc(1024);

    mem_free(ptr1);
    mem_free(ptr2);
    
    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 02 terminé.\n\n");
}


// Allocation de quatre zones de tailles différentes et libération des deux premières zones afin qu'elles se ré-assemblent
void test_03() {
	printf("\nTest 03 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    void *ptr1 = mem_alloc(256);
    void *ptr2 = mem_alloc(512);
    mem_alloc(128);
    mem_alloc(1024);

    mem_free(ptr1);
    mem_free(ptr2);
    
    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 03 terminé.\n\n");
}


// Allocation de quatre zones de tailles différentes et libération des deux dernières zones afin qu'elles se ré-assemblent
void test_04() {
	printf("\nTest 04 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    mem_alloc(256);
    mem_alloc(512);
    void *ptr1 = mem_alloc(128);
    void *ptr2 = mem_alloc(1024);

    mem_free(ptr1);
    mem_free(ptr2);
    
    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 04 terminé.\n\n");
}


// Allocation de quatre zones de tailles différentes et libération de ces quatre zones
void test_05() {
	printf("\nTest 05 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    void *ptr1 = mem_alloc(256);
    void *ptr2 = mem_alloc(512);
    void *ptr3 = mem_alloc(128);
    void *ptr4 = mem_alloc(1024);

    mem_free(ptr1);
    mem_free(ptr2);
    mem_free(ptr3);
    mem_free(ptr4);

    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 05 terminé.\n\n");
}


// Allocation de quatre zones de tailles différentes et libération de ces quatre zones dans un ordre différent
void test_06() {
	printf("\nTest 06 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    void *ptr1 = mem_alloc(256);
    void *ptr2 = mem_alloc(512);
    void *ptr3 = mem_alloc(128);
    void *ptr4 = mem_alloc(1024);

    mem_free(ptr3);
    mem_free(ptr1);
    mem_free(ptr2);
    mem_free(ptr4);

    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 06 terminé.\n\n");
}


// Allocation de deux zones, puis libération de la première et allocation d'une zone plus petite (qui doit se trouver dans la zone libérée)
void test_07() {
	printf("\nTest 07 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    void *ptr = mem_alloc(1024);
    mem_alloc(256);

    mem_free(ptr);

    mem_alloc(512);

    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 07 terminé.\n\n");
}


// Allocation de deux zones, puis libération de la première et allocation d'une zone remplissant l'espace de la zone libérée
void test_08() {
	printf("\nTest 08 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    void *ptr = mem_alloc(1024);
    mem_alloc(256);

    mem_free(ptr);

    mem_alloc(1024);

    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 08 terminé.\n\n");
}


// Allocation d'une zone remplissant tout l'espace mémoire disponible (en renseignant une taille supérieure à celle disponible)
void test_09() {
	printf("\nTest 09 :\n\n");

	void *mem = malloc(MEMORY_SIZE);
    mem_init(mem, MEMORY_SIZE);
    printf("Mémoire initialisée : taille %ld\n", (size_t) MEMORY_SIZE);

    mem_alloc(2048);

    mem_show(&print);

    free(mem);
    printf("\nMémoire libérée. Test 09 terminé.\n\n");
}



int main() {
	printf("Taille de la structure allocator_header : %ld\n", SIZE_OF_STRUCT_ALLOCATOR_HEADER);
	printf("Taille de la structure fb (bloc libre)  : %ld\n", SIZE_OF_STRUCT_FB);
	printf("Taille de la structure de bloc occupé   : %ld\n\n", sizeof(size_t));

    test_00();
    test_01();
    test_02();
    test_03();
    test_04();
    test_05();
    test_06();
    test_07();
    test_08();
    test_09();

    return 0;
}
