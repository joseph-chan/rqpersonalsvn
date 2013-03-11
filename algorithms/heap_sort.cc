#include <assert.h>
#include <stdio.h>
#include <math.h>

int Left(int i)  {
  return 2*i+1;
}
int Right(int i)  {
  return 2*i+2;
}
void Swap(int* a,int* b) {
  int temp=*a;
  *a=*b;
  *b=temp;
}

void MinHeapfy(int heap_array[],int heap_size,int i) {
  assert(heap_array!=NULL);
  //a mistake here : min=0 ; X!
  int min=i;
  int l=Left(i);
  int r=Right(i);
  if(l<heap_size && heap_array[l] < heap_array[i]) {
    min = l;
  }
  //a mistake here: <min;  min is a index, heap_array[min] is a
  //value!!
  if(r<heap_size && heap_array[r] < heap_array[min]) {
    min = r;
  }
  if(min != i) {
    //a mistake: forget the &
    Swap(&(heap_array[min]),&(heap_array[i]));

    MinHeapfy(heap_array,heap_size,min);
  }
}

void BuildHeap(int heap_array[], int heap_size) {
  assert(heap_array!=NULL);
  for(int i=heap_size/2; i>=0;i--) {
    //a mistake: last param set as 0!
    MinHeapfy(heap_array,heap_size,i);
  }
}

void HeapSort(int heap_array[],int heap_size) {
  assert(heap_array!=NULL);
  BuildHeap(heap_array,heap_size);
  for(int i=0;i<heap_size-1;i++) {
    Swap(&heap_array[0],&heap_array[heap_size-i-1]);
    //a mistake: last param set as i
    MinHeapfy(heap_array,heap_size-i-1,0);
  }
}

int main() {
  int A[15]= {4,7,1,8,4,8,0,4,22,56,55,3,53,11,91};
  HeapSort(A,15);
  for(int i=0;i<15;i++) {
    printf("%d\n",A[i]);
  }
}
