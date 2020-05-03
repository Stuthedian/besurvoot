#include <stdlib.h>
#include "linked_list.h"
#include "baht.h"

void list_init(Linked_List_t* ll_list)
{
  ll_list->first = NULL;
  ll_list->last = NULL;
  ll_list->count = 0;
}

void list_add(Linked_List_t* ll_list, char* str)
{
  struct Linked_List_Node* ll_node = malloc(sizeof(
                                       struct Linked_List_Node));
  ll_node BAHT_IS_NULL_ERRNO;
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

void list_del(Linked_List_t* ll_list, int index)
{
  struct Linked_List_Node* node = list_find(ll_list, index);

  if(node == NULL)
    return;

  if(node == ll_list->first)
    ll_list->first = node->next;

  if(node == ll_list->last)
    ll_list->last = node->prev;

  if(node->prev != NULL)
    node->prev->next = node->next;

  if(node->next != NULL)
    node->next->prev = node->prev;

  free(node);
  ll_list->count--;
}

void list_destroy(Linked_List_t* ll_list)
{
  struct Linked_List_Node* next_to_free = NULL;

  while(ll_list->last != NULL)
  {
    next_to_free = ll_list->last->prev;
    free(ll_list->last);
    ll_list->last = next_to_free;
  }

  ll_list->count = 0;
  ll_list->first = NULL;
  ll_list->last = NULL;
}

char* list_get_value(const Linked_List_t* ll_list, int index)
{
  char* result_str = NULL;
  struct Linked_List_Node* node = list_find(ll_list, index);

  if(node != NULL)
    result_str = node->text;

  return result_str;
}

struct Linked_List_Node* list_find(const Linked_List_t* ll_list, int index)
{
  struct Linked_List_Node* result_node = NULL;
  struct Linked_List_Node* node = ll_list->first;

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
      result_node = node;
      goto EXIT;
    }
  }

EXIT:
  return result_node;
}
