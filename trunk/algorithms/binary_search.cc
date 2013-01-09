#include <stdio.h>
#include <assert.h>

int CheckPrev(int A[],int index){
  assert(A != NULL);
  int i=index;
  while(A[i] == A[index] && i>=0){
    --i;
  }
  return i+1;
}

int BinarySearch(int search_list[], int len, int target){
  if (search_list == NULL || len<=0){
    return -1;
  }
  
  int begin=0;
  int end=len;
  int middle=0;
  while(end > begin) {
    if(end - begin == 1){
      if(search_list[begin] == target) return begin;
      else return -1;
    }
    middle = (begin + end)/2;
    if(search_list[middle] == target){
      return CheckPrev(search_list,middle);
    }
    else if(target < search_list[middle]){
      end=middle;
      continue;
    }
    else{
      begin=middle;
      continue;
    }

  }
  return -1;
}

int main(){
  //int A[11]={2,6,8,9,11,11,23,24,66,122,123};
  int A[11]={2,6,8,9,11,11,11,11,11,11,123};
  printf("%d\n",BinarySearch(A,11,123));
  printf("%d\n",BinarySearch(A,11,122));
  printf("%d\n",BinarySearch(A,11,66));
  printf("%d\n",BinarySearch(A,11,24));
  printf("%d\n",BinarySearch(A,11,23));
  printf("%d\n",BinarySearch(A,11,13));
  printf("%d\n",BinarySearch(A,11,11));
  printf("%d\n",BinarySearch(A,11,9));
  printf("%d\n",BinarySearch(A,11,8));
  printf("%d\n",BinarySearch(A,11,6));
  printf("%d\n",BinarySearch(A,11,2));
  printf("%d\n",BinarySearch(A,11,4));
}
