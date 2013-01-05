#include <stdio.h>
#include <assert.h>
#include <iostream>

struct TreeNode{
  TreeNode* left;
  TreeNode* right;
  int value;
  TreeNode():left(NULL),right(NULL),value(0){
  }
};

class BSTree{
 public:
  BSTree():node_num_(0),root_(NULL){
  }
  ~BSTree(){//to be added
  }
  void Insert(int value);
  TreeNode* Search(int value);
  int Delete(int value);
  void Print();
 private:
  void SubInsert(TreeNode* sub_root, int value);
  TreeNode* SubSearch(TreeNode* sub_root,int value);
  TreeNode* SubSearch(TreeNode* sub_root,int value,TreeNode** father);
  void Print(TreeNode* sub_root);

  int node_num_;
  TreeNode* root_;
};

void BSTree::Print(){
  Print(root_);
}

void BSTree::Print(TreeNode* sub_root){
  if(sub_root == NULL){
    return;
  }
  if(sub_root->left!=NULL) Print(sub_root->left);
  printf("%d\n",sub_root->value);
  if(sub_root->right!=NULL) Print(sub_root->right);
  return;
}

void BSTree::Insert(int value){
  if(root_==NULL){
    root_=new TreeNode();
    root_->value=value;
    return;
  }
  SubInsert(root_,value);
}

void BSTree::SubInsert(TreeNode* sub_root, int value){
  assert(sub_root!=NULL);
  if(value < sub_root->value){
    if(sub_root->left!=NULL){
      SubInsert(sub_root->left,value);
    }
    else{
      sub_root->left = new TreeNode();
      sub_root->left->value=value;
      return; 
    }
  }
  else {
      if(sub_root->right!=NULL){
      SubInsert(sub_root->right,value);
    }
    else{
      sub_root->right = new TreeNode();
      sub_root->right->value=value;
      return; 
    }
  }
}

TreeNode* BSTree::Search(int value){
  return SubSearch(root_,value);
}

TreeNode* BSTree::SubSearch(TreeNode* sub_root,int value){
  if(sub_root == NULL){
    return NULL;
  }
  if(value == sub_root->value){
    return sub_root;
  }
  else if(value < sub_root->value){
    return SubSearch(sub_root->left,value);
  }
  else {
    return SubSearch(sub_root->right,value);
  }
}

TreeNode* BSTree::SubSearch(TreeNode* sub_root,int value,TreeNode** father){
  //be true only once if root_  matches
  if(sub_root==root_){
    *father=NULL;
  }

  //not find
  if(sub_root == NULL){
    return NULL;
  }
  if(value == sub_root->value){
    return sub_root;
  }
  else if(value < sub_root->value){
    *father = sub_root;
    return SubSearch(sub_root->left,value,father);
  }
  else {
    *father = sub_root;
    return SubSearch(sub_root->right,value,father);
  }
}

int BSTree::Delete(int value){
  if(root_ == NULL){
    return 1;
  }

  TreeNode* father;
  TreeNode* p;
  TreeNode* tmp_node;
  p=SubSearch(root_,value,&father);
  if(p==NULL){
    return 1;
  }
  if(p->left == NULL && p->right == NULL){
    if(father == NULL){
      //only one node: p == root_
      delete p;
      p=NULL;
      return 0;
    }
    
    if(p->value < father->value) father->left = NULL;
    else father->right =NULL;
    delete p;
    return 0;
  
  }
  else if(p->left == NULL){
    if(father == NULL){
      //p==root_
      tmp_node = p;
      p=p->right;
      delete tmp_node;
      return 0;
    }
    
    if(p->value < father->value) father->left = p->right;
    else  father->right = p->right;         
  }
  else if(p->right == NULL){
    if(father == NULL){
      //p==root_
      tmp_node = p;
      p=p->left;
      delete tmp_node;
      return 0;
    }
    
    if(p->value < father->value) father->left = p->left;
    else  father->right = p->left;  
  }
  else {
    //do not need consider father this time
    TreeNode* replace_node = p->left;
    //tmp_node used as replace_node 's father, because this is just a value replacement
    tmp_node = p;
    while(replace_node->right != NULL){
      tmp_node = replace_node;
      replace_node = replace_node ->right;
    } // replace_node found
    p->value = replace_node->value;
    if(tmp_node == p) {
      //the while has not ran, because is no right node in p->left
      //so replace_node is p->left
      p->left = replace_node->left;
    }
    else tmp_node->right=NULL;
    delete replace_node;
  }
    return 0;

}

int main(){
  BSTree binary_search_tree;
  int a[100]={61,82,67,63,71,73,82,76,80,90,56,77,32,39,66,41,
  28,35,51,4,93,73,81,96,12,99,46,94,22,84,82,45,15,91,36,80,22,
33,83,2,47,48,36,7,20,89,45,70,13,66,91,87,0,16,47,65,62,15,
69,48,72,84,77,35,80,0,84,81,83,60,8,80,43,70,68,71,86,78,2,
24,65,55,31,9,67,71,20,95,8,10,49,80,63,35,16,88,98,72,7,15};
  for(int i=0;i<100;i++){
    binary_search_tree.Insert(a[i]);
  }
  std::cout << (binary_search_tree.Search(35)) << std::endl;
  binary_search_tree.Delete(35);
  std::cout << (binary_search_tree.Search(35)) << std::endl;
  binary_search_tree.Delete(35);
  std::cout << (binary_search_tree.Search(35)) << std::endl;
  binary_search_tree.Delete(35);
  std::cout << (binary_search_tree.Search(35)) << std::endl;
  
  std::cout<<std::endl;
  binary_search_tree.Print();
  return 0;
}

