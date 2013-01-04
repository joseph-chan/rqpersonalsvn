#include <assert.h>
#include <stdio.h>

void Swap(int* a, int *b){
  assert(a!=NULL && b!=NULL);
  int temp=*a;
  *a=*b;
  *b=temp;
}

int qsort(int sort_array[], int array_size){
  assert(sort_array!=NULL);
  if(array_size<=1){
    return 0;
  }
  int middle_index=0;
  int middle=sort_array[middle_index];
  for(int i=0; i<array_size;i++){
    if(sort_array[i]<middle) {
      Swap(&sort_array[i],&sort_array[middle_index]);
      ++middle_index;
    }
  }
  //a mistake:be careful to this if;
  if(middle_index==0){
    ++middle_index;
  }
  qsort(sort_array,middle_index);
  qsort(sort_array+middle_index,array_size-middle_index);

  return 0;
}

int main(){
  int A[15]={4,7,1,8,4,8,0,4,22,56,55,3,53,11,91};
  qsort(A,15);
  for(int i=0;i<15;i++){
    printf("%d\n",A[i]);
  }
}
