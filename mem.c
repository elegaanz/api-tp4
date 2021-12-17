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
  size_t size;
  struct fb *next;
};

size_t unused_block_size(struct fb *block) {
    if (block->next == NULL) {
        return get_system_memory_addr() + get_system_memory_size()
            - (void*)block - sizeof(struct fb);
    } else {
        return (void*)block->next - (void*)block - sizeof(struct fb);
    }
}

/// Renvoie la tête de la liste des free blocks
struct fb *get_list() {
  return memory_addr + sizeof(struct allocator_header);
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
  head->size = 0;
  head->next = NULL;
}

void mem_show(void (*print)(void *, size_t, int)) {
  struct fb *block = get_list();
  while (block != NULL) {
      int s = block->size;
      if (s == 0) {
          s = unused_block_size(block);
      }
    print(block, s, block->size == 0);
    block = block->next;
  }
}

void mem_fit(mem_fit_function_t *f) { get_header()->fit = f; }

void *mem_alloc(size_t taille) {
  // Insère une nouvelle zone dans une zone où il y assez de place pour ça
  struct fb *b = get_header()->fit(get_list(), sizeof(struct fb) + taille); // on trouve une zone qui a encore assez d'espace libre
  if (b == NULL) {
    return NULL;
  }

  b->size = taille;
  if (b->next == NULL) {
    b->next = (struct fb*)((void*)b + taille + sizeof(struct fb));
    b->next->size = 0;
    b->next->next = NULL;
  } else {
    // on est dans cette configuration
    // |                     b           |   b->next  |
    // | header | espace |   espace pas  | header | … |
    // |        | alloué | encore occupé |        |   |
    //
    // on va essayer de ne pas perdre l'espace pas encore occupé
    // pour ça, si on a la place, on ajoute une nouvelle zone libre
    // entre b et b->next
    //
    // si on a pas assez de place, tant pis, on perdra un peu de mémoire

    // TODO: align the new fb
    struct fb* new_next = (void*)b + taille;
    // on a la place pour ajouter
    if (new_next <= b->next - 1) {
        new_next->size = 0; // bloc libre
        new_next->next = b->next;
        b->next = new_next;
    }
  }

  return (void*)b + sizeof(struct fb);
}

void mem_free(void *mem) {
  struct fb *firstBlock = get_list();
  size_t adr = (size_t)mem - sizeof(size_t);
  struct fb *nextBlock;

  if (firstBlock == (struct fb *)adr) {
    // firstBlock->size = get_total_size(firstBlock);
    return;
  }

  while ((firstBlock->next != (struct fb *)adr) && (firstBlock->next != NULL)) {
    firstBlock = firstBlock->next;
  }

  nextBlock = firstBlock->next;
  if (nextBlock != NULL) {
    // firstBlock->size = get_total_size(firstBlock) + sizeof(struct fb);
    firstBlock->next = nextBlock->next;
  } else {
    // firstBlock->size = get_total_size(firstBlock);
  }
}

struct fb *mem_fit_first(struct fb *list, size_t size) {
  while (list != NULL) {
    if (list->size == 0 && unused_block_size(list) >= size) {
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
  return block->size;
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
