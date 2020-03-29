#include <stdlib.h>
#include "check.h"
#include "linked_list.h"

void list_add(struct Linked_List* ll_list, char* str)
{
	struct Linked_List_Node* ll_node = malloc(sizeof(struct Linked_List_Node));
    ll_node CHECK_IS_NULL;
    memcpy(ll_node->text, str, STRING_WIDTH);
	
	if(ll_list->first == NULL && ll_list->last == NULL)
	{
		ll_list->first = ll_node;
		ll_list->last = ll_node;
		ll_node->prev = NULL;
		ll_node->next = NULL;
	}
	else
	{
		ll_list->last->next = ll_node;
		ll_node->prev = ll_list->last;
		ll_list->last = ll_node;
		ll_node->next = NULL;
	}
}

void list_free(struct Linked_List* ll_list)
{
	struct Linked_List_Node* next_to_free = NULL;
	while(ll_list->last != NULL)
	{
		next_to_free = ll_list->last->prev;
		free(ll_list->last);
		ll_list->last = next_to_free;
	}
	
	ll_list->first = NULL;
	ll_list->last = NULL;
}
