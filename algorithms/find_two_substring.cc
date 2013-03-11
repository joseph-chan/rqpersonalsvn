#include <stdio.h>
#include <assert.h>

int FindMaxSubstring(const char * target){
  assert(target !=NULL);
  char first, second;
  const char* repeat_index=target;
  const char* p=target;
  int full_flag=0;
  int len=0,max=0;
  while(*p!='\0'){
    if(full_flag == 0){
      first=*p;
      full_flag=1;
      len=1;
    }
    else if(full_flag==1 ){
      if(*p != first){
        second=*p;
        full_flag=2;
      }
      ++len;
    }
    else { // only full_flag == 2
      if(*p == first || *p == second){
        ++len;
      }
      else{
        // the third char turn out
        len= p-repeat_index+1;
        if(*repeat_index == first){
          second = *p;
        }
        else{
          first = *p;
        }
        full_flag=2;//always = 2,not need
      }
    }

    if(max < len){
      max=len;
    }

    //handle repeat_index
    if(*repeat_index != *p){
  //    printf("p:%c\t repeat_index:%c\n",*p,*repeat_index);
      repeat_index = p;
    }

    ++p;

  }
  return max;
}
int main(){
  for(int i=0;i<10000000;i++){
    const char* test = "ababababababababababaaaddddffffffffbdddddddssaaac"; 
    printf("%d\n",FindMaxSubstring(test));
  }
  //no much time to write test cases;
  return 0;

}

