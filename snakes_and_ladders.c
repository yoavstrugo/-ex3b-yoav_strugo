#include <string.h> // For strlen(), strcmp(), strcpy()
#include <assert.h>

#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY (-1)
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20

#define PROGRAM_NAME_ARG_INDEX  0
#define SEED_ARG_INDEX  1
#define NUM_OF_SENTENCES_ARG_INDEX  2
#define ARG_COUNT 3

#define DECIMAL_BASE            10

#define USAGE_FORMAT   "Usage: %s [seed] [num_of_sentences]"

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
typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case there is one from this square
    int snake_to;  // snake_to represents the jump of the snake in case there is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

/** Error handler **/
static int handle_error(char *error_msg, MarkovChain **database) {
    printf("%s", error_msg);
    if (database != NULL) {
        free_database(database);
    }
    return EXIT_FAILURE;
}

/**
 * @brief Prints the usage message for the program
 * @param program_name The program's name. should be in argv[0]
 */
void usage (char *program_name)
{
  fprintf (stdout, USAGE_FORMAT, basename (program_name));
}

static int create_board(Cell *cells[BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        cells[i] = malloc(sizeof(Cell));
        if (cells[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(cells[j]);
            }
            handle_error(ALLOCATION_ERROR_MASSAGE, NULL);
            return EXIT_FAILURE;
        }
        *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }

    for (int i = 0; i < NUM_OF_TRANSITIONS; i++) {
        int from = transitions[i][0];
        int to = transitions[i][1];
        if (from < to) {
            cells[from - 1]->ladder_to = to;
        } else {
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
static int fill_database(MarkovChain *markov_chain) {
    Cell *cells[BOARD_SIZE];
    if (create_board(cells) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    MarkovNode *from_node = NULL, *to_node = NULL;
    size_t index_to;
    for (size_t i = 0; i < BOARD_SIZE; i++) {
        add_to_database(markov_chain, cells[i]);
    }

    for (size_t i = 0; i < BOARD_SIZE; i++) {
        from_node = get_node_from_database(markov_chain, cells[i])->data;

        if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY) {
            index_to = MAX(cells[i]->snake_to, cells[i]->ladder_to) - 1;
            to_node = get_node_from_database(markov_chain, cells[index_to])
                    ->data;
            add_node_to_frequencies_list(from_node, to_node);
        } else {
            for (int j = 1; j <= DICE_MAX; j++) {
                index_to = ((Cell *) (from_node->data))->number + j - 1;
                if (index_to >= BOARD_SIZE) {
                    break;
                }
                to_node = get_node_from_database(markov_chain, cells[index_to])
                        ->data;
                add_node_to_frequencies_list(from_node, to_node);
            }
        }
    }
    // free temp arr
    for (size_t i = 0; i < BOARD_SIZE; i++) {
        free(cells[i]);
    }
    return EXIT_SUCCESS;
}

void parse_arguments(char *argv[], unsigned int *seed, int
*num_of_sentences) {
    char *end_ptr;

    *seed = (unsigned int) strtol(argv[SEED_ARG_INDEX], &end_ptr,
                                  DECIMAL_BASE);
    *num_of_sentences =
            (int) strtol(argv[NUM_OF_SENTENCES_ARG_INDEX], &end_ptr,
                         DECIMAL_BASE);
}

bool check_usage(int argc, char *argv[]) {
    if (argc != ARG_COUNT) {
        return true;
    }

    return false;
}

bool is_cell_last(Cell *cell)

{
    return cell->number == BOARD_SIZE;
}

void print_cell(Cell *cell) {
    printf("[%d]", cell->number);
    if (cell->snake_to != EMPTY) {
        printf("-snake to %d", cell->snake_to);
    } else if (cell->ladder_to != EMPTY) {
        printf("-ladder to %d", cell->ladder_to);
    }

    if (!is_cell_last(cell)) {
        printf(" ->");
    }
}

int compare_cells(Cell *a, Cell *b)
{
    assert(a != NULL);
    assert(b != NULL);

    return a->number - b->number;
}

Cell *dupcell(Cell *cell)
{
    Cell *duplicated_cell = (Cell *)malloc(sizeof *duplicated_cell);
    if (duplicated_cell == NULL)
    {
        return NULL;
    }

    memcpy(duplicated_cell, cell, sizeof(*cell));
    return duplicated_cell;
}

void generate_walks(int num_of_walks, MarkovChain *markov_chain,
                    MarkovNode *first_markov_node)
{
    for (int i = 0; i < num_of_walks; ++i) {
        printf("Random Walk %d: ", i + 1);
        generate_tweet(markov_chain, first_markov_node, MAX_GENERATION_LENGTH);
    }
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
    if (check_usage(argc, argv)) {
        usage(argv[PROGRAM_NAME_ARG_INDEX]);
        return EXIT_FAILURE;
    }

    unsigned int seed;
    int num_of_sentences;

    parse_arguments(argv, &seed, &num_of_sentences);

    MarkovChain *markov_chain = new_markov_chain(
    (print_func_t) print_cell,
    (comp_func_t) compare_cells,
    (copy_func_t) dupcell,
    (free_data_t) free,
    (is_last_t) is_cell_last
    );

    if (fill_database(markov_chain) == EXIT_FAILURE)
    {
        free_database(&markov_chain);
        return EXIT_FAILURE;
    }

    Cell first_cell_data = {.number = 1};

    Node *first_node = get_node_from_database(markov_chain,
                                                &first_cell_data);

    srand(seed);
    generate_walks(num_of_sentences, markov_chain, first_node->data);

    free_database(&markov_chain);
    return EXIT_SUCCESS;
}
