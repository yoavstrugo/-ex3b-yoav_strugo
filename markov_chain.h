#ifndef _MARKOV_CHAIN_H_
#define _MARKOV_CHAIN_H_
#define NDEBUG

#include "linked_list.h"
#include <stdio.h>  // For printf(), sscanf()
#include <stdlib.h> // For exit(), malloc()
#include <stdbool.h> // for bool

#define ALLOCATION_ERROR_MASSAGE "Allocation failure: Failed to allocate"\
            "new memory\n"

/***************************/
/*   insert typedefs here  */
/***************************/

/**
 * @brief This represents the frequency of a Markov node (a word), it
 * contains the pointer to the node, and the frequency at which it can
 * appear following the previous word.
 */
typedef struct MarkovNodeFrequency MarkovNodeFrequency;

/**
 * @brief This represents the whole Markov chain. It contains a list of all
 * the unique available words (Markov nodes) which we can start a tweet from.
 */
typedef struct MarkovNode MarkovNode;

typedef void *data_ptr_t;

typedef void (*print_func_t)(data_ptr_t);
typedef int (*comp_func_t)(data_ptr_t, data_ptr_t);
typedef data_ptr_t (*copy_func_t)(data_ptr_t);
typedef void (*free_data_t)(data_ptr_t);
typedef bool (*is_last_t)(data_ptr_t);

/**
 * @brief This represents a single markov node, a word, in a possible tweet.
 * It contains the word itself, as well as a frequency list to the possible
 * following words.
 */
typedef struct MarkovChain MarkovChain;

/***************************/

/***************************/
/*        STRUCTS          */
/***************************/

struct MarkovNodeFrequency
{
    /** A pointer to the markov node this frequency relates to */
    MarkovNode *markov_node;

    /** How many times, the word inside of `markov_node` appears after the
     * previous word, which its node points to this struct */
    int frequency;
};

struct MarkovChain
{
    /** Pointer to a linked list of the unique words (Markov nodes) the program
     * can use (which are read from the text corpus) */
    LinkedList *database;

    // pointer to a func that receives data from a generic type and prints it
    // returns void.
    print_func_t print_func;

    // pointer to a func that gets 2 pointers of generic data type(same one) and compare between them */
    // returns: - a positive value if the first is bigger
    //          - a negative value if the second is bigger
    //          - 0 if equal
    comp_func_t comp_func;

    // a pointer to a function that gets a pointer of generic data type and frees it.
    // returns void.
    free_data_t free_data;

    // a pointer to a function that  gets a pointer of generic data type and returns a newly allocated copy of it
    // returns a generic pointer.
    copy_func_t copy_func;

    //  a pointer to function that gets a pointer of generic data type and returns:
    //      - true if it's the last state.
    //      - false otherwise.
    is_last_t is_last;
};

struct MarkovNode
{
     data_ptr_t data;

    /** A list of the available paths from the current node (word) and the
     * frequency of each next word.
     * NULL if its the this node is the last in a sentence.
     * You are responsible for allocation and deallocation of this array.
     * */
    MarkovNodeFrequency *frequencies_list;

    /** The size of the dynamic array `frequencies_list` */
    int frequencies_list_size;

    /** The maximum size of the dynamic array `frequencies_list` */
    int frequencies_list_max_size;
};

/***************************/

/***************************/
/*        METHODS          */
/***************************/

/**
 * @brief A "constructor" for MarkovChain, you are resposible for freeing it
 * @return A new initialized instance (pointer) of MarkovChain. NULL if
 * memory allocation failed.
 */
MarkovChain *new_markov_chain (print_func_t print_func, comp_func_t
comp_func, copy_func_t copy_func, free_data_t free_data, is_last_t is_last);

/**
 * @brief Allocated the database field of the markov_chain, if it is not
 * already allocated.
 * @param markov_chain The markov chain
 * @return NULL if allocation failed, database pointer otherwise
 */
LinkedList *allocate_database (MarkovChain *markov_chain);

/**
 * @brief A "constructor" for MarkovNode, you are resposible for freeing it
 * @return A new initialized instance (pointer) of MarkovNode. NULL if
 * memory allocation failed.
 */
MarkovNode *new_markov_node ();

/**
 * @brief Adds one new place in the MarkovNodeFrequency dynamic array, for the
 * given MarkovNode. You are in charge of freeing it.
 * @param markov_node The MarkovNode to allocate the MarkovFrequencyNode to.
 * @return The newly allocated MarkovNodeFrequency, NULL if allocation failed.
 */
MarkovNodeFrequency *increase_markov_node_frequency_size(MarkovNode
*markov_node);

/**
* If data_ptr in markov_chain, return it's node. Otherwise, create new
 * node, add to end of markov_chain's database and return it.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return markov_node wrapping given data_ptr in given chain's database,
 * returns NULL in case of memory allocation failure.
 */
Node *add_to_database (MarkovChain *markov_chain, data_ptr_t data_ptr);

/**
* Check if data_ptr is in database. If so, return the markov_node wrapping it in
 * the markov_chain, otherwise return NULL.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return Pointer to the Node wrapping given state, NULL if state not in
 * database.
 */
Node *get_node_from_database (MarkovChain *markov_chain, data_ptr_t data_ptr);

/**
 * @brief Returns the index of the second_node inside the frequencies list of
 * the first node
 * @param first_node The node of the frequencies list
 * @param second_node The to search
 * @return The index of second_node inside first_node's frequencies list,
 * NOT_IN_ARRAY if it's not there.
 */
int get_node_from_frequencies_list (MarkovNode *first_node,
                                    MarkovNode *second_node);

/**
 * Add the second markov_node to the frequency list of the first markov_node.
 * If already in list, update it's occurrence frequency value.
 * @param first_node
 * @param second_node
 * @return success/failure: true if the process was successful, false if in
 * case of allocation error.
 */
bool
add_node_to_frequencies_list (MarkovNode *first_node, MarkovNode *second_node);

/**
 * Free markov_chain and all of it's content from memory
 * @param markov_chain markov_chain to free
 */
void free_database (MarkovChain **ptr_chain);

/**
 * Get one random state from the given markov_chain's database.
 * @param markov_chain
 * @return
 */
MarkovNode *get_first_random_node (MarkovChain *markov_chain);

/**
 * Choose randomly the next state, depend on it's occurrence frequency.
 * @param state_struct_ptr MarkovNode to choose from
 * @return MarkovNode of the chosen state
 */
MarkovNode *get_next_random_node (MarkovNode *state_struct_ptr);

/**
 * Receive markov_chain, generate and print random sentence out of it. The
 * sentence most have at least 2 words in it.
 * @param markov_chain
 * @param first_node markov_node to start with,
 *                   if NULL- choose a random markov_node
 * @param  max_length maximum length of chain to generate
 */
void generate_sentence (MarkovChain *markov_chain, MarkovNode *
first_node, int max_length);

/**
 * @brief Get random number between 0 and max_number [0, max_number).
 * @param max_number maximal number to return (not including).
 * @return Random number
 */
int get_random_number (int max_number);

/**
 * @brief Returns the node in the given index from the linked list
 * @param list The linked list
 * @param index The index
 * @return Node in the given index
 */
Node *get_node_in_index(LinkedList *list, int index);

#endif /* _MARKOV_CHAIN_H_ */