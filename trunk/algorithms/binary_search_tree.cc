

struct TreeNode{
  TreeNode* left;
  TreeNode* right;
  int value;
  TreeNode():left(NULL),right(NULL),value(0){
  }
}

class BSTree{
 public:
  BSTree():node_num_(0),root(NULL){
  }
  ~BSTree(){//to be added
  }
  void Insert(int value);
  TreeNode* Search(int value);
  int Delete(int value);
 private:
  void SubInsert(TreeNode* sub_root, int value);
  TreeNode SubSearch(TreeNode* sub_root,int value);

  int node_num_;
  TreeNode* root_;
}

void BSTree::Insert(int value){
  if(root_==NULL){
    root_=new TreeNode();
    root_->value=value;
    return;
  }
  SubInsert(root,value);
}

void BSTree::SubInsert(TreeNode* sub_root, int value){
  assert(sub_root!=NULL);
  if(value < sub_root->value){
    if(sub_root->left!=NULL){
      SubInsert(subroot->left,value);
    }
    else{
      sub_root->left = new TreeNode();
      sub_root->left->value=value;
      return; 
    }
  else {
      if(sub_root->right!=NULL){
      SubInsert(subroot->right,value);
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
    return SubSearch(sub_root->left,value)
  }
  else {
    return SubSearch(sub_root->right,value);
  }
}

TreeNode* BSTree::SubSearch(TreeNode* sub_root,int value,TreeNode** father){
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
    return SubSearch(sub_root->left,value,father)
  }
  else {
    return SubSearch(sub_root->right,value,father);
  }
}

int BSTree::Delete(int value){
  if(node)
}
