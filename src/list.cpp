#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>

#include "ansi.h"
#include "list.h"
#include "base.h"

namespace mylist {

#define verified(code) \
    || ({code; false;})

const int POISON = 0xDEDB333C0CA1;

int list_contairing_t_comparator(int first, int second) {
    return first - second;
}

// typedef int list_containing_t;

// enum LIST_ERRNO {
//     LIST_NO_PROBLEM = 0,
//     LIST_POISON_COLLISION,
//     LIST_CANNOT_ALLOC_MEMORY,
//     LIST_CANNOT_REALLOC_MEMORY,
//     LIST_OVERFLOW,
//     LIST_INTERNAL_STRUCT_DAMAGED,
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

bool verifier(list_t *list) {
    if (list == NULL)           return false;
    if (list->elements == NULL) {
        list->errno = LIST_INTERNAL_STRUCT_DAMAGED;
        return false;
    }
    if (list->errno != LIST_NO_PROBLEM)
        return false;
    return true;
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
    if (!empty(list))
        return list->elements[0].next; // list.head
    else return 0;
}

size_t back(list_t *list) {
    verifier(list) verified(return 0;);
    if (!empty(list))
        return list->elements[0].prev; // list.tail
    else return 0;
}

size_t get_next_element(list_t *list, size_t element_idx) {
    verifier(list) verified(return 0;);
    return list->elements[element_idx].next;
}

size_t get_prev_element(list_t *list, size_t element_idx) {
    verifier(list) verified(return 0;);
    return list->elements[element_idx].prev;
}

size_t slow::index(list_t *list, size_t logic_index) {
    verifier(list) verified(return 0;);

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
    verifier(list) || list->errno == LIST_OVERFLOW verified(return list->errno;);

    if (new_capacity++ <= size(list))
        return LIST_OVERFLOW;

    list_element_t *old_elements = list->elements;
    list_element_t *new_buf = (list_element_t *) calloc(new_capacity, sizeof(list_element_t));
    if (new_buf == NULL) {
        return LIST_CANNOT_REALLOC_MEMORY;
    }

    // list_t tmp_list = *list;
    // tmp_list.elements = new_buf;
    // tmp_list.capacity = new_capacity;

    // dump(list, stdout, "logs/test", "Dump old list into resize after create new buf");
    // dump(&tmp_list, stdout, "logs/test", "Dump new list into resize after create new buf");
    // printf("\n\n\n");

    new_buf[0].data = POISON;
    new_buf[0].next = 1;
    size_t new_idx = 1;
    size_t old_idx = front(list);
    do {
        new_buf[new_idx].data = old_elements[old_idx].data;
        new_buf[new_idx].next = new_idx + 1;
        new_buf[new_idx].prev = new_idx - 1;
        ++new_idx;
        old_idx = get_next_element(list, old_idx);
        // printf("old_idx = %zu \tnew_idx = %zu\n", old_idx, new_idx);
        // dump(list, stdout, "logs/test", "Dump old list into resize into while iteration");
        // dump(&tmp_list, stdout, "logs/test", "Dump new list into resize into while iteration");
        // printf("\n\n\n");
    } while (old_idx != 0);

    new_buf[--new_idx] /*последний не пустой*/ .next = 0;
    new_buf[0].prev = new_idx;

    list->elements = new_buf;
    list->free_idx = ++new_idx;
    list->capacity = new_capacity;

    for ( ; new_idx < new_capacity - 1; ++new_idx) {
        // printf("[%zu]\t", new_idx);
        new_buf[new_idx].prev = SIZE_T_MAX;
        new_buf[new_idx].next = new_idx + 1;
        // printf(".prev = %lld \t.next = %lld\n", new_buf[new_idx].prev, new_buf[new_idx].next);
    }

    new_buf[new_idx].prev = SIZE_T_MAX;
    new_buf[new_idx].next = 0;

    // dump(list, stdout, "logs/test", "Dump new list into resize into while iteration");
    free(old_elements);
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

function size_t _push_to_empty(list_t *list, list_containing_t value) {
    verifier(list) verified(printf("error state\n"); return 0;);

    if (value == POISON) {
        list->errno = LIST_POISON_COLLISION;
        return 0;
    }

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    size_t paste_place = alloc_new_list_element(list);
    if (paste_place == 0)
        return 0;

    elements[paste_place].next = elements[paste_place].prev = 0;
    elements[paste_place].data = value;

    list->elements[0].next /*list.head*/ = list->elements[0].prev /*list.tail*/ = paste_place;

    ++list->size;
    return paste_place;
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

    if (list->errno == LIST_OVERFLOW) // Не хватает места для вставки, пользователю необходимо расширить список
        return 0;

    elements[paste_place].prev = element_idx;
    elements[paste_place].next = next_el_idx;
    elements[paste_place].data = value;
    elements[element_idx].next = paste_place;
    elements[next_el_idx].prev = paste_place;

    return paste_place;
}

size_t emplace(list_t *list, size_t element_idx, list_containing_t value) {
    verifier(list) verified(return 0;);

    if (value == POISON) {
        list->errno = LIST_POISON_COLLISION;
        return 0;
    }

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    size_t prev_el_idx = get_prev_element(list, element_idx); // Предыдущий элемент перед помещенным
    size_t paste_place = alloc_new_list_element(list);        // Место куда помещаем новый элемент

    if (list->errno == LIST_OVERFLOW) // Не хватает места для вставки, пользователю необходимо расширить список
        return 0;

    elements[paste_place].prev = prev_el_idx;
    elements[paste_place].next = element_idx;
    elements[paste_place].data = value;
    elements[element_idx].prev = paste_place;
    elements[prev_el_idx].next = paste_place;

    return paste_place;
}

size_t push_front(list_t *list, list_containing_t value) {
    verifier(list) verified(printf("error state\n"); return 0;);

    if (value == POISON) {
        list->errno = LIST_POISON_COLLISION;
        return 0;
    }

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    if (empty(list)) {
        return _push_to_empty(list, value);
    } else { // Не пустой
        return emplace(list, front(list), value);
    }
}

size_t push_back(list_t *list, list_containing_t value) {
    verifier(list) verified(printf("error state\n"); return 0;);

    if (value == POISON) {
        list->errno = LIST_POISON_COLLISION;
        return 0;
    }

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    if (empty(list)) {
        return _push_to_empty(list, value);
    } else { // Не пустой
        return insert(list, back(list), value);
    }
}

void erase(list_t *list, size_t element_idx) {
    verifier(list) verified(printf("error state\n"); return;);

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list

    elements[elements[element_idx].prev].next = elements[element_idx].next;
    elements[elements[element_idx].next].prev = elements[element_idx].prev;

    elements[element_idx].prev = -1;
    elements[element_idx].next = list->free_idx;

    --list->size;
    list->free_idx = element_idx;
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
}

}