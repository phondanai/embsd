//	set path=C:\MinGW\bin;%path%

#include <stdio.h>
#include <stdlib.h>

#ifdef ENABLE_PRINT
void printArray2(int a[], int l, int r)	/* sum up to n */
{
	int j;
	printf("a[%d:%d] = {", l, r);
	for(j = l; j <= r; j ++){
		if(j > l) printf(", "); /* print comma from 2nd iteration */
		printf("%d", a[j]);
	}
	printf("}\n");
}
void printPivot(int a[], int l, int r, int pivot)
{
	printf("pivot(%d): ", pivot);
	printArray2(a, l, r);
}
#else
#define printArray2(a, b, c)
#define printPivot(a, b, c, d)
#endif

void printArray3(int a[], int l, int r)	/* sum up to n */
{
	int j;
	printf("a[%d:%d] = {", l, r);
	if(r - l < 20){
		for(j = l; j <= r; j ++){
			if(j > l) printf(", "); /* print comma from 2nd iteration */
			printf("%d", a[j]);
		}
	}
	else{
		for(j = l; j < l + 5; j ++){
			if(j > l) printf(", "); /* print comma from 2nd iteration */
			printf("%d", a[j]);
		}
		printf(", ... ");
		for(j = r - 4; j <= r; j ++){
			printf(", %d", a[j]);
		}
	}
	printf("}\n");
}

double swap_count = 0;
void swap2(int * left, int * right)
{
	int t = *left;
	*left = *right;
	*right = t;
	swap_count ++;
}

double comp_count = 0;


int partition1(int a[], int left, int right)
{
	int pivot = a[right];
	int i = left, k = right;
	while(i < k){
		while(a[i] < pivot && i < k) i ++;
		while(a[k] >= pivot && i < k) k --;
		if(i < k) swap2(&a[i], &a[k]);
	}
	if(right > k) swap2(&a[right], &a[k]);
	printPivot(a, left, right, pivot);
	comp_count += right - left - 1;
	return k;
}

void quick_sort(int a[], int left, int right)
{
	if(left < right){
		int pivot_pos = partition1(a, left, right);
		quick_sort(a, left, pivot_pos - 1);
		quick_sort(a, pivot_pos + 1, right);
	}
}

void bubble_sort2(int a[], int n)
{
	int i, j, t = 0;	
	for(i = 0; i < n - 1; i = i + 1){
		int swapped = 0;
		for(j = 1; j < n - i; j = j + 1){
			if(a[j - 1] > a[j]){
				swap2(&a[j - 1], &a[j]); 
				swapped = 1;
			}
		}
		printArray2(a, 0, n - i - 1);
		comp_count += n - i - 1;
	}
}

void init_rand_array(int a[], int n, int rand_seed)
{
	int i;
	for(i = 0; i < n; i ++)
		a[i] = i;
	srand(rand_seed);
	int use_rand2 = (n > RAND_MAX);
#ifndef ENABLE_PRINT
	use_rand2 = 1;
#endif
	for(i = 0; i < 2 * n; i ++){
		int j0, j1;
		if(use_rand2){
			j0 = ((rand() << 15) | rand()) % n;
			j1 = ((rand() << 15) | rand()) % n;
		}
		else{
			j0 = rand() % n;
			j1 = rand() % n;
		}
		swap2(&a[j0], &a[j1]);
	}
	swap_count = 0;
}


#define N 1000000
#define RAND_SEED 1

#include <time.h>
int a[N];

int main(int argc, char * argv[])
{
	int n, rand_seed = 1, sort_type = 1;
	int clk;
	double aa[1];
	printf("aa = %f\n", aa[0]);
	if(argc < 2 || argc > 4){
		printf("Invalid command!\n");
		printf("usage: %s <array_size> <rand_seed> <sort-type>\n", argv[0]);
		printf("<rand_seed>, <sort-type> : optional\n");
		printf("sort-type = 0:bubble-sort, 1:quick-sort\n");
		return 0;
	}
	n = atoi(argv[1]);
	if(n <= 0 || n > N){
		printf("Invalid array size!\n");
		printf("Array size must be between %d and %d\n", 1, N);
		return 0;
	}
	if(argc >= 3){
		sscanf(argv[2], "%d", &sort_type);
	}
	if(argc >= 4){
		sscanf(argv[3], "%d", &rand_seed);
	}
	if(sort_type < 0 || sort_type > 1){
		printf("Invalid sort type!\n");
		printf("sort-type = 0:bubble-sort, 1:quick-sort\n");
		return 0;
	}
	init_rand_array(a, n, rand_seed);
	printArray3(a, 0, n - 1);
	clk = clock();
	switch(sort_type){
		case 0:
			bubble_sort2(a, n);
			break;
		case 1:
			quick_sort(a, 0, n - 1);
			break;
	}
	clk = clock() - clk;
	printArray3(a, 0, n - 1);
	printf("comp_count = %.f\nswap_count = %.f\n", comp_count, swap_count);
	printf("elapse time = %.3f sec\n", (double) clk / (double) CLOCKS_PER_SEC);

	return 0;
}
