#define AOS
#ifndef LIST_H
#define LIST_H

#ifdef AOS
#include "list_aos.h"

namespace mylist {
    using namespace mylist_aos;
}

#endif

#endif // LIST_H