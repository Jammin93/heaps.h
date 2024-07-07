// Static library for generic min and max heaps.
#ifndef HEAPS_H
#define HEAPS

#define VERSION "1.0.1"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0
#define bool short

#define ALLOC_ERROR 1
#define EMPTY_ARRAY_ERROR 1

// todo: add prototypes

typedef struct {
	void *elems;                    // Dynamic array to be heapified.
	long long size;                 // Allocated size of the array.
	long long logsize;              // Logical size of the array.
	long long elemsize;             // Size of the elements in bytes.
	bool is_max;                    // Whether the heap is a max heap.
	short (*cmp_fn)(void*, void*);  // Element comparison function.
	void (*free_fn)(void*);         // Freeing function for nested pointers.
} HEAP;

static void
swap(void *vp1, void *vp2, long long elemsize) {
	/*	Swap the bit patterns of the two void pointers. The number of bytes to 
		swap is determined by `elemsize`. 
	*/
	unsigned char *p = vp1, *r = vp2, temp;
	for (long long i = 0; i < elemsize; ++i) {
		temp = p[i];
		p[i] = r[i];
		r[i] = temp;
	}

}

static bool
gte(void *vp1, void *vp2, short (*cmp_fn)(void*, void*)) {
	short res = cmp_fn(vp1, vp2);
	return (res == 0 || res == 1);
}

static bool
lte(void *vp1, void *vp2, short (*cmp_fn)(void*, void*)) {
	short res = cmp_fn(vp1, vp2);
	return (res == 0 || res == -1);
}

static short
heap_double(HEAP *heap) {
	// Double the size of the heap.
	heap->size *= 2;
	heap->elems = realloc(heap->elems, heap->size * heap->elemsize);
	if (! heap->elems) {
		return ALLOC_ERROR;
	}
	return 0;
}

static void
sift_down(HEAP *heap, long long idx, long long upper) {
	/*	For max heaps, move the value at `idx` down in the heap until it is 
		smaller than its parent and larger than both of its children. For min
		heaps, move the value at `idx` down until the opposite is true.
	*/
	char *elems = (char*) heap->elems;
	long long left, right, swap_idx;
	/*	The inequality function for comparing the values of child nodes with
		the value at `idx`. For max heaps, we use `gte`, and for min heaps, we 
		use `lte`.
	*/
	bool (*ineq_fn)(void*, void*, short (*)(void*, void*));
	if (heap->is_max) {
		ineq_fn = &gte;
	}
	else {
		ineq_fn = &lte;
	}
	idx *= heap->elemsize;
	upper *= heap->elemsize;
	while (true) {
		// Compute the left and right child nodes.
		left = idx * 2 + heap->elemsize;
		right = left + heap->elemsize;
		// Check to see if we have two descendants.
		if (right <= upper) {
			// Pick index with the larger value.
			if (ineq_fn(&elems[left], &elems[right], heap->cmp_fn) == true) {
				swap_idx = left;
			}
			else {
				swap_idx = right;
			}
		}
		// Check to see if we have at least one descendant.
		else if (left <= upper) {
			swap_idx = left;
		}
		// We have no descendants; nothing to swap; bail
		else {
			break;
		}
		if (ineq_fn(&elems[idx], &elems[swap_idx], heap->cmp_fn) == true) {
			// The value at `idx` is in its correct position in the heap.
			break;
		}
		swap(&elems[idx], &elems[swap_idx], heap->elemsize);
		idx = swap_idx;
	}	
}

static void
sift_up(HEAP *heap, long long idx) {
	/*	For max heaps, move the value at `idx` up in the heap until it is
		smaller than its parent and larger than both of its children. For min
		heaps, we move the value at `idx` up until the opposite is true.
	*/
	char *elems = (char*) heap->elems;
	/*	The inequality value to compare against for determining whether the 
		value at `idx` is less-than or greater-than some other value. This is
		determined by whether we receive a min or max heap. Use -1 for 
		less-than comparisons (min heaps) and 1 for greater-than comparisons
		(min heap).
	*/
	short ineq = (heap->is_max) ? 1 : -1;
	long long parent = ((idx - 1) >> 1) * heap->elemsize;
	idx = idx * heap->elemsize;
	while (idx > 0 && heap->cmp_fn(&elems[idx], &elems[parent]) == ineq) {
		swap(&elems[idx], &elems[parent], heap->elemsize);
		
		idx = parent;
		parent = ((idx / heap->elemsize - 1) >> 1) * heap->elemsize;
	}
}

short
heap_push(HEAP *heap, void *elem_addr) {
	/*	Push the value at `elem_addr` onto the heap. Resize the heap array if 
		necessary.

		Params
		------
		*heap: a HEAP pointer
		*elem_addr: pointer to the base address of the value to be pushed
	*/
	if (heap->size == heap->logsize) {
		short exit_code = heap_double(heap);
		if (exit_code == ALLOC_ERROR) {
			return exit_code;
		}
	}
	void * target = (char*) heap->elems + heap->logsize * heap->elemsize;
	memcpy(target, elem_addr, heap->elemsize);
	++heap->logsize;
	sift_up(heap, heap->logsize - 1);
	return 0;
}

short
heap_pop(HEAP *heap, void *elem_addr) {
	/*	Pop the first element off of the heap and copy it's value to 
		`elem_addr`. The heap is re-heapified after the value has been popped.

		Params
		------
		*heap: a HEAP pointer
		*elem_addr: the base address where the popped value will be copied
	*/
	long long upper = (heap->logsize - 1) * heap->elemsize;
	char *elems = (char*) heap->elems;
	void *source = elems + upper;
	if (heap->logsize >= 2) {
		swap(&elems[0], &elems[upper], heap->elemsize);
		memcpy(elem_addr, source, heap->elemsize);
		--heap->logsize;
		if (heap->logsize > 1) {
			sift_down(heap, 0, heap->logsize - 1);
		}
	}
	else if (heap->logsize == 1) {
		memcpy(elem_addr, source, heap->elemsize);
		--heap->logsize;
	}
	else {
		return EMPTY_ARRAY_ERROR;
	}
	return 0;
}

static void
heapify(HEAP *heap) {
	/*	Heapify the heap array.
		See https://tinyurl.com/34m7tun3 for why this method is optimal.
	*/
	long long last_parent = (heap->logsize - 2) >> 1;
	for (; last_parent >= 0; --last_parent) {
		sift_down(heap, last_parent, heap->logsize - 1);
	}
}

void
heap_dispose(HEAP *heap) {
	/*	Free the heap array and any nested pointers inside the array.

		Params
		------
		*heap: a HEAP pointer
	*/
	if (heap->free_fn) {
		for (int i = 0; i < heap->logsize; ++i) {
			heap->free_fn((char*) heap->elems + i * heap->elemsize);
		}
	}
	free(heap->elems);
}

HEAP
build_heap(
		void *arr,
		long long size,
		long long elemsize,
		bool is_max,
		short (*cmp_fn)(void*, void*),
		void (*free_fn)(void*)) {
	/*	Build a min or max heap.

		Params
		------
		*arr: pointer to the dynamic array to be heapified
		size: the allocated size of the array
		elemsize: the size, in bytes, of the elements of the array
		is_max: whether the heap is a max heap
		cmp_fn: the comparison function for comparing array elements
		free_fn: optional freeing function for nested pointers

		Returns
		-------
		A heap struct containing a heapified dynamic array.
	*/
	HEAP heap;
	heap.elems = arr;
	heap.size = size;
	heap.logsize = size;
	heap.elemsize = elemsize;
	heap.is_max = is_max;
	heap.cmp_fn = cmp_fn;
	heap.free_fn = free_fn;
	heapify(&heap);
	return heap;
}

void
heap_sort(
		void *arr,
		long long size,
		long long elemsize,
		bool asc,
		short (*cmp_fn)(void*, void*)) {
	/*	Sort the array in either ascending or descending order.

		Params
		------
		*arr: the dynamic array to be sorted
		size: the allocated size of the array
		elemsize: the size, in bytes, of the elements of the array
		asc: whether to sort in ascending order
		cmp_fn: the comparison function for comparing array elements
	*/
	char *elems = (char*) arr;
	HEAP heap;
	heap = build_heap(arr, size, elemsize, asc, cmp_fn, NULL);
	long long upper = (heap.logsize - 1) * heap.elemsize;
	for (; upper > 0; upper -= heap.elemsize) {
		swap(&elems[0], &elems[upper], heap.elemsize);
		sift_down(&heap, 0, upper / heap.elemsize - 1);
	}
}

#endif // HEAPS_H
