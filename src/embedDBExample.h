/******************************************************************************/
/**
 * @file		embedDBExample.h
 * @author		EmbedDB Team (See Authors.md)
 * @brief		This file includes and example for insterting and retrieving sequential records for EmbeDB.
 * @copyright	Copyright 2023
 * 			    EmbedDB Team
 * @par Redistribution and use in source and binary forms, with or without
 * 	modification, are permitted provided that the following conditions are met:
 *
 * @par 1.Redistributions of source code must retain the above copyright notice,
 * 	this list of conditions and the following disclaimer.
 *
 * @par 2.Redistributions in binary form must reproduce the above copyright notice,
 * 	this list of conditions and the following disclaimer in the documentation
 * 	and/or other materials provided with the distribution.
 *
 * @par 3.Neither the name of the copyright holder nor the names of its contributors
 * 	may be used to endorse or promote products derived from this software without
 * 	specific prior written permission.
 *
 * @par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * 	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * 	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * 	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * 	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * 	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * 	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * 	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * 	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * 	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * 	POSSIBILITY OF SUCH DAMAGE.
 */
/******************************************************************************/

#ifndef PIO_UNIT_TESTING

#include <errno.h>
#include <string.h>
#include <time.h>

#ifdef DIST
#include "embedDB.h"
#else
#include "embedDB/embedDB.h"
#include "embedDBUtility.h"
#endif

/**
 * 0 = SD Card
 * 1 = Dataflash
 */
#define STORAGE_TYPE 0

#define SUCCESS 0

#ifdef ARDUINO

#if defined(MEMBOARD) && STORAGE_TYPE == 1

#include "dataflashFileInterface.h"

#endif

#include "SDFileInterface.h"
#define getFileInterface getSDInterface
#define setupFile setupSDFile
#define tearDownFile tearDownSDFile

#define DATA_FILE_PATH "dataFile.bin"
#define INDEX_FILE_PATH "indexFile.bin"
#define VAR_DATA_FILE_PATH "varFile.bin"

#else

#include "desktopFileInterface.h"
#define DATA_FILE_PATH "build/artifacts/dataFile.bin"
#define INDEX_FILE_PATH "build/artifacts/indexFile.bin"
#define VAR_DATA_FILE_PATH "build/artifacts/varFile.bin"

#endif

embedDBState* init_state();

uint32_t embedDBExample() {
    uint32_t totalRecordsInserted = 0;
    uint32_t totalRecordsToInsert = 10;

    printf("******************* Performing an example of EmbeDB with sequentially generated data **************\n");
    // init state, see function for details.
    embedDBState* state;
    state = init_state();
    embedDBPrintInit(state);

    // Inserting 10 fixed-length records
    printf("******************* For inserting 10 fixed-length records using embedDBPut() **********************\n");
    // iterate from 0 -> 10
    for (uint32_t i = 0; i < totalRecordsToInsert; i++) {
        // Initalize key (as they must be in ascending order)
        uint32_t key = i;

        // calloc dataPtr in the heap
        void* dataPtr = calloc(1, state->dataSize);

        // set value to be inserted
        *((uint32_t*)dataPtr) = i % 100;

        // perform insertion of key and data value. Record value of embedDBPut to ensure no errors.
        int8_t result = embedDBPut(state, &key, dataPtr);
        printf("Inserted key = %ld and data = %d\n", i, *((uint32_t*)dataPtr));
        if (result != SUCCESS) {
            printf("Error inserting fixed-length records\n");
        }
        totalRecordsInserted++;

        // free dynamic memory
        free(dataPtr);
    }

    // Retrieving 10 fixed-length records
    printf("******************* For retrieving %d fixed-length records using embedDBGet() *********************\n", totalRecordsToInsert);
    for (uint32_t i = 0; i < totalRecordsToInsert; i++) {
        // key for data retrieval
        uint32_t key = i;

        // data pointer for retrieval
        uint32_t returnDataPtr[] = {0, 0, 0};

        // query embedDB
        embedDBGet(state, (void*)&key, (void*)returnDataPtr);

        // print result
        printf("Returned key = %d data = %d\n", key, *returnDataPtr);
    }

    // Iterating over 10 fixed-length records
    printf("******************* Iterating over %d fixed-length records using embedDBNext() ********************\n", 10);

    // declare EmbedDB iterator.
    embedDBIterator it;

    // ensure that itKey is the same size as state->recordSize.
    uint32_t* itKey;

    // Allocate memory for itData matching the size of state->datasize.
    uint32_t* itData[] = {0, 0, 0};

    // specify min and max key to perform search on.
    uint32_t minKey = 0;
    uint32_t maxKey = 9;  // iterating over all records

    // initalize buffer variables.
    it.minKey = &minKey;
    it.maxKey = &maxKey;
    it.minData = NULL;
    it.maxData = NULL;

    // initialize
    embedDBInitIterator(state, &it);

    // while there are records to read.
    while (embedDBNext(state, &it, (void**)&itKey, (void**)&itData)) {
        printf("Iterated key = %d and data = %ld \n", itKey, *(long*)itData);
    }

    // flushing the 10 records from the fixed-length write buffer into non volitile storage
    printf("******************* Flush EmbedDB()'s write buffers to storage ************************************\n");
    embedDBFlush(state);
    printf("Flush complete\n");

    printf("******************* Insert 10 variable-length record **********************************************\n");
    for (uint32_t i = 0; i < 10; i++) {
        // init key
        uint32_t key = totalRecordsInserted;

        // calloc dataPtr in the heap.
        void* dataPtr = calloc(1, state->dataSize);

        // set value to be inserted into embedDB.
        *((uint32_t*)dataPtr) = totalRecordsInserted % 100;

        // specify the length, in bytes.
        uint32_t length = 12;

        char str[] = "Hello World";  // ~ 12 bytes long including null terminator

        // insert variable record
        int8_t result = embedDBPutVar(state, (void*)&key, (void*)dataPtr, (void*)str, length);
        if (result != SUCCESS) {
            printf("Error inserting variable-length records\n");
        }
        printf("Inserted key = %d  fixed-length record= %d, variable-length record = %s\n", key, *((uint32_t*)dataPtr), str);
        totalRecordsInserted++;
        // free dynamic memory.
        free(dataPtr);
        dataPtr = NULL;
    }

    printf("******************* Retrieve a single variable-length record **************************************\n");

    uint32_t key = 10;

    // declare a varDataStream.
    embedDBVarDataStream* varStream = NULL;

    // allocate memory to read fixed data into.
    char fixedRec[] = {0, 0, 0};

    // allocate buffer to read variable record into.
    uint32_t varRecSize = 12;  // This must be at least the same size of the variable record.
    void* varRecBufPtr = malloc(varRecSize);

    // retrieve fixed record and create a data stream.
    embedDBGetVar(state, (void*)&key, (void*)fixedRec, &varStream);
    printf("After calling EmbedDBGetVar()\n");
    printf("Returned key = %d fixed length record data = %d\n", key, *fixedRec);

    // read data from varStream.
    if (varStream != NULL) {
        uint32_t bytesRead;
        // as long as there is bytes to read
        while ((bytesRead = embedDBVarDataStreamRead(state, varStream, varRecBufPtr, varRecSize)) > 0) {
            printf("After calling embedDBVarDataStreamRead()\n");
            printf("Returned key = %d fixed-length record= %d, variable-length record = %s\n", key, *fixedRec, (char*)varRecBufPtr);
            // printf("eek!\n");
        }
        free(varStream);
        varStream = NULL;
    }
    free(varRecBufPtr);
    varRecBufPtr = NULL;

    printf("******************* Iterate over %d variable-length records ***************************************\n", 10);

    // declare embedDB iterator
    embedDBIterator varIt;
    // Memory to store key and fixed data into.
    uint32_t* itVarKey;
    uint32_t varItData[] = {0, 0, 0};

    uint32_t varMinKey = 10, varMaxKey = 19;
    varIt.minKey = &varMinKey;
    varIt.maxKey = &varMaxKey;
    varIt.minData = NULL;
    varIt.maxData = NULL;

    embedDBVarDataStream* itVarStream = NULL;
    // Choose any size. Must be at least the size of the variable record if you would like the entire record on each iteration.
    uint32_t varBufSize = 12;
    void* varDataBuffer = malloc(varBufSize);

    embedDBInitIterator(state, &varIt);

    while (embedDBNextVar(state, &varIt, &itVarKey, varItData, &itVarStream)) {
        /* Process vardata if this record has it */
        if (itVarStream != NULL) {
            uint32_t numBytesRead = 0;
            while ((numBytesRead = embedDBVarDataStreamRead(state, itVarStream, varDataBuffer, varBufSize)) > 0) {
                printf("Iterated key = %d, fixed-length record= %d, variable-length record = %s\n", itVarKey, *varItData, (char*)varDataBuffer);
            }
            free(varStream);
            varStream = NULL;
        }
    }

    free(varDataBuffer);
    varDataBuffer = NULL;
    embedDBCloseIterator(&varIt);

    printf("Example completed!\n");
    return 0;
}

embedDBState* init_state() {
    embedDBState* state = (embedDBState*)malloc(sizeof(embedDBState));

    // ensure successful malloc
    if (state == NULL) {
        printf("Unable to allocate state. Exiting\n");
        exit(-1);
    }
    /* configure EmbedDB state variables */
    // for fixed-length records
    state->keySize = 4;
    state->dataSize = 8;

    // for buffer(s)
    state->pageSize = 512;
    state->bufferSizeInBlocks = 6;
    state->buffer = malloc((size_t)state->bufferSizeInBlocks * state->pageSize);
    // ensure successful malloc
    if (state->buffer == NULL) {
        printf("Unable to allocate buffer. Exciting\n");
        exit(-1);
    }

    // for learned indexing and bitmap
    state->numSplinePoints = 300;
    state->bitmapSize = 1;

    // address storage characteristics
    state->numDataPages = 1000;
    state->numIndexPages = 48;
    state->numVarPages = 76;
    state->eraseSizeInPages = 4;

    if (STORAGE_TYPE == 1) {
        printf("Dataflash storage is not currently supported in this example. Proceeding using SD storage.\n");
    }

    char dataPath[] = DATA_FILE_PATH, indexPath[] = INDEX_FILE_PATH, varPath[] = VAR_DATA_FILE_PATH;
    state->fileInterface = getFileInterface();
    state->dataFile = setupFile(dataPath);
    state->indexFile = setupFile(indexPath);
    state->varFile = setupFile(varPath);

    // enable parameters
    state->parameters = EMBEDDB_USE_BMAP | EMBEDDB_USE_INDEX | EMBEDDB_USE_VDATA | EMBEDDB_RESET_DATA;

    // Setup for data and bitmap comparison functions */
    state->inBitmap = inBitmapInt8;
    state->updateBitmap = updateBitmapInt8;
    state->buildBitmapFromRange = buildBitmapInt8FromRange;
    state->compareKey = int32Comparator;
    state->compareData = int32Comparator;

    // init embedDB
    size_t splineMaxError = 1;
    if (embedDBInit(state, splineMaxError) != 0) {
        printf("Initialization error");
        exit(-1);
    }

    embedDBResetStats(state);
    return state;
}

#endif
