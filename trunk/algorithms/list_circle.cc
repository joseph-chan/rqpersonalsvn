#include <stdio.h>
#include <assert.h>

struct ListNode{
  ListNode* next;
  int value_;
  ListNode():next(NULL),value_(){
  }
  ListNode(int value):next(NULL),value_(value){
  }
};

bool ListCircle(ListNode* root){
  if(root==NULL || root->next == NULL){
    return false;
  }

  ListNode* fast_p=root;
  ListNode* slow_p=root;
  
  while(true){
    if(fast_p->next == NULL || fast_p->next->next == NULL){//
      return false;
    }
    fast_p=fast_p->next->next;
    slow_p=slow_p->next;
    if(fast_p == slow_p){
      return true;
    } 
  }

}

int main(){
  ListNode *root,*node1,*node2,*node3,*node4,*node5;

  root = new ListNode(0);
  node1 = new ListNode(1);
  node2 = new ListNode(2);
  node3 = new ListNode(3);
  node4 = new ListNode(4);
  node5 = new ListNode(5);
  root->next=node1;
  node1->next=node2;
  node2->next=node1;
  node3->next=node4;
  node4->next=node5;
  printf("%d\n",ListCircle(node1)) ;
  return 0;
}
