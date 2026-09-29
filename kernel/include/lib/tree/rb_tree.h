#ifndef LIB_TREE_RB_TREE_H
#define LIB_TREE_RB_TREE_H

#include <stdint.h>

typedef enum rb_node_color_e {
  RB_BLACK = 0,
  RB_RED,
} rb_node_color_t;

typedef int (*rb_tree_cmp_t)(const void* a, const void* b);

typedef struct rb_tree_node_s {
  struct rb_tree_node_s* left;
  struct rb_tree_node_s* right;
  struct rb_tree_node_s* parent;
  rb_node_color_t color;

  void* data;
} rb_tree_node_t;

typedef struct rb_tree_s {
  rb_tree_node_t* root;
  rb_tree_node_t* NIL;

  rb_tree_cmp_t cmp;

  rb_tree_node_t __NIL;
} rb_tree_t;

extern int rb_tree_init(rb_tree_t* tree, rb_tree_cmp_t cmp);
extern int rb_tree_insert(rb_tree_t* tree, rb_tree_node_t* node);
extern rb_tree_node_t* rb_tree_find(const rb_tree_t* tree, const void* data);
extern rb_tree_node_t* rb_tree_successor(const rb_tree_t* tree, rb_tree_node_t* node);
extern int rb_tree_remove(rb_tree_t* tree, rb_tree_node_t* node);
extern rb_tree_node_t* rb_tree_minimum(const rb_tree_t* tree, rb_tree_node_t* node);
extern rb_tree_node_t* rb_tree_maximum(const rb_tree_t* tree, rb_tree_node_t* node);

#endif
