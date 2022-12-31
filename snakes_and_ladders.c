#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define VALID_ARGUMENTS 3
#define WRONG_NUM_ARGUMENTS "Usage: The program needs to get 2 parameters!\n"
#define TEN_BASE 10
#define RANDOM_WALK_I "Random Walk %d: "
#define RIGHT_ARROW " -> "
#define CELL "[%d]"
#define CELL_SNAKE_TO "[%d]-snake to %d"
#define CELL_LADDER_TO "[%d]-ladder to %d"
#define EXIT_CELL "[100]"
#define NEW_LINE "\n"

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell
{
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case
    // there is one from this square
    int snake_to;  // snake_to represents the jump of the snake
    // in case there is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

/** Error handler **/
static int handle_error (char *error_msg, MarkovChain **database)
{
  printf ("%s", error_msg);
  if (database != NULL)
  {
    free_markov_chain (database);
  }
  return EXIT_FAILURE;
}

static int create_board (Cell *cells[BOARD_SIZE])
{
  for (int i = 0; i < BOARD_SIZE; i++)
  {
    cells[i] = malloc (sizeof (Cell));
    if (cells[i] == NULL)
    {
      for (int j = 0; j < i; j++)
      {
        free (cells[j]);
      }
      handle_error (ALLOCATION_ERROR_MASSAGE, NULL);
      return EXIT_FAILURE;
    }
    *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
  }

  for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
  {
    int from = transitions[i][0];
    int to = transitions[i][1];
    if (from < to)
    {
      cells[from - 1]->ladder_to = to;
    }
    else
    {
      cells[from - 1]->snake_to = to;
    }
  }
  return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database (MarkovChain *markov_chain)
{
  Cell *cells[BOARD_SIZE];
  if (create_board (cells) == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  MarkovNode *from_node = NULL, *to_node = NULL;
  size_t index_to;
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    add_to_database (markov_chain, cells[i]);
  }

  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    from_node = get_node_from_database (markov_chain, cells[i])->data;

    if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
    {
      index_to = MAX(cells[i]->snake_to, cells[i]->ladder_to) - 1;
      to_node = get_node_from_database (markov_chain, cells[index_to])
          ->data;
      add_node_to_counter_list (from_node, to_node, markov_chain);
    }
    else
    {
      for (int j = 1; j <= DICE_MAX; j++)
      {
        index_to = ((Cell *) (from_node->data))->number + j - 1;
        if (index_to >= BOARD_SIZE)
        {
          break;
        }
        to_node = get_node_from_database (markov_chain, cells[index_to])
            ->data;
        add_node_to_counter_list (from_node, to_node, markov_chain);
      }
    }
  }
  // free temp arr
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    free (cells[i]);
  }
  return EXIT_SUCCESS;
}

/**
 *Get a cell and prints it
 * @param cell
 */
static void print_cell (void *cell)
{
  Cell *converted_cell = (Cell *) cell;
  if (converted_cell->number == BOARD_SIZE)
  {
    printf (EXIT_CELL);
    return;
  }
  else if (converted_cell->ladder_to != -1)
  {
    printf (CELL_LADDER_TO, converted_cell->number,
            converted_cell->ladder_to);
  }
  else if (converted_cell->snake_to != -1)
  {
    printf (CELL_SNAKE_TO, converted_cell->number,
            converted_cell->snake_to);
  }
  else
  {
    printf (CELL, converted_cell->number);
  }
  printf (RIGHT_ARROW);
}

/**
 * Compare if cell1 and cell2 are the same cells
 * @param cell1
 * @param cell2
 * @return 0 if they are that same , otherwise false
 */
static int compare_cells (void *cell1, void *cell2)
{
  Cell *converted_cell1 = (Cell *) cell1;
  Cell *converted_cell2 = (Cell *) cell2;
  return converted_cell1->number - converted_cell2->number;
}

/**
 *Get a cell as a generic element and free its content
 * return nothing
 */
static void free_cell (void *cell)
{
  Cell *converted_cell = (Cell *) cell;
  free (converted_cell);
  converted_cell = NULL;
}

/**
 * get a generic data and copy its content and return its copy
 * @param cell
 * @return the copy of this cell
 */
static void *copy_cell (void *cell)
{
  Cell *converted_cell = (Cell *) cell;
  Cell *copied_cell = malloc (sizeof (Cell));
  if (copied_cell == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    return NULL;
  }
  copied_cell->number = converted_cell->number;
  copied_cell->ladder_to = converted_cell->ladder_to;
  copied_cell->snake_to = converted_cell->snake_to;
  return (void *) copied_cell;
}
/**
 * Check if we get to a final cell
 * @param cell
 * @return true if it is a last cell and false otherwise
 */
static bool is_last_cell (void *cell)
{
  Cell *converted_cell = (Cell *) cell;
  if (converted_cell->number == BOARD_SIZE)
  {
    return true;
  }
  return false;
}

/**
 * Print the i'th random walks
 * @param markov_chain
 * @param i
 */
static void write_walk (MarkovChain *markov_chain, int i)
{
  printf (RANDOM_WALK_I, i);
  MarkovNode *first_markov_node = markov_chain->database->first->data;
  generate_random_sequence (markov_chain,
                            first_markov_node,
                            MAX_GENERATION_LENGTH);
  printf(NEW_LINE);
}

/**
 *fill the markov chain and prints the num_paths of walks, free the relevent
 * memory and return SUCCESS if we succeed otherwise FAILURE
 */
static bool fill_markov_chain_and_print_walks (int seed, int num_paths)
{
  //todo: check if i need to srand the seed
  srand (seed);
  MarkovChain *my_markov_chain =
      chain_initialize ((VoidPointer) print_cell,
                        (CmpFunc) compare_cells,
                        (VoidPointer) free_cell,
                        (CpyFunc) copy_cell,
                        (IsLast) is_last_cell);
  if (my_markov_chain == NULL)
  {
    return EXIT_FAILURE;
  }
  fill_database (my_markov_chain);
  for (int i = 0; i < num_paths; i++)
  {
    write_walk (my_markov_chain, i + 1);
  }
  free_markov_chain (&my_markov_chain);
  return EXIT_SUCCESS;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main (int argc, char *argv[])
{
  if (argc != VALID_ARGUMENTS)
  {
    printf (WRONG_NUM_ARGUMENTS);
    return EXIT_FAILURE;
  }
  unsigned int seed = (int) strtol (argv[1], NULL, TEN_BASE);
  int num_paths = (int) strtol (argv[2], NULL, TEN_BASE);
  return fill_markov_chain_and_print_walks (seed, num_paths);

}
