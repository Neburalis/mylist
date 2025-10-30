#include <stdio.h>
#include <string.h>
#include <time.h>

#include "base.h"
#include "list.h"
#include "io_utils.h"

using namespace mylist;

global const time_t _start_time = time(NULL) - (clock() / CLOCKS_PER_SEC);

const char *create_dump_directory() {
    create_folder_if_not_exists("logs/");
    local char buf[128] = "";
    char start_time_string[32] = "";
    strftime(start_time_string, 32, "%FT%TZ", gmtime(&_start_time));
    snprintf(buf, 128, "logs/log%s", start_time_string);
    create_folder_if_not_exists(buf);

    return buf;
}

#define DO(code) \
    code         \
    dump(list, logfile, dump_dirname, "Dump after doing <font style=\"color: blue;\">" #code "</font>");

int main() {
    printf("Hello, kitty!\n");
    const char * dump_dirname = create_dump_directory();

    char log_filename[128] = "";
    char start_time_string[32] = "";
    strftime(start_time_string, 32, "%T %F", gmtime(&_start_time));
    snprintf(log_filename, 128, "%s/log.html", dump_dirname);

    FILE *logfile = fopen(log_filename, "a");
    fprintf(logfile,
        "<html>\n"
        "<head>\n"
        "\t<title>%s</title>\n"
        "</head>\n"
        "<body>\n"
        "<pre>\n", start_time_string);

    mylist::list_t *list = mylist::constructor(3);

    DO(size_t idx1 = push_front(list, 5);)

    DO(push_front(list, 4);)

    DO(list->elements[1].next = 690;)
    // printf("%d", verifier(list));

    DO(slow::resize(list, 10);)

    DO(push_front(list, 3);)

    DO(insert(list, idx1, 52);)

    DO(emplace(list, idx1, 1000-7);)

    DO(erase(list, 2);)

    DO(push_back(list, 1);)

    // DO(list->size = 4;)
    // printf("%d", verifier(list));

    for (int i = 3; i < 7; ++i) {
        DO(push_front(list, i*10);)
    }

    DO(pop_back(list);)
    DO(pop_front(list);)
    DO(erase(list, 4);)
    DO(erase(list, 6);)

    destructor(&list);

    fprintf(logfile, "</pre>\n</body>\n</html>");
    fclose(logfile);

    char command[128] = "";
    snprintf(command, sizeof(command), "open %s", log_filename);
    system(command);
    return 0;
}