
#ifndef SORT_WRAPPER_H
#define SORT_WRAPPER_H

#include "../schema.h"
#include "../advancedQueries.h"
#include "external_sort.h"
#include "in_memory_sort.h"
#include "flash_minsort.h"

#include <string.h>
#include <desktopFileInterface.h>

#define SORT_DATA_LOCATION "sort_data.bin"
#define SORT_TEMP_LOCATION "sort_temp.bin"
#define SORT_ORDER_LOCATION "sort_order.bin"

typedef struct embedDBOperator embedDBOperator;
typedef struct orderByInfo orderByInfo;

void initSort(embedDBOperator *op);
uint8_t readNextRecord(file_iterator_state_t *state, void *buffer);

typedef struct {
    uint32_t key;
    void *value;
} rowData;

#endif