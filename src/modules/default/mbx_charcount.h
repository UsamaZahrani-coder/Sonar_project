#ifndef MBX_CHARCOUNT_H
#define MBX_CHARCOUNT_H
#include <stdbool.h>

#include "mojibake/mojibake.h"

// Character frequency analysis module
bool mbx_charcount(mojibake_target_t *target, unsigned int index, void *arg);

// Helper structure for character counting
typedef struct {
    int letters;
    int digits;
    int spaces;
    int punctuation;
    int others;
} char_stats_t;

#endif