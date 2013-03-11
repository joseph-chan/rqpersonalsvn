#include <stdio.h>
#include <assert.h>
#include <vector>

void Swap(int* a, int* b){
  assert(a!=NULL && b!=NULL);
  int tmp;
  tmp=*a;
  *a=*b;
  *b=tmp;
}

/*
   void PositiveNegative(int sort_list[], int len){
   if(sort_list == NULL || len <= 0 ){
   return;
   }
   std::vector<int> index_buf;
   for(int i=0;i<len;i++){
   if(sort_list[i]<=0){
   index_buf.push_back(i);
   }
   else{
//check if there is negative number in the index_buf
if(index_buf.empty()==false){
Swap(&(sort_list[i]),&(sort_list[index_buf.back()]));
index_buf.pop_back();
index_buf.push_back(i);
}
else continue;
}
}

}
 */
void PositiveNegative(int sort_list[], int len){
  assert(sort_list != NULL && len>0);
  int nega_index=0;//i
  int posi_index=0;//j
  bool flag=false;//true for positive pointer to move, false for  negative to move
  while(nega_index <= len && posi_index<=len){
    if(flag==false){//check negative
      if(sort_list[nega_index]<=0){
        flag=true;
        continue;
      }
      else{
        ++nega_index;
        continue;
      }
    }
    else{//check positive
      if(sort_list[posi_index]>0 and posi_index>nega_index){
        Swap(&(sort_list[posi_index]),&(sort_list[nega_index]));
        flag=false;
        ++posi_index;
        continue;
      }
      else{
        ++posi_index;
        continue;
      }
    }
  }

}

int main(){
  int A[10]={2,3,-5,-1,-3,-5,-1,-6,7,-1};
  PositiveNegative(A,10);
  for(int i=0;i<10;i++){
    printf("%d\n",A[i]);
  }

}
