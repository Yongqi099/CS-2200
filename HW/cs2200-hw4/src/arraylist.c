/**
 * Name: Yongqi Ma
 * GTID: 903569868
 */

/*  PART 2: A CS-2200 C implementation of the arraylist data structure.
    Implement an array list.
    The methods that are required are all described in the header file. Description for the methods can be found there.

    Hint 1: Review documentation/ man page for malloc, calloc, and realloc.
    Hint 2: Review how an arraylist works.
    Hint 3: You can use GDB if your implentation causes segmentation faults.
*/

#include "arraylist.h"

/* Student code goes below this point */

#include "arraylist.h"
#include <string.h>

arraylist_t *create_arraylist(uint capacity) {
    arraylist_t *arraylist = malloc(sizeof(arraylist_t));
    arraylist->capacity = capacity;
    arraylist->size = 0;
    arraylist->backing_array = malloc(capacity * sizeof(char *));
    return arraylist;
}

void add_at_index(arraylist_t *arraylist, char *data, int index) {
    if (index < 0 || index > arraylist->size) {
        return;
    }
    if (arraylist->size == arraylist->capacity) {
        resize(arraylist);
    }
    for (int i = arraylist->size - 1; i >= index; i--) {
        arraylist->backing_array[i + 1] = arraylist->backing_array[i];
    }
    arraylist->backing_array[index] = strdup(data);
    arraylist->size++;
}

void append(arraylist_t *arraylist, char *data) {
    add_at_index(arraylist, data, arraylist->size);
}

char *remove_from_index(arraylist_t *arraylist, int index) {
    if (index < 0 || index >= arraylist->size) {
        return NULL;
    }
    char *removed_data = arraylist->backing_array[index];
    for (int i = index; i < arraylist->size - 1; i++) {
        arraylist->backing_array[i] = arraylist->backing_array[i + 1];
    }

    arraylist->backing_array[arraylist->size-1] = NULL;
    arraylist->size--;
    return removed_data;
}

void resize(arraylist_t *arraylist) {
    uint new_capacity = arraylist->capacity * 2;
    char **new_array = malloc(new_capacity * sizeof(char *));
    if (new_array == NULL) {
        fprintf(stderr, "Failed to allocate memory for new array\n");
        return;
    }
    for (int i = 0; i < arraylist->size; i++) {
        new_array[i] = arraylist->backing_array[i];
    }

    free(arraylist->backing_array);
    arraylist->backing_array = new_array;
    arraylist->capacity = new_capacity;
}

void destroy(arraylist_t *arraylist) {
    if (!arraylist || !arraylist->backing_array) return;
    for (int i = 0; i < arraylist->capacity; i++) {
        if (!arraylist->backing_array[i]) free(arraylist->backing_array[i]);
    }
    free(arraylist->backing_array);
//    free(arraylist);
}