#include <lib/tree/rb_tree.h>

#include <stddef.h>

static void rb_rotate_left(rb_tree_t* tree, rb_tree_node_t* node);
static void rb_rotate_right(rb_tree_t* tree, rb_tree_node_t* node);

static void rb_fix_insert(rb_tree_t* tree, rb_tree_node_t* node);
static void rb_fix_remove(rb_tree_t* tree, rb_tree_node_t* node);
static void rb_transplant(rb_tree_t* tree, rb_tree_node_t* old_node, rb_tree_node_t* new_node);
static int rb_tree_contains_node(const rb_tree_t* tree, const rb_tree_node_t* node);

int rb_tree_init(rb_tree_t* tree, rb_tree_cmp_t cmp)
{
  if(!tree || !cmp)
    return -1;

  tree->NIL = &tree->__NIL;
  tree->NIL->right = tree->NIL->left = tree->NIL;
  tree->NIL->parent = tree->NIL;
  tree->NIL->color = RB_BLACK;
  tree->NIL->data = NULL;

  tree->root = tree->NIL;

  tree->cmp = cmp;

  return 0;
}

int rb_tree_insert(rb_tree_t* tree, rb_tree_node_t* node)
{
  if(!tree || !node || !tree->cmp)
    return -1;

  rb_tree_node_t* parent = NULL;
  rb_tree_node_t* current = tree->root;

  node->left = node->right = tree->NIL;
  node->color = RB_RED;

  while(current != tree->NIL)
  {
    parent = current;
    if (tree->cmp(node->data, current->data) < 0) 
    {
      current = current->left;
    }
    else
    {
      current = current->right;
    }
  }

  node->parent = parent;

  if (parent == NULL) 
  {
    tree->root = node;
  }
  else if (tree->cmp(node->data, parent->data) < 0)
  {
    parent->left = node;
  }
  else
  {
    parent->right = node;
  }
  rb_fix_insert(tree, node);

  return 0;
}

rb_tree_node_t* rb_tree_find(const rb_tree_t* tree, const void* data)
{
  if (!tree || !tree->cmp || !tree->NIL)
    return NULL;

  rb_tree_node_t* current = tree->root;
  while (current != tree->NIL)
  {
    int result = tree->cmp(data, current->data);
    if (result == 0)
      return current;
    current = result < 0 ? current->left : current->right;
  }

  return NULL;
}

int rb_tree_remove(rb_tree_t* tree, rb_tree_node_t* node)
{
  if (!tree || !node || !tree->cmp || !tree->NIL || !rb_tree_contains_node(tree, node))
    return -1;

  rb_tree_node_t* moved_node = node;
  rb_node_color_t moved_color = moved_node->color;
  rb_tree_node_t* replacement;

  if (node->left == tree->NIL)
  {
    replacement = node->right;
    rb_transplant(tree, node, node->right);
  }
  else if (node->right == tree->NIL)
  {
    replacement = node->left;
    rb_transplant(tree, node, node->left);
  }
  else
  {
    moved_node = rb_tree_minimum(tree, node->right);
    moved_color = moved_node->color;
    replacement = moved_node->right;

    if (moved_node->parent == node)
    {
      replacement->parent = moved_node;
    }
    else
    {
      rb_transplant(tree, moved_node, moved_node->right);
      moved_node->right = node->right;
      moved_node->right->parent = moved_node;
    }

    rb_transplant(tree, node, moved_node);
    moved_node->left = node->left;
    moved_node->left->parent = moved_node;
    moved_node->color = node->color;
  }

  if (moved_color == RB_BLACK)
    rb_fix_remove(tree, replacement);

  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->color = RB_BLACK;
  tree->NIL->parent = tree->NIL->parent;
  return 0;
}

rb_tree_node_t* rb_tree_successor(const rb_tree_t* tree, rb_tree_node_t* node)
{
  if (!tree || !tree->NIL || !node || !rb_tree_contains_node(tree, node))
    return NULL;

  if (node->right != tree->NIL)
  {
    node = node->right;
    while (node->left != tree->NIL)
      node = node->left;
    return node;
  }

  rb_tree_node_t* parent = node->parent;
  while (parent && node == parent->right)
  {
    node = parent;
    parent = parent->parent;
  }

  return parent;
}

rb_tree_node_t* rb_tree_minimum(const rb_tree_t* tree, rb_tree_node_t* node)
{
  while (node->left != tree->NIL)
    node = node->left;

  return node;
}

rb_tree_node_t* rb_tree_maximum(const rb_tree_t* tree, rb_tree_node_t* node)
{
  while (node->right != tree->NIL)
    node = node->right;

  return node;
}

static int rb_tree_contains_node(const rb_tree_t* tree, const rb_tree_node_t* node)
{
  if (node == tree->NIL)
    return 0;

  while (node->parent)
  {
    const rb_tree_node_t* parent = node->parent;
    if (parent->left != node && parent->right != node)
      return 0;
    node = parent;
  }

  return node == tree->root;
}

static void rb_transplant(rb_tree_t* tree, rb_tree_node_t* old_node, rb_tree_node_t* new_node)
{
  if (!old_node->parent)
    tree->root = new_node;
  else if (old_node == old_node->parent->left)
    old_node->parent->left = new_node;
  else
    old_node->parent->right = new_node;

  new_node->parent = old_node->parent;
}

static void rb_fix_remove(rb_tree_t* tree, rb_tree_node_t* node)
{
  while (node != tree->root && node->color == RB_BLACK)
  {
    if (node == node->parent->left)
    {
      rb_tree_node_t* sibling = node->parent->right;

      if (sibling->color == RB_RED)
      {
        sibling->color = RB_BLACK;
        node->parent->color = RB_RED;
        rb_rotate_left(tree, node->parent);
        sibling = node->parent->right;
      }

      if (sibling->left->color == RB_BLACK && sibling->right->color == RB_BLACK)
      {
        sibling->color = RB_RED;
        node = node->parent;
      }
      else
      {
        if (sibling->right->color == RB_BLACK)
        {
          sibling->left->color = RB_BLACK;
          sibling->color = RB_RED;
          rb_rotate_right(tree, sibling);
          sibling = node->parent->right;
        }

        sibling->color = node->parent->color;
        node->parent->color = RB_BLACK;
        sibling->right->color = RB_BLACK;
        rb_rotate_left(tree, node->parent);
        node = tree->root;
      }
    }
    else
    {
      rb_tree_node_t* sibling = node->parent->left;

      if (sibling->color == RB_RED)
      {
        sibling->color = RB_BLACK;
        node->parent->color = RB_RED;
        rb_rotate_right(tree, node->parent);
        sibling = node->parent->left;
      }

      if (sibling->right->color == RB_BLACK && sibling->left->color == RB_BLACK)
      {
        sibling->color = RB_RED;
        node = node->parent;
      }
      else
      {
        if (sibling->left->color == RB_BLACK)
        {
          sibling->right->color = RB_BLACK;
          sibling->color = RB_RED;
          rb_rotate_left(tree, sibling);
          sibling = node->parent->left;
        }

        sibling->color = node->parent->color;
        node->parent->color = RB_BLACK;
        sibling->left->color = RB_BLACK;
        rb_rotate_right(tree, node->parent);
        node = tree->root;
      }
    }
  }

  node->color = RB_BLACK;
}

static void rb_fix_insert(rb_tree_t* tree, rb_tree_node_t* node)
{
  while (node != tree->root && node->parent->color == RB_RED) 
  {
    if (node->parent == node->parent->parent->left) 
    {
      rb_tree_node_t* uncle = node->parent->parent->right;

      if (uncle->color == RB_RED) 
      {            
        node->parent->color = RB_BLACK;
        uncle->color = RB_BLACK;
        node->parent->parent->color = RB_RED;
        node = node->parent->parent;
      } 
      else 
      {
        if (node == node->parent->right) 
        {       
          node = node->parent;
          rb_rotate_left(tree, node);
        }
        node->parent->color = RB_BLACK;        
        node->parent->parent->color = RB_RED;
        rb_rotate_right(tree, node->parent->parent);
      }
    }
    else 
    {                                   
      rb_tree_node_t* uncle = node->parent->parent->left;

      if (uncle->color == RB_RED) 
      {
        node->parent->color = RB_BLACK;
        uncle->color = RB_BLACK;
        node->parent->parent->color = RB_RED;
        node = node->parent->parent;
      } 
      else 
      {
        if (node == node->parent->left)
        {
          node = node->parent;
          rb_rotate_right(tree, node);
        }
        node->parent->color = RB_BLACK;
        node->parent->parent->color = RB_RED;
        rb_rotate_left(tree, node->parent->parent);
      }
    }
  }
  tree->root->color = RB_BLACK;
}

static void rb_rotate_left(rb_tree_t* tree, rb_tree_node_t* node)
{
  rb_tree_node_t* y = node->right;
  node->right = y->left;

  if (y->left != tree->NIL) 
  {
    y->left->parent = node;
  }

  y->parent = node->parent;

  if (node->parent == NULL)
  { 
    tree->root = y;
  }
  else if (node == node->parent->left) 
  {
    node->parent->left = y;
  }
  else                          
  {
    node->parent->right = y;
  }

  y->left = node;
  node->parent = y;
}

static void rb_rotate_right(rb_tree_t* tree, rb_tree_node_t* node)
{
  rb_tree_node_t* y = node->left;
  node->left = y->right;

  if (y->right != tree->NIL) 
  {
    y->right->parent = node;
  }

  y->parent = node->parent;

  if (node->parent == NULL)
  {
    tree->root = y;
  }
  else if (node == node->parent->right) 
  {
    node->parent->right = y;
  }
  else                           
  {
    node->parent->left = y;
  }

  y->right = node;
  node->parent = y;
}
