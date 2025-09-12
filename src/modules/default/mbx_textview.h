#ifndef MBX_TEXTVIEW_H
#define MBX_TEXTVIEW_H
#include <stdbool.h>

#include "mojibake/mojibake.h"

// Text preview module - displays readable characters
bool mbx_textview(mojibake_target_t *target, unsigned int index, void *arg);

#endif