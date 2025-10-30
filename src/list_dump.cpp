#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include "list.h"
#include "stringNthong.h"
#include "io_utils.h"
#include "base.h"

#include <stddef.h>

namespace mylist {

void print_centered(FILE * fp, const char *str, int field_width) {
    size_t str_len = strlen(str);
    if (str_len >= field_width) {
        fprintf(fp, "%s", str); // If string is too long, print as-is
        return;
    }

    size_t left_padding = (field_width - str_len) / 2;
    size_t right_padding = field_width - str_len - left_padding;

    fprintf(fp, "%*s%s%*s", left_padding, "", str, right_padding, "");
}

void _generate_dot_dump(list_t *list, FILE * fp) {
    verifier(list);
    fprintf(fp,
        "digraph DoublyLinkedList {\n"
        "\t// Настройки графа\n"
        "\trankdir=\"LR\";\n");
    if (list->errno != LIST_NO_PROBLEM) {
        fprintf(fp, "\tsplines=ortho\n");
    }
    fprintf(fp, "\tranksep=0.0\n"
        "\tnodesep=0.08\n"
        "\n"
        "\tnode [shape=record, height=0.5, color = \"red\", fontcolor = \"red\", style = \"filled\", fillcolor = \"#ffc5c5\"];\n"
        "\tedge [arrowsize=0.8, minlen=4];\n"
        "\n");

    // fprintf(fp, "\tlist_t [label=\"<head> head = %zu | <tail> tail = %zu | <free_idx> free_idx = %zu\"]\n", list->head, list->tail, list->free_idx);

    list_element_t *elements = list->elements;

    fprintf(fp, "\tnode0 [label=\"<index> index = 0 | <data> data = PSN | { <prev> prev = %lld | <next> next = %zu}\", color = \"black\", fontcolor = \"black\", fillcolor = \"#adeda7ff\"];\n", elements[0].prev, elements[0].next);

    // for (size_t i = front(list), j = get_next_element(list, i); // Добавляем занятые
    //         i != 0 ; i = j, j = get_next_element(list, j)) {
    //     fprintf(fp, "\tnode%zu [label=\"<index> index = %zu | <data> data = %d | { <prev> prev = %lld | <next> next = %zu}\"];\n",
    //         i, i, elements[i].data, elements[i].prev, elements[i].next);
    // }

    for (size_t i = 1; i < list->capacity; ++i) {
        if (elements[i].prev == SIZE_T_MAX) { // Это free элемент
            // printf("free node%zu\n", i);
            fprintf(fp, "\tnode%zu [label=\"<index> index = %zu | <data> data = empty | { <prev> prev = %lld | <next> next = %zu}\", color = \"#ff9500ff\", fontcolor = \"black\", fillcolor = \"#ffd997\"];\n",
                i, i, elements[i].prev, elements[i].next);
        } else { // Это занятый элемент
            // printf("node%zu\n", i);
            fprintf(fp, "\tnode%zu [label=\"<index> index = %zu | <data> data = %d | { <prev> prev = %lld | <next> next = %zu}\", color = \"black\", fontcolor = \"black\", fillcolor = \"#d0ceceff\"];\n",
                        i, i, elements[i].data, elements[i].prev, elements[i].next);
        }
    }

    fprintf(fp,
        "\n"
        "\t// Выравнивание\n"
    );

    for (size_t i = 1; i <= capacity(list); ++i) {
        fprintf(fp, "\tnode%zu -> node%zu [weight=100, color = \"#ffffff\"];\n", i-1, i);
    }

    fprintf(fp, "\n\t// Связи next\n");

    size_t el1 = 0, el2 = get_next_element(list, el1);
    for (size_t i = 0; i < list->size; ++i) {
        if (el2 > list->capacity || el1 > list->capacity)
            fprintf(fp, "\tnode%zu -> node%zu [color = \"#cc0c0cff\", constraint=false];\n", el1, el2);
        else
            fprintf(fp, "\tnode%zu -> node%zu [color = \"#0c0ccc\", constraint=false];\n", el1, el2);
        el1 = el2;
        el2 = get_next_element(list, el2);
    }

    fprintf(fp, "\n\t// Связи free\n");

    for (size_t i = list->free_idx, j = get_next_element(list, i);
            i != 0 ; i = j, j = get_next_element(list, j)) {
        fprintf(fp, "\tnode%zu -> node%zu [color = \"#e39e3d\", constraint=false];\n", i, j);
    }

    fprintf(fp, "\n\t// Связи prev\n");

    // el1 = 0, el2 = get_next_element(list, el1);
    // for (size_t i = 0, j = get_prev_element(list, i);
    //         j != 0 ; i = j, j = get_prev_element(list, j)) {
    //     // if (!(i > list->capacity || j > list->capacity))
    //     fprintf(fp, "\tnode%zu -> node%zu [color = \"#3dad3d\", arrowhead=inv, constraint=false];\n", j, i);
    // }

    el1 = 0, el2 = get_prev_element(list, el1);
    for (size_t i = 0; i < list->size; ++i) {
        fprintf(fp, "\tnode%zu -> node%zu [color = \"#3dad3d\", constraint=false];\n", el1, el2);
        el1 = el2;
        el2 = get_prev_element(list, el2);
    }

//     fprintf(fp, "\n\t// Head, tail, free\n");
//
//     fprintf(fp,
//         "\thead [label=\"head\", color = \"gray\", fontcolor = \"black\"];\n"
//         "\ttail [label=\"tail\", color = \"gray\", fontcolor = \"black\"];\n"
//         "\tfree [label=\"free\", color = \"#e39e3d\", fontcolor = \"black\"];\n"
//         "\thead -> node%zu [color = \"gray\", constraint=false];\n"
//         "\ttail -> node%zu [color = \"gray\", constraint=false];\n"
//         "\tfree -> node%zu [color = \"#e39e3d\", constraint=false];\n",
//         list->elements[0].next, list->elements[0].prev, list->free_idx
//     );

    // if (front(list) < back(list)) {
    //     fprintf(fp, "\thead -> tail [weight=100, style=invis];\n");
    // }
    // else {
    //     fprintf(fp, "\ttail -> head [weight=100, style=invis];\n");
    // }

    fprintf(fp, "}");
}

const char *_generate_image(list_t *list, const char *dir_name) {
    local size_t iter;

    char tmp_dot_filename[128] = "";
    if (dir_name[strlen(dir_name) - 1] == '/')
        snprintf(tmp_dot_filename, 128, "%slist_dump.dot.tmp", dir_name);
    else
        snprintf(tmp_dot_filename, 128, "%s/list_dump.dot.tmp", dir_name);

    FILE *tmp_dot_file = fopen(tmp_dot_filename, "w");
    _generate_dot_dump(list, tmp_dot_file);
    fclose(tmp_dot_file);

    local char image_filename[128] = "";
    char image_path[128] = "";

    snprintf(image_filename, 128, "image%zu.svg", iter++);

    if (dir_name[strlen(dir_name) - 1] == '/')
        snprintf(image_path, 128, "%s%s", dir_name, image_filename);
    else
        snprintf(image_path, 128, "%s/%s", dir_name, image_filename);

    char command[128] = "";
    snprintf(command, 128, "dot -Tsvg %s -o %s", tmp_dot_filename, image_path);
    system(command);

    return image_filename;
}

void _dump_impl(list_t *list, FILE *logfile, const char *log_dirname, const char *prompt,
        int line, const char *func, const char *file) {
    verifier(list);
    fprintf(logfile, "<h3>-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- "
        "LIST DUMP"" -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-</h3>\n");
    fprintf(logfile, "reason: %s\n", prompt);
    fprintf(logfile, "dump from %s:%d at %s\n", file, line, func);
    fprintf(logfile, "\n");
    fprintf(logfile, "list struct at <font style=\"color :#03CFCF;\">%p</font>\n", list);
    fprintf(logfile, "size is %zu\t capacity is %zu\n", list->size, list->capacity);
    fprintf(logfile, "front is %zu\t back is %zu\n", list->elements[0].next, list->elements[0].prev);
    fprintf(logfile, "free_idx is %zu\n", list->free_idx);
    if (list->errno != LIST_NO_PROBLEM) {
        fprintf(logfile, "<font style=\"color :red;\">LIST in invalid state</font>, errno is %s\n", error(list));
    }
    fprintf(logfile, "elements at <font style=\"color :#03CFCF;\">%p</font>\n\n", list->elements);

    list_element_t *elements = list->elements;

    const size_t DLINA_CTPOKI = 128;
    char CTPOKA[DLINA_CTPOKI] = "";
    print_centered(logfile, "", 6);
    fprintf(logfile, "|");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, DLINA_CTPOKI, "[%zu]", i);
        print_centered(logfile, CTPOKA, 9);
        fprintf(logfile, "|");
    }
    fprintf(logfile, "\n");

    size_t free_idx = list->free_idx;
    fprintf(logfile, "data: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        if (i == 0)
            snprintf(CTPOKA, DLINA_CTPOKI, "PSN");
        else if (i == free_idx) {
            snprintf(CTPOKA, DLINA_CTPOKI, "empty");
            free_idx = elements[free_idx].next;
        }
        else
            snprintf(CTPOKA, DLINA_CTPOKI, "%d", elements[i].data);

        // fprintf(logfile, " [%zu] ", i);
        print_centered(logfile, CTPOKA, 9);
        fprintf(logfile, "|");
    }
    fprintf(logfile, "\n");

    fprintf(logfile, "next: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, DLINA_CTPOKI, "%zu", elements[i].next);

        // fprintf(logfile, " [%zu] ", i);
        print_centered(logfile, CTPOKA, 9);
        fprintf(logfile, "|");
    }
    fprintf(logfile, "\n");

    fprintf(logfile, "prev: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, DLINA_CTPOKI, "%lld", elements[i].prev);

        // fprintf(logfile, " [%zu] ", i);
        print_centered(logfile, CTPOKA, 9);
        fprintf(logfile, "|");
    }
    fprintf(logfile, "\n\n");

    const char * image_filename = _generate_image(list, log_dirname);

    fprintf(logfile, "<img src=\"%s\">\n", image_filename);
}

}