#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>

#include "ansi.h"
#include "list.h"
#include "base.h"

namespace mylist_aos {

#define eval_print(code) \
    printf("%s = %lld\n", #code, (code))

#define verified(code) \
    || ({code; false;})

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconstant-conversion"
const int POISON = 0xDEDB333C0CA1;
#pragma clang diagnostic push

int list_contairing_t_comparator(int first, int second) {
    return first - second;
}

// typedef int list_containing_t;

// enum LIST_ERRNO {
//     LIST_NO_PROBLEM = 0,
//     LIST_POISON_COLLISION,         // attempted to store a POISON value
//
//     // Memory errors
//     LIST_CANNOT_ALLOC_MEMORY,      // calloc/malloc failed
//     LIST_CANNOT_REALLOC_MEMORY,    // realloc/copy failed
//
//     // Logical errors
//     LIST_OVERFLOW,                 // no free element available
//     LIST_INTERNAL_STRUCT_DAMAGED,  // generic internal corruption (fallback)
//
//     LIST_NULL_POINTER,             // a required pointer (e.g. elements) is NULL
//     LIST_INVALID_INDEX,            // encountered an index >= capacity
//     LIST_FREE_LIST_CORRUPT,        // free-list chain is corrupted/invalid
//     LIST_SIZE_MISMATCH,            // list->size doesn't match actual elements
//     LIST_LOOP_IN_NEXT,             // detected inconsistency in `.next` chain
//     LIST_LOOP_IN_PREV,             // detected inconsistency in `.prev` pointers
//     LIST_LOOP_IN_FREE,             // detected inconsistency in `.prev` pointers
// };

// typedef struct {
//     list_containing_t   data;
//     size_t              next, prev;
// } list_element_t;

// typedef struct {
//     LIST_ERRNO          errno;
//
//     list_element_t      *elements;
//
//     size_t              size, capacity,
//                         free_idx;
// } list_t;


list_t * constructor(size_t capacity) {
    list_t * list = (list_t *) calloc(1, sizeof(list_t));

    if (list == NULL)
        return NULL;

    ++capacity; // Добавляем zombi 0 элемент, индексируем с 1

    list->capacity = capacity;
    list->size = 1;

    list_element_t *buf = (list_element_t *) calloc(capacity * sizeof(list->elements[0]), 1);

    if (buf == NULL) {
        free(list);
        return NULL;
    }

    list->elements = buf;

    list->elements[0].data = POISON;

    list->elements[0].next /*list.head*/ = list->elements[0].prev /*list.tail*/ = 0;

    //              ↓------------------------ с 1 т.к. в 0 - zombi элемент
    for (size_t i = 1; i < capacity - 1; ++i) {
        list->elements[i].next = i + 1;
        list->elements[i].prev = SIZE_T_MAX;
    }

    list->elements[capacity - 1].next = 0; // После последнего ничего не идет
    list->elements[capacity - 1].prev = SIZE_T_MAX;

    list->free_idx = 1;

    list->errno = LIST_NO_PROBLEM;
    list->is_line = true;

    return list;
}

void destructor(list_t **list_ptr) {
    (list_ptr != NULL) verified(return;);
    list_t *list = *list_ptr;

    free(list->elements);
    list->elements = NULL;

    list->capacity = list->free_idx = 0;

    free(list);
    *list_ptr = NULL;
}

const char *error(list_t *list) {
    switch(list->errno) {
        case LIST_NO_PROBLEM:               return "LIST_NO_PROBLEM";
        case LIST_POISON_COLLISION:         return "LIST_POISON_COLLISION";
        case LIST_CANNOT_ALLOC_MEMORY:      return "LIST_CANNOT_ALLOC_MEMORY";
        case LIST_CANNOT_REALLOC_MEMORY:    return "LIST_CANNOT_REALLOC_MEMORY";
        case LIST_OVERFLOW:                 return "LIST_OVERFLOW";
        case LIST_INTERNAL_STRUCT_DAMAGED:  return "LIST_INTERNAL_STRUCT_DAMAGED";
        case LIST_NULL_POINTER:             return "LIST_NULL_POINTER";
        case LIST_INVALID_INDEX:            return "LIST_INVALID_INDEX";
        case LIST_FREE_LIST_CORRUPT:        return "LIST_FREE_LIST_CORRUPT";
        case LIST_SIZE_MISMATCH:            return "LIST_SIZE_MISMATCH";
        case LIST_LOOP_IN_NEXT:           return "LIST_LOOP_IN_NEXT";
        case LIST_LOOP_IN_PREV:           return "LIST_LOOP_IN_PREV";
        case LIST_LOOP_IN_FREE:           return "LIST_LOOP_IN_FREE";
        default:                            return "Unknown error";
    }
}

// ------------------------------ capacity ------------------------------

bool empty(list_t *list) {
    verifier(list) verified(return true);
    if (list->size <= 1) return true;
    return false;
}

size_t size(list_t *list) {
    verifier(list) verified(return 0;);
    return list->size - 1;
}

size_t capacity(list_t *list) {
    verifier(list) verified(return 0;);
    return list->capacity - 1;
}

// ------------------------------ element access ------------------------------

size_t front(list_t *list) {
    verifier(list) verified(return 0;);
    return list->elements[0].next; // list.head
}

size_t back(list_t *list) {
    verifier(list) verified(return 0;);
    return list->elements[0].prev; // list.tail
}

size_t get_next_element(list_t *list, size_t element_idx) {
    // verifier(list) verified(return 0;);
    if (element_idx < list->capacity)
        return list->elements[element_idx].next;
    return 0;
}

size_t get_prev_element(list_t *list, size_t element_idx) {
    // verifier(list) verified(return 0;);
    if (element_idx < list->capacity)
        return list->elements[element_idx].prev;
    return 0;
}

size_t slow::index(list_t *list, size_t logic_index) {
    verifier(list) verified(return 0;);

    if (list->is_line) {
        if (logic_index >= size(list)) {
            list->errno = LIST_INVALID_INDEX;
            return 0;
        }
        return logic_index + 1; // logical 0 -> physical 1
    }

    size_t now_logic_index = 0;
    for (size_t i = front(list); i != 0; i = get_next_element(list, i), ++now_logic_index) {
        if (now_logic_index == logic_index)
            return i;
    }
    return 0;
}

size_t slow::search(list_t *list, list_containing_t value) {
    verifier(list) verified(return 0;);

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    for (size_t i = front(list); i != 0; i = get_next_element(list, i)) {
        if (list_contairing_t_comparator(elements[i].data, value) == 0)
            return i;
    }
    return 0;
}

// ------------------------------ modifiers ------------------------------

LIST_ERRNO slow::resize(list_t *list, size_t new_capacity) {
    // Allow resize to be called when the only existing errno is LIST_OVERFLOW
    if (list == NULL) return LIST_NULL_POINTER;

    LIST_ERRNO saved_errno = list->errno;
    if (saved_errno == LIST_OVERFLOW) {
        list->errno = LIST_NO_PROBLEM; // temporarily clear so verifier will run
    }

    if (!verifier(list)) {
        // restore original errno if it wasn't overflow (or verifier set new one)
        if (saved_errno == LIST_OVERFLOW && list->errno == LIST_NO_PROBLEM) {
            // verifier passed but set nothing — continue
        } else {
            // verifier discovered an error we can't fix here
            LIST_ERRNO ret = list->errno;
            list->errno = ret;
            return ret;
        }
    }

    // new_capacity argument is logical capacity (without zombi at index 0)
    // internal buffer always holds +1 (zombi) element
    size_t requested = new_capacity + 1;

    size_t used = (list->size > 0) ? list->size - 1 : 0;
    size_t old_cap = list->capacity;

    // cannot shrink below actual used elements
    if (requested <= used) {
        list->errno = LIST_OVERFLOW;
        return LIST_OVERFLOW;
    }

    // shrinking requires the array to be linearized — we won't call linearization here
    if (requested < old_cap && !list->is_line) {
        list->errno = LIST_INTERNAL_STRUCT_DAMAGED;
        return LIST_INTERNAL_STRUCT_DAMAGED;
    }

    // grow or shrink via realloc
    list_element_t *old_buf = list->elements;
    list_element_t *new_buf = (list_element_t *) realloc(old_buf, requested * sizeof(list_element_t));
    if (new_buf == NULL) {
        list->errno = LIST_CANNOT_REALLOC_MEMORY;
        return LIST_CANNOT_REALLOC_MEMORY;
    }

    list->elements = new_buf;

    if (requested > old_cap) {
        // initialize newly appended elements as free-list entries
        size_t first_new = old_cap;
        for (size_t i = first_new; i < requested - 1; ++i) {
            list->elements[i].prev = SIZE_T_MAX;
            list->elements[i].next = i + 1;
        }
        // last new element
        list->elements[requested - 1].prev = SIZE_T_MAX;
        list->elements[requested - 1].next = 0;

        // attach new free segment to existing free list
        if (list->free_idx == 0) {
            // no free elements previously
            list->free_idx = first_new;
        } else {
            // find tail of existing free list and append
            size_t cur = list->free_idx;
            while (list->elements[cur].next != 0) {
                cur = list->elements[cur].next;
            }
            list->elements[cur].next = first_new;
        }
    }

    // update capacity
    list->capacity = requested;

    // resize fixes overflow condition
    list->errno = LIST_NO_PROBLEM;
    list->is_line = list->is_line; // keep previous state

    return LIST_NO_PROBLEM;
}

LIST_ERRNO slow::linearization(list_t *list) {
    verifier(list) verified(return LIST_INTERNAL_STRUCT_DAMAGED;);

    // empty or single-element list is already linear
    if (list->size <= 1) {
        list->is_line = true;
        return LIST_NO_PROBLEM;
    }

    size_t used = list->size - 1; // number of actual elements
    size_t cap = list->capacity;

    list_element_t *old = list->elements;
    list_element_t *new_buf = (list_element_t *) calloc(cap, sizeof(list_element_t));
    if (new_buf == NULL)
        return LIST_CANNOT_REALLOC_MEMORY;

    new_buf[0].data = POISON;
    new_buf[0].next = (used > 0) ? 1 : 0;

    size_t new_idx = 1;
    for (size_t old_idx = front(list); old_idx != 0; old_idx = get_next_element(list, old_idx), ++new_idx) {
        new_buf[new_idx].data = old[old_idx].data;
        new_buf[new_idx].prev = new_idx - 1;
        new_buf[new_idx].next = new_idx + 1;
    }

    new_buf[1].prev = 0;
    new_buf[used].next = 0;
    new_buf[0].prev = used;

    list->free_idx = used + 1;
    for (size_t i = list->free_idx; i < cap - 1; ++i) {
        new_buf[i].prev = SIZE_T_MAX;
        new_buf[i].next = i + 1;
    }
    if (list->free_idx < cap) {
        new_buf[cap - 1].prev = SIZE_T_MAX;
        new_buf[cap - 1].next = 0;
    }

    free(old);
    list->elements = new_buf;
    list->is_line = true;

    return LIST_NO_PROBLEM;
}

// Вернет индекс новой ячейки (список свободных будет валидным после вызова)
function size_t alloc_new_list_element(list_t *list) {
    verifier(list) verified(return 0;);

    size_t free_el_idx = list->free_idx;
    size_t new_free_idx = get_next_element(list, free_el_idx);
    if (new_free_idx == 0) { // Нет места для еще 1 элемента
        list->errno = LIST_OVERFLOW;
        return 0;
    }
    list->free_idx = new_free_idx;
    return free_el_idx;
}

size_t insert(list_t *list, size_t element_idx, list_containing_t value) {
    verifier(list) verified(return 0;);

    if (value == POISON) {
        list->errno = LIST_POISON_COLLISION;
        return 0;
    }

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    size_t next_el_idx = get_next_element(list, element_idx); // Следующий элемент за помещенным
    size_t paste_place = alloc_new_list_element(list);        // Место куда помещаем новый элемент

    bool remain_line = false;
    if (list->is_line && paste_place != 0) {
        if (paste_place == element_idx + 1 && next_el_idx == 0)
            remain_line = true;
    }

    if (list->errno == LIST_OVERFLOW) // Не хватает места для вставки, пользователю необходимо расширить список
        return 0;

    // eval_print(paste_place);
    elements[paste_place].prev = element_idx;
    elements[paste_place].next = next_el_idx;
    elements[paste_place].data = value;
    // eval_print(element_idx);
    elements[element_idx].next = paste_place;
    // eval_print(next_el_idx);
    elements[next_el_idx].prev = paste_place;

    ++list->size;

    list->is_line = remain_line;

    return paste_place;
}

size_t emplace(list_t *list, size_t element_idx, list_containing_t value) {
    verifier(list) verified(return 0;);

    if (value == POISON) {
        list->errno = LIST_POISON_COLLISION;
        return 0;
    }

    return insert(list, get_prev_element(list, element_idx), value);
}

size_t push_front(list_t *list, list_containing_t value) {
    verifier(list) verified(printf("error state\n"); return 0;);

    if (value == POISON) {
        list->errno = LIST_POISON_COLLISION;
        return 0;
    }

    return emplace(list, front(list), value);
}

size_t push_back(list_t *list, list_containing_t value) {
    verifier(list) verified(printf("error state\n"); return 0;);

    if (value == POISON) {
        list->errno = LIST_POISON_COLLISION;
        return 0;
    }

    // eval_print(empty(list));
    // eval_print(back(list));
    return insert(list, back(list), value);
}

void erase(list_t *list, size_t element_idx) {
    verifier(list) verified(printf("error state\n"); return;);

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    bool remain_line = (list->is_line && element_idx == back(list));

    elements[elements[element_idx].prev].next = elements[element_idx].next;
    elements[elements[element_idx].next].prev = elements[element_idx].prev;

    elements[element_idx].prev = SIZE_T_MAX;
    elements[element_idx].next = list->free_idx;

    --list->size;
    list->free_idx = element_idx;

    list->is_line = remain_line;
}

void pop_front(list_t *list) {
    erase(list, front(list));
}

void pop_back(list_t *list) {
    erase(list, back(list));
}

void swap(list_t *list, size_t el_idx_1, size_t el_idx_2) {
    verifier(list) verified(printf("error state\n"); return;);

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    elements[get_prev_element(list, el_idx_1)].next = el_idx_2;
    elements[get_prev_element(list, el_idx_2)].next = el_idx_1;

    elements[get_next_element(list, el_idx_1)].prev = el_idx_2;
    elements[get_next_element(list, el_idx_2)].prev = el_idx_1;

    list->is_line = false;
}

}