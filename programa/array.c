#ifndef ARRAY
#define ARRAY
#include <stdlib.h>
#include "string.c"

// O------------------- PtrArray implementation -------------------O

// Array that stores pointers.
typedef struct PtrArray {
  void **data;  // Pointer to first item (pointer to a void pointer).
  int len;      // Array's length (number of used items).
  int capacity; // Array's capacity (number of items that can be stored).
} PtrArray;

// Creates a new array that stores pointers.
PtrArray *newPtrArray() {
  const int INITIAL_CAPACITY = 10;
  void **data = malloc(sizeof(void *) * INITIAL_CAPACITY);
  if (data == NULL) {
    return NULL;
  }

  PtrArray *array = (PtrArray *)malloc(sizeof(PtrArray));
  array->data = data;
  array->len = 0;
  array->capacity = INITIAL_CAPACITY;
  return array;
}

// Resizes the capacity of the array.
// If successful, the array will be able to store newCapacity items.
void resizePtrArray(PtrArray *array, int newCapacity) {
  if (array == NULL) {
    return;
  }
  array->data = (void **)realloc(array->data, sizeof(void *) * newCapacity);
  array->capacity = newCapacity;
}

// Appends an item to the array.
void ptrArrayAppend(void *item, PtrArray *array) {
  if (array == NULL) {
    return;
  }
  if (array->len == array->capacity) {
    resizePtrArray(array,
                   array->capacity *
                       2); // The capacity doubles every time it gets full.
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
    array->data[i] = array->data[i + 1];
    i++;
  }
  array->len--;
}

void deletePtrArray(PtrArray *array) {
  free(array->data);
  free(array);
}

// O------------------- IntArray implementation -------------------O

// Array that stores integers.
typedef struct IntArray {
  int *data;    // Pointer to first item.
  int len;      // Array's length (number of used items).
  int capacity; // Array's capacity (number of items that can be stored).
} IntArray;

// Creates a new array that stores integers.
IntArray *newIntArray() {
  const int INITIAL_CAPACITY = 10;
  int *data = malloc(sizeof(int) * INITIAL_CAPACITY);
  if (data == NULL) {
    return NULL;
  }

  IntArray *array = (IntArray *)malloc(sizeof(IntArray));
  array->data = data;
  array->len = 0;
  array->capacity = INITIAL_CAPACITY;
  return array;
}

// Resizes the capacity of the array.
// If successful, the array will be able to store newCapacity items.
void resizeIntArray(IntArray *array, int newCapacity) {
  if (array == NULL) {
    return;
  }
  array->data = (int *)realloc(array->data, sizeof(int) * newCapacity);
  array->capacity = newCapacity;
}

// Appends an item to the array.
void intArrayAppend(int item, IntArray *array) {
  if (array == NULL) {
    return;
  }
  if (array->len == array->capacity) {
    resizeIntArray(array,
                   array->capacity *
                       2); // The capacity doubles every time it gets full.
  }
  array->data[array->len] = item;
  array->len++;
}

// Removes an item from the array.
void intArrayRemove(int index, IntArray *array) {
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
    array->data[i] = array->data[i + 1];
    i++;
  }
  array->len--;
}

void deleteIntArray(IntArray *array) {
  free(array->data);
  free(array);
}

// O------------------ FloatArray implementation ------------------O

// Array that stores floats.
typedef struct FloatArray {
  float *data;  // Pointer to first item.
  int len;      // Array's length (number of used items).
  int capacity; // Array's capacity (number of items that can be stored).
} FloatArray;

// Creates a new array that stores integers.
FloatArray *newFloatArray() {
  const int INITIAL_CAPACITY = 10;
  float *data = malloc(sizeof(float) * INITIAL_CAPACITY);
  if (data == NULL) {
    return NULL;
  }

  FloatArray *array = (FloatArray *)malloc(sizeof(FloatArray));
  array->data = data;
  array->len = 0;
  array->capacity = INITIAL_CAPACITY;
  return array;
}

// Resizes the capacity of the array.
// If successful, the array will be able to store newCapacity items.
void resizeFloatArray(FloatArray *array, int newCapacity) {
  if (array == NULL) {
    return;
  }
  array->data = (float *)realloc(array->data, sizeof(float) * newCapacity);
  array->capacity = newCapacity;
}

// Appends an item to the array.
void floatArrayAppend(float item, FloatArray *array) {
  if (array == NULL) {
    return;
  }
  if (array->len == array->capacity) {
    resizeFloatArray(array,
                     array->capacity *
                         2); // The capacity doubles every time it gets full.
  }
  array->data[array->len] = item;
  array->len++;
}

// Removes an item from the array.
void floatArrayRemove(int index, FloatArray *array) {
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
    array->data[i] = array->data[i + 1];
    i++;
  }
  array->len--;
}

void deleteFloatArray(FloatArray *array) {
  free(array->data);
  free(array);
}

// O----------------------- Other utilities -----------------------O

// Deletes every string in the array and then deletes the array itself.
void deleteStringArray(PtrArray *ptr) {
  for (int i = 0; i < ptr->len; i++) {
    deleteString((String*)(ptr->data[i]));
  }
  deletePtrArray(ptr);
}

#endif
