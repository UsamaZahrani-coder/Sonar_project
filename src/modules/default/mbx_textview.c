#include "mbx_textview.h"
#include <ctype.h>
#include <stdio.h>

bool mbx_textview(mojibake_target_t *target, unsigned int index, void *arg)
{
    if (target == NULL || target->block == NULL || index >= target->partition_count)
        return false;

    unsigned char *partition = MOJIBAKE_BLOCK_OFFSET(target, index);
    
    printf("=== Partition %d Text Preview ===\n", index);
    printf("Text content: ");
    
    // Display readable characters, replace non-printable with dots
    for (int i = 0; i < target->partition_size; i++) {
        unsigned char ch = partition[i];
        
        if (isprint(ch)) {
            printf("%c", ch);
        } else if (ch == '\n') {
            printf("\\n");
        } else if (ch == '\t') {
            printf("\\t");
        } else if (ch == '\r') {
            printf("\\r");
        } else {
            printf(".");
        }
    }
    
    printf("\n\n");
    return true;
}