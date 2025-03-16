#include <stdlib.h>

#ifndef ARRAY
#define ARRAY


// O------------------- PtrArray implementation -------------------O

// Array that stores pointers.
typedef struct PtrArray {
  void **data;  // Pointer to first item (pointer to a void pointer).
  int len;  // Array's length (number of used items).
  int capacity;  // Array's capacity (number of items that can be stored).
} PtrArray;

// Creates a new array that stores pointers.
PtrArray *newPtrArray() {
  const int INITIAL_CAPACITY = 10;
  void **data = malloc(sizeof(void*) * INITIAL_CAPACITY);
  if (data == NULL) {
    return NULL;
  }

  PtrArray *array = (PtrArray*)malloc(sizeof(PtrArray));
  array->data = data;
  array->len = 0;
  array->capacity = INITIAL_CAPACITY;
  return array;
}

// Resizes the capacity of the array.
// If successful, the array will be able to store newCapacity items.
void resizePrtArray(PtrArray *array, int newCapacity) {
  if (array == NULL) {
    return;
  }
  array->data = (void**)realloc(array->data, sizeof(void*) * newCapacity);
  array->capacity *= 2;
}

// Appends an item to the array.
void ptrArrayAppend(void *item, PtrArray *array) {
  if (array == NULL) {
    return;
  }
  if (array->len == array->capacity) {
    resizePrtArray(array, array->capacity * 2);  // The capacity doubles every time it gets full.
  }
  array->data[array->len] = item;
  array->len++;
}

// Removes an item from the array.
void ptrArrayRemove(int index, PtrArray *array) {
  if (array == NULL) {
    return;
  }
  if (array->len == 0) {
    return;
  }
  if (index >= array->len) {
    return;
  }
  int i = index;
  while (i < array->len) {
    array->data[i] = array->data[i+1];
  }
}

// O------------------- IntArray implementation -------------------O


// O------------------- Type-independent implementations -------------------O

// Deallocates an array regardless of the type (works for PtrArray, IntArray, etc).
// If freeItems is equals to 1, calls free on each item (useful to delete an array of String*).
void deleteArray(PtrArray *array) {
  free(array->data);
  free(array);
}


#endif
