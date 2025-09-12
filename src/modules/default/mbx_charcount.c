#include "mbx_charcount.h"
#include <ctype.h>
#include <stdio.h>

bool mbx_charcount(mojibake_target_t *target, unsigned int index, void *arg)
{
    if (target == NULL || target->block == NULL || index >= target->partition_count)
        return false;

    unsigned char *partition = MOJIBAKE_BLOCK_OFFSET(target, index);
    char_stats_t stats = {0, 0, 0, 0, 0};
    
    printf("=== Partition %d Character Analysis ===\n", index);
    
    // Analyze each character in this partition
    for (int i = 0; i < target->partition_size; i++) {
        unsigned char ch = partition[i];
        
        if (isalpha(ch)) {
            stats.letters++;
        } else if (isdigit(ch)) {
            stats.digits++;
        } else if (isspace(ch)) {
            stats.spaces++;
        } else if (ispunct(ch)) {
            stats.punctuation++;
        } else {
            stats.others++;
        }
    }
    
    // Display statistics
    printf("Letters:     %d\n", stats.letters);
    printf("Digits:      %d\n", stats.digits);
    printf("Spaces:      %d\n", stats.spaces);
    printf("Punctuation: %d\n", stats.punctuation);
    printf("Others:      %d\n", stats.others);
    printf("Total chars: %d\n\n", target->partition_size);
    
    return true;
}