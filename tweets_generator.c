#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "markov_chain.h"
#define VALID_ARGUMENTS 4
#define VALID_ARGUMENTS_2 5
#define MAX_LEN_TWEET 1001
#define TEN_BASE 10
#define MAX_TWEET 20
#define WRONG_NUM_ARGUMENTS "Usage: The program needs to get 3/4 parameters!\n"
#define WRONG_PATH "Error: The path to Text corpus doesn't work/exist\n"
#define FALSE false
#define TRUE true
#define NEW_LINE "\n"
#define TWEET_NUM_I "Tweet %d: "

/**
 * Get a word and return if it ends with . or not
 * @param char* word
 * @return true is the word end with point, otherwise false
 */
//todo: check maybe the argument to generic
static bool is_last_word_str (char *word)
{
  char last = word[strlen (word) - 1];
  if (last == '.' || last == '\n')
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/**
 * @param curr_sentence The current sentence we are in
 * @param words_to_read The number of words the user told us to read
 * @param curr_word The current word we in
 * @param curr_tweet The current tweet we in
 * @param curr_node The current word the includes the word itself
 * @param prev_word The previous word we want to add to her counterlist
 * array our current word
 * @param markov_chain the markov chain we fill
 * @param is_problem a variable that helps the loop to know when we need to
 * stop the loop
 * @param fp The file we read from
 */
static void add_words_to_db (int words_to_read, char *curr_tweet,
                             MarkovNode *curr_node, MarkovNode *prev_word,
                             MarkovChain *markov_chain,
                             int is_problem, FILE *fp)
{
  char *curr_sentence = fgets (curr_tweet, MAX_LEN_TWEET, fp);
  char *curr_word;
  while (curr_sentence != NULL && (words_to_read != 0))
  {
    curr_word = strtok (curr_tweet, " \n");
    while (curr_word != NULL && words_to_read != 0)
    {
      curr_node = (add_to_database (markov_chain, curr_word))->data;
      if (curr_node == NULL)
      {
        free_markov_chain (&markov_chain);
        is_problem = 1;
        break;
      }
      if (prev_word != NULL)
      {
        add_node_to_counter_list (prev_word, curr_node, markov_chain);
      }
      if (is_last_word_str (curr_word) == FALSE)
      {
        prev_word = curr_node;
      }
      else
      {
        prev_word = NULL;
      }
      words_to_read--;
      curr_word = strtok (NULL, " \n");
    }
    if (is_problem == 0)
    {
      prev_word = NULL;
      curr_sentence = fgets (curr_tweet, MAX_LEN_TWEET, fp);
    }
  }
}

/**
 * The function gets a file and number of words and pt to ds and fill the ds
 * @param fp
 * @param words_to_read
 * @param markov_chain
 */

static void fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain)
{
  MarkovNode *prev_word;
  char curr_tweet[MAX_LEN_TWEET];
  MarkovNode *curr_node;
  curr_node = NULL;
  prev_word = NULL;
  int is_problem = 0;
  add_words_to_db (words_to_read, curr_tweet,
                   curr_node, prev_word, markov_chain, is_problem, fp);
}
/**
 * The function get a markov chain and and the number of tweet(i) and print
 * a random tweet
 * @param markov_chain
 * @param i
 */
static void write_tweets (MarkovChain *markov_chain, int i)
{
  printf (TWEET_NUM_I, i);
  MarkovNode *first_node = get_first_random_node (markov_chain);
  generate_random_sequence (markov_chain, first_node, MAX_TWEET);
  printf (NEW_LINE);
}

/**
 * The function gets a string and make a copy of it and return the new
 * copied char
 * @param str
 * @return the new copied string
 */
static void *strcpy_to_copy_str (void *str)
{
  char *char_str = (char *) str;
  char *my_copied_str = malloc (sizeof (char) * (strlen (char_str) + 1));
  if (my_copied_str == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    return NULL;
  }
  strcpy (my_copied_str, char_str);
  return (void *) my_copied_str;
}

/** pointer to a func that gets 2 pointers of generic data type(same one)
 * @param data1  first pointer
 * @param data2  second pointer
 * and compare between them
 * returns: - a positive value if the first is bigger
 *          - a negative value if the second is bigger
 *          - 0 if equal
 * @param data1
 * @param data2
 */
static int compare_strings (void *data1, void *data2)
{
  char *str1 = (char *) data1;
  char *str2 = (char *) data2;
  int res = strcmp (str1, str2);
  return res;
}

/**
 * The function gets string and prints it
 * @param str1
 */
static void print_string (void *str1)
{
  char *converted_str = (char *) str1;
  if (is_last_word_str (converted_str) == TRUE)
  {
    printf ("%s", converted_str);
  }
  else
  {
    printf ("%s ", converted_str);
  }
}

/**
 * The function gets a string and free it from the memory
 * returns Nothing
 * @param a str to free
 */
static void free_str (char *str)
{
  free (str);
}

/**
 * The function fill the markov chain with the input file and than print the
 * number of tweets that were given
 * @param num_words_to_read
 * @param input_file the input file
 * @param seed the seef for the random function
 * @param num_tweets number of tweets to print
 * @return false if there were a problem, EXIT_SUCCESS otherwise
 */
static bool fill_markovchain_and_print_tweets (int num_words_to_read, FILE *
input_file, int seed, int num_tweets)
{
  srand (seed);
  MarkovChain *my_markov_chain =
      chain_initialize ((VoidPointer) print_string,
                        (CmpFunc) compare_strings,
                        (VoidPointer) free_str,
                        (CpyFunc) strcpy_to_copy_str,
                        (IsLast) is_last_word_str);
  if (my_markov_chain == NULL)
  {
    return EXIT_FAILURE;
  }

  if (num_words_to_read != 0)
  {
    fill_database (input_file, num_words_to_read,
                   my_markov_chain);
  }
  else
  {
    //todo: check the option that we didnt succedd
    fill_database (input_file, -1, my_markov_chain);
  }
  for (int i = 1; i <= num_tweets; i++)
  {
    write_tweets (my_markov_chain, i);
  }
  free_markov_chain (&my_markov_chain);
  //todo: have been here linkedlist but i passed it to markov chain
  //free (my_markov_chain->database);
  fclose (input_file);
  return EXIT_SUCCESS;
}

int main (int argc, char *argv[])
{
  if (argc != VALID_ARGUMENTS && argc != VALID_ARGUMENTS_2)
  {
    printf (WRONG_NUM_ARGUMENTS);
    return EXIT_FAILURE;
  }
  unsigned int seed = (int) strtol (argv[1], NULL, TEN_BASE);
  int num_tweets = (int) strtol (argv[2], NULL, TEN_BASE);
  //Checking the path
  FILE *input_file = fopen (argv[3], "r");
  if (input_file == NULL)
  {
    printf (WRONG_PATH);
    return EXIT_FAILURE;
  }
  int num_words_to_read;
  if (argc == 4)
  {
    num_words_to_read = 0; //Read all the file
  }
  else
  {
    num_words_to_read = (int) strtol (argv[4], NULL, TEN_BASE);
  }
  //Start my program
  return fill_markovchain_and_print_tweets (num_words_to_read,
                                            input_file, seed, num_tweets);
}