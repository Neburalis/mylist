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

    mylist::list_t *list = mylist::constructor(10);

    size_t idx1 = push_front(list, 5);
    dump(list, logfile, dump_dirname, "Dump after adding 5");

    push_front(list, 4);
    dump(list, logfile, dump_dirname, "Dump after adding 4");

    push_front(list, 3);
    dump(list, logfile, dump_dirname, "Dump after adding 3");

    insert(list, idx1, 52);
    dump(list, logfile, dump_dirname, "Dump after adding 52");

    emplace(list, idx1, 1000-7);
    dump(list, logfile, dump_dirname, "Dump after adding 1000-7 𝕯𝖊𝖆𝖉 𝖎𝖓𝖘𝖎𝖉𝖊");

    push_back(list, 1);
    dump(list, logfile, dump_dirname, "Dump after adding 1 to back");

    destructor(&list);

    fprintf(logfile, "</pre>\n</body>\n</html>");
    fclose(logfile);

    char command[128] = "";
    snprintf(command, sizeof(command), "open %s", log_filename);
    system(command);
    return 0;
}