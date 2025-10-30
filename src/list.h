#ifndef LIST_H
#define LIST_H

#include <stddef.h>

namespace mylist {

typedef int list_containing_t;

int list_contairing_t_comparator(int first, int second);

// Detailed error codes used by the list implementation. Prefer using the most
// specific code when setting `list->errno` so caller can diagnose failures.
// Notes:
//  - Many routines expect `list->size` to include the sentinel/zombie element
//    at index 0 (so an empty list has size == 1).
//  - Index 0 is a valid sentinel. Cycles that reach index 0 are considered
//    part of the valid circular sentinel scheme; cycles that revisit a
//    non-zero node are reported as corruption errors.
enum LIST_ERRNO {
    LIST_NO_PROBLEM = 0,
    LIST_POISON_COLLISION,         // attempted to store a POISON value

    // Memory errors
    LIST_CANNOT_ALLOC_MEMORY,      // calloc/malloc failed
    LIST_CANNOT_REALLOC_MEMORY,    // realloc/copy failed

    // Logical errors
    LIST_OVERFLOW,                 // no free element available
    LIST_INTERNAL_STRUCT_DAMAGED,  // generic internal corruption (fallback)

    LIST_NULL_POINTER,             // a required pointer (e.g. elements) is NULL
    LIST_INVALID_INDEX,            // encountered an index >= capacity
    LIST_FREE_LIST_CORRUPT,        // free-list chain is corrupted/invalid
    LIST_SIZE_MISMATCH,            // list->size doesn't match actual elements
    LIST_CORRUPTED_NEXT,           // detected inconsistency in `.next` chain
    LIST_CORRUPTED_PREV,           // detected inconsistency in `.prev` pointers

};

typedef struct {
    list_containing_t   data;
    size_t              next, prev;
} list_element_t;

typedef struct {
    LIST_ERRNO          errno;

    list_element_t      *elements;

    size_t              size, capacity,
                        free_idx;
} list_t;

// constructs the list
list_t * constructor(size_t capacity);

// destructs the list
void destructor(list_t **list_ptr);

const char *error(list_t *list);

// verify state of list
// return true if in valid state, false if in incorrect state
bool verifier(list_t *list);

void _generate_dot_dump(list_t *list, FILE * fp);
const char *_generate_image(list_t *list, const char *dir_name);
void _dump_impl(list_t *list, FILE *logfile, const char *log_dirname, const char *prompt,
    int line, const char *func, const char *file);

// dump list to log
#define dump(list, logfile, log_dirname, prompt) \
    _dump_impl((list), (logfile), (log_dirname), (prompt), __LINE__, __FUNCTION__, __FILE__)

// ----- capacity ----

bool empty(list_t *list);

size_t size(list_t *list);

size_t capacity(list_t *list);

// ----- element access -----

// get index of first element
size_t front(list_t *list);

// get index of last element
size_t back(list_t *list);

// get index of next element
size_t get_next_element(list_t *list, size_t element_idx);

// get index of prev element
size_t get_prev_element(list_t *list, size_t element_idx);

namespace slow {
    // get index by logic index
    size_t index(list_t *list, size_t logic_index);

    // search index of the first occurrence of a value (0 if not found)
    size_t search(list_t *list, list_containing_t value);
}

// ----- modifiers -----

namespace slow {
    // change capacity of list
    LIST_ERRNO resize(list_t *list, size_t new_capacity);
}

// insert new value after element_idx
size_t insert(list_t *list, size_t element_idx, list_containing_t value);

// insert new value before element_idx
size_t emplace(list_t *list, size_t element_idx, list_containing_t value);

// insert new value at first list position
size_t push_front(list_t *list, list_containing_t value);

// insert new value at last list position
size_t push_back(list_t *list, list_containing_t value);

// delete value at element_idx
void erase(list_t *list, size_t element_idx);

// pop value at first list position
void pop_front(list_t *list);

// pop value at last list position
void pop_back(list_t *list);

// swap two value
void swap(list_t *list, size_t el_idx_1, size_t el_idx_2);

// ----- operations -----

// void slow::reverse(list_t *list);

}

#endif // LIST_H