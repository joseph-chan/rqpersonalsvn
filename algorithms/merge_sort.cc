#include <stdio.h>
#include <assert.h>

void Merge(int sort_list[],int begin,int middle,int end,int buf[]){
  assert(middle > begin && end > middle);
  int i=begin,j=middle;
  int buf_index=begin;
  while(i<middle && j<end){
    if(sort_list[i]<= sort_list[j]){
      buf[buf_index]=sort_list[i];
      buf_index++;
      i++;
    }
    else{
      buf[buf_index]=sort_list[j];
      buf_index++;
      j++;
    }
  }
  while(i<middle){
    buf[buf_index]=sort_list[i];
    i++;
  }
  while(j<end){
    buf[buf_index]=sort_list[j];
    j++;
  }
  for(int k=begin;k<end;k++){
    sort_list[k]=buf[k];
  }
  return;
}

void SubMergeSort(int sort_list[],int begin, int end,int buf[]){
  int middle=(begin+end)/2;
  assert(sort_list != NULL && end >= begin && buf != NULL);
  if(end - begin > 2){
    SubMergeSort(sort_list,middle,end,buf);
    SubMergeSort(sort_list,begin,middle,buf);
  }
  if(end - begin >1){
    Merge(sort_list,begin,middle,end,buf);
  }
}

void MergeSort(int sort_list[], int len, int buf[]){
  assert(sort_list != NULL && buf != NULL && len>0);
  SubMergeSort(sort_list,0,len,buf);
}

int main(){
  int a[8]={3,42,3,4,6,2,3,5};
  int b[8];
  MergeSort(a,8,b);
  for(int i=0;i<8;i++){
    printf("%d\t%d\n",a[i],b[i]);
  }

}
