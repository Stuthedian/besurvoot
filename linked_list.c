#include <stdlib.h>
#include "check.h"
#include "linked_list.h"

void list_add(struct Linked_List* ll_list, char* str)
{
	struct Linked_List_Node* ll_node = malloc(sizeof(struct Linked_List_Node));
    ll_node CHECK_IS_NULL;
    strncpy(ll_node->text, str, STRING_WIDTH);
	
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
    ll_list->count++;
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
	
    ll_list->count = -1;
	ll_list->first = NULL;
	ll_list->last = NULL;
}

char* list_find(const Linked_list_t ll_list, int index)
{
    char* result_str = NULL;
    struct Linked_List_Node* node = ll_list.first;

    if(index < 0)
        goto EXIT;

    while(1)
    {
        if(index > 0)
        {
            node = node->next; 
            if(node == NULL)
                goto EXIT;
            index--;
        }
        else
        {
            result_str = node->text;
            goto EXIT;
        }
    }

EXIT:
    return result_str;
}
