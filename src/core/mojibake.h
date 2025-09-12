/* Mojibake 1.0.0a */
#ifndef CRAFT_H
#define CRAFT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define MOJIBAKE_VERSION 1

#define MOJIBAKE_DEFAULT_PARTITION_COUNT 8
#define MOJIBAKE_MAX_FILE_SIZE 1024 * 1024 * 4
#define RETURN_NULL_IF(con) \
    if ((con))              \
    {                       \
        return NULL;        \
    }

#define MOJIBAKE_BASE_OFFSET(b, i, sf) (b) + ((i) * (sf))
#define MOJIBAKE_BLOCK_OFFSET(t, i) MOJIBAKE_BASE_OFFSET(((t)->block), (i), ((t)->partition_size))

typedef struct mojibake_target_t mojibake_target_t;
typedef struct mojibake_partition_t mojibake_partition_t;
typedef bool (*mojibake_partition_callback_t)(mojibake_target_t *, unsigned int index, void *arg);

struct mojibake_partition_t
{
    unsigned int index;
};

struct mojibake_target_t
{
    unsigned int version;
    unsigned int partition_size;
    unsigned int partition_count;
    unsigned int extra;
    unsigned int size;
    void *block;
    mojibake_partition_t *partitions;
};

mojibake_partition_t *mojibake_partitionize(mojibake_target_t *target);
void mojibake_departitionize(mojibake_partition_t *partitions);
mojibake_target_t *mojibake_open(char *file_path, unsigned int partition_count);
void mojibake_close(mojibake_target_t *target);
void mojibake_print(mojibake_target_t *target);
bool mojibake_execute(mojibake_target_t *target, mojibake_partition_callback_t callback, void *arg);

#endif
