#ifndef HEAPS_H_
#define HEAPS_H_

// Header file for building generic min and max heaps.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define true 1
#define false 0
#define bool short

typedef struct {
	void* arr;			    // Pointer to the array to be heapified.
	long long size;			    // The size of the array.
	long long end;			    // The last index position of the array.
	long long tsize;		    // The size of the data types in the array.
	bool is_max;			    // Indicate whether the heap is a max heap.
	short (*cmp)(void*, void*);	    // The comparison function for the data
} HEAP;					    // types stored in the array.

void
swap(void* vp1, void* vp2, long long tsize) {
	/*	
	Swap the bit patterns of the two void pointers. Number of bytes to swap 
	is determined by `tsize`.
	*/
	unsigned char *x = vp1, *y = vp2, temp;
	for (long long i = 0; i < tsize; ++i) {
		temp = x[i];
		x[i] = y[i];
		y[i] = temp;	
	}
}

long long
double_size(void *vp, long long size, long long tsize) {
	/*	
	Double the size of the allocated space for the pointer and return the 
	new size.
	*/
	long long new_size = size * 2;
	char* temp = (char*) realloc(vp, new_size * tsize);
	if (temp != NULL) {
		vp = temp;	
	}
	else {
		fprintf(stderr, "Unable to allocate sufficient memory for array!\n");
		exit(1);
	}
	return new_size;
}

short
cmpstr(void* vp1, void* vp2) {
	/*
	Compare two strings. Returns:
		-1: if the value at `vp1` is less than the value at `vp2`
		 0: if the values are equal
		 1: if the value at `vp1` is greater than the value at `vp2`
	*/
	char* ps1 = *(char**) vp1;
	char* ps2 = *(char**) vp2;
	int res = strcmp(ps1, ps2);
	if (res < 0) { 
		return -1; 
	}
	else if (res == 0) { 
		return res; 
	}
	else {
		return 1;
	}
}

short
cmpint(void* vp1, void* vp2) {
	/*
	Compare two integers. Returns:
		-1: if the value at `vp1` is less than the value at `vp2`
		 0: if the values are equal
		 1: if the value at `vp1` is greater than the value at `vp2`
	*/
	int* pi1 = (int*) vp1;
	int* pi2 = (int*) vp2;
	if (*pi1 < *pi2) {
		return -1;
	}
	else if (*pi1 == *pi2) {
		return 0;	
	}
	else {
		return 1;	
	}
}

short
cmpfloat(void* vp1, void* vp2) {
	/*
	Compare two floats. Returns:
		-1: if the value at `vp1` is less than the value at `vp2`
		 0: if the values are equal
		 1: if the value at `vp1` is greater than the value at `vp2`
	*/
	float* vf1 = (float*) vp1;
	float* vf2 = (float*) vp2;
	if (fabsf(*vf1 - *vf2) < 1e-10) {
		return 0;
	}
	else if (vf1 < vf2) {
		return -1;
	}
	else { 
		return 1; 
	}
}

const short const (*CMPSTR)(void*, void*) = &cmpstr;
const short const (*CMPINT)(void*, void*) = &cmpint;
const short const (*CMPFLOAT)(void*, void*) = &cmpfloat;

void
sift_down(HEAP heap, long long idx, long long end) {
	/*
	For max heaps, move the value at `idx` down in the heap until it is smaller 
	than its parent and larger than both of its children. For min heaps we 
	move the value at `idx` down until the opposite is true.
	*/
	char* pa = (char*) heap.arr;
	long long left, right, to_swap;
	long long plus_two = 2 * heap.tsize;
	/*	
	The value to compare against for determining whether the value at `idx` is 
        less-than or greater-than some other value. This is determined by whether 
	or not we receive a max heap or a min heap. Use -1 for less-than 
	comparisons (max heap) and 1 for greater-than comparisons (min heap).
	*/
	short cmp_value = (heap.is_max == true) ? 1 : -1;
	idx = idx * heap.tsize;
	end = end * heap.tsize;
	while (true) {
		left = (idx * 2 + heap.tsize); // The left child
		right = (idx * 2 + plus_two);  // The right child
		// First check to see if `idx` has two direct children.		
		if (right <= end) {
			if (
					heap.cmp(&pa[left], &pa[right]) == 0 ||
					heap.cmp(&pa[left], &pa[right]) == cmp_value) {
				to_swap = left;					
			}
			else {
				to_swap = right;			
			}
		
			if (
					heap.cmp(&pa[idx], &pa[to_swap]) == 0 ||
					heap.cmp(&pa[idx], &pa[to_swap]) == cmp_value) {
				break;			
			}
			swap(&pa[idx], &pa[to_swap], heap.tsize);
			idx = to_swap;
		}
		/* 
		If `idx` doesn't have two children, check to see if it has at least one.
		*/
		else if (left <= end) {
			if (
					heap.cmp(&pa[idx], &pa[left]) == 0 ||
					heap.cmp(&pa[idx], &pa[left]) == cmp_value) {
				break;			
			}
			swap(&pa[idx], &pa[left], heap.tsize);
			idx = left;
		}
		// If there are no children then we bail; there's nothing to swap.
		else { break; }
	}
}

void
sift_up(HEAP heap, long long idx) {
	/*
	For max heaps, move the value at `idx` up in the heap until it is smaller 
	than its parent and larger than both of its children. For min heaps we 
	move the value at `idx` up until the opposite is true.
	*/
	long long parent = (idx - 1) / 2 * heap.tsize;
	char* pa = (char*) heap.arr;
	/*	
	The value to compare against for determining whether the value at `idx` is 
        less-than or greater-than some other value. This is determined by whether 
	or not we receive a max heap or a min heap. Use -1 for less-than 
	comparisons (max heap) and 1 for greater-than comparisons (min heap).
	*/
	short cmp_value = (heap.is_max == true) ? 1 : -1;
	idx = idx * heap.tsize;
	while (idx > 0 && heap.cmp(&pa[idx], &pa[parent]) == cmp_value) {
		swap(&pa[idx], &pa[parent], heap.tsize);
		idx = parent;
		parent = (idx / heap.tsize - 1) / 2 * heap.tsize;
	}
}

void*
pop(HEAP* heap) {
	// Pop the last element off of the heap and return a pointer to that value.
	static void* res;
	char* pa = (char*) heap->arr;
	long long end = heap->end * heap->tsize;
	if (heap->end >= 1) {
		swap(&pa[0], &pa[end], heap->tsize);
		res = (int*) &pa[end];
		heap->end = heap->end - 1;
		if (heap->end > 1) {
			sift_down(*heap, 0, heap->end);		
		}	
	}
	else if (heap->end == 0) {
		res = (int*) &pa[end];
		printf("%d\n", *(int*)res);
		heap->end = heap->end - 1;
	}
	else {
		fprintf(stderr, "Array empty or array index out of bounds!\n");
		exit(1);
	}
	return res;
	
}

void
push(HEAP* heap, void* vp) {
	/*
	Push the value at `vp` onto the heap. Resize the array if necessary.
	*/
	char* pa = (char*) heap->arr;
	if (heap->end == heap->size - 1) {
		/*
		Resize the array and set `heap.size` to the new size returned by the 
		call to `double_size`.
		*/
		heap->size = double_size(heap->arr, heap->size, heap->end);
	}
	// Increment `heap.end` to account for the new value we're pushing.
	heap->end = heap->end + 1;
	memcpy(&pa[heap->end * heap->tsize], value, heap->tsize);
	/*
	The value is likely not located at the correct index position, sift it up 
	in the heap.
	*/
	sift_up(*heap, heap->end);
}

HEAP
heapify(HEAP heap) {
	long long last_parent = (heap.end - 1) / 2;
	for (long long i = last_parent; i >= 0; --i) {
		sift_down(heap, i, heap.end);
	}
	return heap;
}

HEAP
build_heap(
		void* arr, 
		long long size, 
		long long tsize,
		bool is_max, 
		short (*cmp_fn)(void*, void*)) {
	/*
	Build and return either a min or max heap out of the supplied array (`arr`).
	*/
	HEAP heap;
	heap.arr = arr;
	heap.size = size;
	heap.end = size - 1;
	heap.tsize = tsize;
	heap.is_max = is_max;
	heap.cmp = cmp_fn;
	heap = heapify(heap);
	return heap;
}

void
heap_sort(
		void* arr,
		long long size,
		long long tsize,
		short (*cmp_fn)(void*, void*),
		bool reverse) {
	/*
	Sort the array in either ascending (`reverse`=0) or descending
	(`reverse`=1) order.
	*/
	char* pa = (char*) arr;
	HEAP heap;
	// Build the appropriate heap based on the direction of the sort.
	if (reverse == true) {
		heap = build_heap(arr, size, tsize, 0, cmp_fn);
	}
	else {
		heap = build_heap(arr, size, tsize, 1, cmp_fn);
	}
	long long upper = heap.end * heap.tsize;
	for (upper; upper >= 0; upper = upper - heap.tsize) {
		swap(&pa[0], &pa[upper], heap.tsize);
		sift_down(heap, 0, upper / heap.tsize - 1);
	}
}

#endif // HEAPS_H_
