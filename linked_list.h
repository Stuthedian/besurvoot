#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define STRING_WIDTH 80

struct Linked_List_Node
{
  struct Linked_List_Node* prev;
  struct Linked_List_Node* next;
  char text[STRING_WIDTH];
};

typedef struct Linked_List
{
  struct Linked_List_Node* first;
  struct Linked_List_Node* last;
  int count;
} Linked_List_t;

void list_init(Linked_List_t* ll_list);
void list_destroy(Linked_List_t* ll_list);
void list_add(Linked_List_t* ll_list, char* str);
char* list_get_value(const Linked_List_t* ll_list, int index);
struct Linked_List_Node* list_find(const Linked_List_t* ll_list, int index);
//list_swap(ll_list, idx or node_ptr, bool: prev or next)

#endif //LINKED_LIST_H 
