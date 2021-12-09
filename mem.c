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
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void *memory_addr;

static inline void *get_system_memory_addr() { return memory_addr; }

static inline struct allocator_header *get_header() {
    struct allocator_header *h;
    h = get_system_memory_addr();
    return h;
}

static inline size_t get_system_memory_size() {
    return get_header()->memory_size;
}

struct fb {
    /// La taille de l'espace libre en octets
    /// La taille du free block lui-même n'est pas comprise
    size_t size;
    struct fb *next;
};

/// Renvoie la tête de la liste des free blocks
struct fb *get_list() {
    return memory_addr + sizeof(struct allocator_header);
}

/// Renvoie la taille totale (alloué + pas alloué) de ce bloc
size_t get_total_size(struct fb *block) {
    if (block->next != NULL) {
        return block->next - block + sizeof(struct fb);
    } else {
        return (get_system_memory_addr() + get_system_memory_size()) - block + sizeof(struct fb);
    }
}

void mem_init(void *mem, size_t taille) {
    memory_addr = mem;
    *(size_t *)memory_addr = taille;
    /* On vérifie qu'on a bien enregistré les infos et qu'on
     * sera capable de les récupérer par la suite
     */
    assert(mem == get_system_memory_addr());
    assert(taille == get_system_memory_size());
    mem_fit(&mem_fit_first);

    // On initialise le premier free block
    struct fb *head = get_list();
    head->size = taille - sizeof(struct allocator_header) - sizeof(struct fb);
    head->next = NULL;
}

void mem_show(void (*print)(void *, size_t, int)) {
    struct fb *block = get_list();
    while (block != NULL) {
        print(block, block->size, block->size == get_total_size(block));
        block = block->next;
    }
}

void mem_fit(mem_fit_function_t *f) { get_header()->fit = f; }

void *mem_alloc(size_t taille) {
    struct fb *b = get_header()->fit(get_list(), taille);
    if (b == NULL) {
        return NULL;
    }

    // TODO: align the new fb
    struct fb *new_fb = b + sizeof(struct fb) + taille;
    new_fb->size = b->size - taille - sizeof(struct fb);
    new_fb->next = NULL;
    b->next = new_fb;

    return new_fb + sizeof(struct fb);
}

void mem_free(void *mem) {

    struct fb *firstBlock = get_list();
    size_t adr = mem - sizeof(size_t);
    struct fb *nextBlock;

    if(firstBlock == adr)
    {
        firstblock->taille = get_total_size(firstblock);
        return;
    }

    while ((firstBlock->next != adr) && (firstBlock->next != NULL)){
        firstBlock = firstBlock->next;
    }

    nextBlock = firstBlock->next;
    struct fb *nextBlock = firstBlock->next;
    firstBlock->taille = firstBlock->taille + get_total_size(nextblock) + sizeof(struct fb);
    firstBlock->next = nextblock->next;
}

struct fb *mem_fit_first(struct fb *list, size_t size) {
    while (list != NULL) {
        if (list->size >= size) {
            return list;
        }
        list = list->next;
    }
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
    struct fb *block = zone - sizeof(struct fb);
    return get_total_size(block);
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb *mem_fit_best(struct fb *list, size_t size) {
    struct fb *best = NULL;
    while (list != NULL) {
        if (list->size >= size) {
            if (best == NULL || best->size > list->size) {
                best = list;
            }
        }
        list = list->next;
    }
    return best;
}

struct fb *mem_fit_worst(struct fb *list, size_t size) {
    struct fb *worst = NULL;
    while (list != NULL) {
        if (list->size >= size) {
            if (worst == NULL || worst->size < list->size) {
                worst = list;
            }
        }
        list = list->next;
    }
    return worst;
}
