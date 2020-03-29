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
} Linked_list_t;

void list_add(Linked_list_t* ll_list, char* str);
void list_free(Linked_list_t* ll_list);
//list_find(ll_list, idx) -recursive func
//list_swap(ll_list, idx or node_ptr, bool: prev or next)

#endif //LINKED_LIST_H 
