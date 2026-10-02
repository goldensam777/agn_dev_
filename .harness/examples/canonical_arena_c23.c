/**
 * @file canonical_arena_c23.c
 * @brief Modèle canonique C23 : allocateur d'arène — propriété unique, libération en bloc.
 *
 * Règles illustrées (corpus `.harness/knowledge/languages/c/`) :
 *  - core/memory-arenas.md   : arène par-dessus malloc ; O(1) par allocation ;
 *    libération globale en fin de passe (AST, tables temporaires).
 *  - core/pointers-ub.md     : arithmétique de pointeurs bornée, alignement explicite,
 *    jamais d'accès hors bloc (bornes vérifiées AVANT écriture).
 *  - core/interfaces-c23.md  : nullptr natif, [[nodiscard]], alignof/bool (C23),
 *    fonction de libération documentée (CONVENTIONS.md).
 *
 * Compilation : clang -std=c23 -Wall -Wextra -Wpedantic -Wconversion -Werror \
 *               -fsanitize=address,undefined canonical_arena_c23.c -o /tmp/arena_c
 */

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ArenaBlock {
    struct ArenaBlock* next;
    size_t used;
    size_t capacity;
} ArenaBlock;

typedef struct {
    ArenaBlock* head; /* liste LIFO — seul propriétaire des blocs */
    size_t block_size;
} Arena;

/* En-tête sur-aligné : sizeof(ArenaBlock) arrondi à alignof(max_align_t), puis
 * + alignof(max_align_t). Le payload commence donc TOUJOURS sur une frontière
 * max_align_t : tout align <= alignof(max_align_t) est servi par construction. */
static size_t arena_header_size(void) {
    const size_t align = alignof(max_align_t);
    const size_t rounded = (sizeof(ArenaBlock) + (align - 1u)) & ~(align - 1u);
    return rounded + align;
}

[[nodiscard]] static bool arena_init(Arena* arena, size_t block_size) {
    if (arena == nullptr || block_size == 0u) {
        return false;
    }
    /* Le bloc initial est chaîné dès l'init : arena_alloc n'a jamais à gérer
     * l'absence de bloc, et son seul NULL possible = échec malloc/overflow. */
    ArenaBlock* first = malloc(arena_header_size() + block_size);
    if (first == nullptr) {
        return false;
    }
    first->next = nullptr;
    first->used = 0u;
    first->capacity = block_size;
    arena->head = first;
    arena->block_size = block_size;
    return true;
}

/// Libère TOUS les blocs — fonction de libération unique et documentée.
/// Aucune libération individuelle n'existe : par design.
static void arena_deinit(Arena* arena) {
    if (arena == nullptr) {
        return;
    }
    ArenaBlock* block = arena->head;
    while (block != nullptr) {
        ArenaBlock* next = block->next;
        free(block);
        block = next;
    }
    arena->head = nullptr;
}

static void* arena_alloc(Arena* arena, size_t size, size_t align) {
    assert(arena != nullptr);
    if (align == 0u || align > alignof(max_align_t)) {
        return nullptr; /* sur-alignement : refus explicite plutôt qu'UB silencieux */
    }

    ArenaBlock* block = arena->head;
    if (block == nullptr) {
        return nullptr;
    }

    const size_t header = arena_header_size();
    const uintptr_t base = (uintptr_t)block + header;
    const uintptr_t cursor = base + block->used;
    const size_t padding = (size_t)((align - (cursor % align)) % align);

    if (block->used + padding + size < block->used || /* overflow de size_t */
        block->used + padding + size > block->capacity) {
        /* Bloc plein : chaîner un bloc neuf (plus grand si nécessaire). */
        const size_t payload = (size > arena->block_size) ? size : arena->block_size;
        ArenaBlock* fresh = malloc(header + payload);
        if (fresh == nullptr) {
            return nullptr;
        }
        fresh->next = arena->head;
        fresh->used = 0u;
        fresh->capacity = payload;
        arena->head = fresh;
        /* Bloc neuf : base alignée sur max_align_t -> padding nul. */
        return arena_alloc(arena, size, align);
    }

    block->used += padding + size;
    return (void*)(cursor + padding);
}

#define ARENA_ALLOC(arena_ptr, T) ((T*)arena_alloc((arena_ptr), sizeof(T), alignof(T)))

/* --- Démonstration ------------------------------------------------------- */

typedef struct {
    double x;
    double y;
} Vec2;

int main(void) {
    Arena arena;
    assert(arena_init(&arena, 256u));

    Vec2* a = ARENA_ALLOC(&arena, Vec2);
    Vec2* b = ARENA_ALLOC(&arena, Vec2);
    assert(a != nullptr && b != nullptr && a != b);

    a->x = 3.0;
    a->y = 4.0;
    b->x = 1.0;
    b->y = 2.0;
    /* ASan valide ici : aucune fuite, aucun débordement, aucun UB. */
    assert(a->x == 3.0 && b->y == 2.0);

    /* Tout est libéré d'un coup, une seule fois. Double deinit = no-op sûr. */
    arena_deinit(&arena);
    arena_deinit(&arena);

    puts("Modèle d'arène C23 validé sous ASan/UBSan.");
    return 0;
}
