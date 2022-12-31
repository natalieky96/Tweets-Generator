#include "linked_list.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "markov_chain.h"
#define ALLOCATION_ERROR_MARKOVNODE \
"Allocation failure: fail to allocate memory for markovnode\n"
#define ADD_FAILURE 1
#define TRUE true
#define FALSE false

/**
* Get random number between 0 and max_number [0, max_number).
* @param max_number maximal number to return (not including)
* @return Random number
*/
int get_random_number (int max_number)
{
  return rand () % max_number;
}

/**
 * The function gets the word as data of the current MarkovNode
 * returns a pointer to the current new markov node
 */
MarkovNode *new_markov_node (void *data, CpyFunc *copy_func)
{
  MarkovNode *new_markov_node = (MarkovNode *) malloc (sizeof (MarkovNode));
  if (new_markov_node == NULL)
  {
    printf (ALLOCATION_ERROR_MARKOVNODE);
    return NULL;
  }
  //?????? CHANGE THIS and i deleted the malloc because copy malooc by itself
  //strcpy (str, data);
  void *cpy_data = (*copy_func) (data);
  new_markov_node->data = cpy_data;
  new_markov_node->possible_words_arr = NULL;
  new_markov_node->length = 0;
  new_markov_node->total_words = 0;
  return new_markov_node;
}
/**
 * Get a number-i and return the i'th number in the database list
 * @param markov_chain and k which is the kth node we want to return
 * @return the MarkovNode
 */
MarkovNode *get_ith_node (MarkovChain *markov_chain, int k)
{
  if (markov_chain != NULL)
  {
    Node *curr_node = markov_chain->database->first;
    for (int i = 0; i < k; i++)
    {
      curr_node = curr_node->next;
    }
    return curr_node->data;
  }
  return NULL;
}

MarkovNode *get_first_random_node (MarkovChain *markov_chain)
{
  IsLast is_last_node = markov_chain->is_last;
  int random_num = get_random_number (markov_chain->database->size);
  MarkovNode *curr_markov_node = get_ith_node (markov_chain, random_num);
  while (is_last_node (curr_markov_node->data))
  {
    random_num = get_random_number (markov_chain->database->size);
    curr_markov_node = get_ith_node (markov_chain, random_num);
  }
  return curr_markov_node;
}

MarkovNode *get_next_random_node (MarkovNode *state_struct_ptr)
{
  int rnd_num = get_random_number (state_struct_ptr->total_words);
  if (state_struct_ptr->possible_words_arr == NULL)
  {
    return NULL;
  }
  int i = -1;
  NextNodeCounter curr_nxt_node;

  while (rnd_num >= 0)
  {
    i++;
    curr_nxt_node = state_struct_ptr->possible_words_arr[i];
    rnd_num = rnd_num - (curr_nxt_node.frequency);
  }
  return curr_nxt_node.markov_node;
}

void generate_random_sequence (MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
  IsLast is_last_node = markov_chain->is_last;
  VoidPointer print_func = markov_chain->print_func;
  while (first_node == NULL || is_last_node (first_node->data) == TRUE)
  {
    first_node = get_first_random_node (markov_chain);
  }
  MarkovNode *curr_node = first_node;
  while (max_length != 0 && is_last_node (curr_node->data) == FALSE)
  {
    if (curr_node == first_node)
    {
      print_func (curr_node->data);
    }
    else
    {
      print_func (curr_node->data);
    }
    max_length--;
    curr_node = get_next_random_node (curr_node);
    if (curr_node == NULL)
    {
      break;
    }
  }
  if (is_last_node (curr_node->data) != FALSE)
  {
    print_func (curr_node->data);
  }
}

/**
 * Free markov_node and all of it's content from memory
 * @param free_markov_node to free
 * @param free_func the function to use to free the data
 */
void free_markov_node (MarkovNode *markov_node, VoidPointer *free_func)
{
  if (markov_node != NULL)
  {
    (*free_func) (markov_node->data);
    //free the counter_list
    NextNodeCounter *counter_list_arr = markov_node->possible_words_arr;
    if (counter_list_arr != NULL)
    {
      free (counter_list_arr);
    }
    free (markov_node);
    markov_node = NULL;
  }
}
/**
 * Free all of node's  content from memory
 * @param node to free it's content
 */
void free_content_node (Node *node, VoidPointer *free_func)
{
  if (node != NULL)
  {
    //free the Markov node that inside of the node
    free_markov_node (node->data, free_func);
    node->data = NULL;
  }
}

/**
 * Free the linked list's content from memory
 * @param LinkedList linked_list to free
 */
void free_linked_list (LinkedList *linked_list, VoidPointer *free_func)
{
  if (linked_list != NULL)
  {
    if ((linked_list->size) >= 1)
    {
      Node *curr_node = linked_list->first;
      Node *nxt_node = curr_node->next;
      for (int i = 0; i < linked_list->size; i++)
      {
        free_content_node (curr_node, free_func);
        free (curr_node);
        curr_node = nxt_node;
        if (nxt_node != NULL)
        {
          nxt_node = nxt_node->next;
        }
      }
      free_content_node (curr_node, free_func);
      free (curr_node);
    }
      //It is a single node
    else
    {
      free_content_node (linked_list->first, free_func);
      free (linked_list->first);
    }
    free (linked_list);
  }
}
/**
 * Free markov_chain and all of it's content from memory
 * @param markov_chain markov_chain to free
 */
void free_markov_chain (MarkovChain **ptr_chain)
{
  free_linked_list ((*ptr_chain)->database, &((*ptr_chain)->free_data));
  free (*ptr_chain);
  *ptr_chain = NULL;
}

/**
 * Add the second markov_node to the counter list of the first markov_node.
 * If already in list, update it's counter value.
 * @param first_node
 * @param second_node
 * @param markov_chain
 * @return success/failure: true if the process was successful, false if in
 * case of allocation error.
 */
bool
add_node_to_counter_list (MarkovNode *first_node, MarkovNode *second_node,
                          MarkovChain *markov_chain)
{
  CmpFunc comp_func = markov_chain->comp_func;
  NextNodeCounter *curr_counter_arr;
  int runner;
  if (first_node->possible_words_arr != NULL)
  {
    curr_counter_arr = first_node->possible_words_arr;
    for (int i = 0; i < first_node->length; i++)
    {
      //we find the second node and increase it frequency +1
      if (comp_func (curr_counter_arr[i].markov_node->data, second_node->data)
          == 0)
      {
        first_node->possible_words_arr[i].frequency++;
        first_node->total_words++;
        return true;
      }
    }
  }
  NextNodeCounter *curr_arr = first_node->possible_words_arr;
  NextNodeCounter *new_counter_arr =
      realloc (curr_arr, (first_node->length + 1)
                         * sizeof (NextNodeCounter));
  if (new_counter_arr == NULL)
  {
    return false;
  }
  first_node->possible_words_arr = new_counter_arr;
  runner = (first_node->length);
  (first_node->possible_words_arr)[runner].markov_node = second_node;
  (first_node->possible_words_arr)[runner].frequency = 1;
  first_node->total_words++;
  //our counter list of the first node grow by 1
  first_node->length++;
  return true;
}

/**
* Check if data_ptr is in database.If so,return the markov_node wrapping
 * it in the markov_chain, otherwise return NULL.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return Pointer to the Node wrapping given state, NULL if state not in
 * database.
 */
Node *get_node_from_database (MarkovChain *markov_chain, void *data_ptr)
{
  CmpFunc compare_func = markov_chain->comp_func;
  Node *runner = markov_chain->database->first;
  if (runner == NULL)
  {
    return runner;
  }
  while ((runner) != (markov_chain->database->last))
  {
    if (compare_func (runner->data->data, data_ptr) == 0)
    {
      return runner;
    }
    runner = runner->next;
  }
  //Comparing the last node with the data_ptr
  if (compare_func (runner->data->data, data_ptr) == 0)
  {
    return runner;
  }
  //data_ptr is not in markov chain database
  return NULL;
}

/**
* If data_ptr in markov_chain, return it's markov_node. Otherwise, create new
 * markov_node, add to end of markov_chain's database and return it.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return node wrapping given data_ptr in given chain's database,
 * returns NULL in case of memory allocation failure.
 */
Node *add_to_database (MarkovChain *markov_chain, void *data_ptr)
{
  Node *search_node = get_node_from_database (markov_chain, data_ptr);
  if (search_node != NULL)
  {
    return search_node;
  }

  //Add the MarkovNode to the end and update the last node in the MarkovChain
  MarkovNode *my_markov_node = new_markov_node
      (data_ptr, &(markov_chain->copy_func));
  if (my_markov_node == NULL)
  {
    return NULL;
  }
  //if the markov chain is empty
  int add_succes = add (markov_chain->database, my_markov_node);
  if (add_succes == ADD_FAILURE)
  {
    return NULL;
  }
  return markov_chain->database->last;
}

MarkovChain *chain_initialize (VoidPointer print_func, CmpFunc comp_func,
                               VoidPointer free_func, CpyFunc copy_func,
                               IsLast is_last_func)
{
  MarkovChain *my_markov_chain = malloc (sizeof (MarkovChain));
  if (my_markov_chain == NULL)
  {
    printf(ALLOCATION_ERROR_MASSAGE);
    return NULL;
  }
  my_markov_chain->is_last = is_last_func;
  my_markov_chain->print_func = print_func;
  my_markov_chain->copy_func = copy_func;
  my_markov_chain->comp_func = comp_func;
  my_markov_chain->free_data = free_func;

  LinkedList *linked_list = malloc (sizeof (LinkedList));
  if (linked_list == NULL)
  {
    printf(ALLOCATION_ERROR_MASSAGE);
    free (my_markov_chain);
    return NULL;
  }
  my_markov_chain->database = linked_list;
  my_markov_chain->database->first = NULL;
  my_markov_chain->database->last = NULL;
  my_markov_chain->database->size = 0;

  return my_markov_chain;
}
