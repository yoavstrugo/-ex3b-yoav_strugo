#include <string.h>
#include <assert.h>

#include "markov_chain.h"

#define STRCMP_EQUAL  0

#define LINKED_LIST_ADD_FAILED  1

#define NOT_IN_ARRAY ((int)(-1))

MarkovChain *new_markov_chain(print_func_t print_func, comp_func_t
comp_func, copy_func_t copy_func, free_data_t free_data, is_last_t is_last) {
    MarkovChain *markov_chain = (MarkovChain *) malloc(sizeof *markov_chain);
    markov_chain->database = NULL;

    markov_chain->print_func = print_func;
    markov_chain->comp_func = comp_func;
    markov_chain->copy_func = copy_func;
    markov_chain->free_data = free_data;
    markov_chain->is_last = is_last;

    return markov_chain;
}

MarkovNode *new_markov_node() {
    MarkovNode *markov_node = (MarkovNode *) malloc(sizeof *markov_node);
    markov_node->data = NULL;
    markov_node->frequencies_list = NULL;
    markov_node->frequencies_list_size = 0;
    markov_node->frequencies_list_max_size = 0;

    return markov_node;
}

LinkedList *allocate_database(MarkovChain *markov_chain) {
    assert(markov_chain != NULL);

    if (markov_chain->database == NULL) {
        // allocate database
        markov_chain->database =
                (LinkedList *) malloc(sizeof *markov_chain->database);

        if (markov_chain->database == NULL) {
            return NULL;
        }

        markov_chain->database->first = NULL;
        markov_chain->database->last = NULL;
        markov_chain->database->size = 0;
    }

    return markov_chain->database;
}

Node *add_to_database(MarkovChain *markov_chain, data_ptr_t data_ptr) {
    assert (markov_chain != NULL);
    assert(data_ptr != NULL);

    if (allocate_database(markov_chain) == NULL) {
        return NULL;
    }

    Node *existing_node = get_node_from_database(markov_chain, data_ptr);

    if (existing_node != NULL) {
        return existing_node;
    }

    // The node does not exist, create it
    MarkovNode *markov_node = new_markov_node();
    if (markov_node == NULL) {
        return NULL;
    }

    // duplicate the data
    markov_node->data = markov_chain->copy_func(data_ptr);
    if (markov_node->data == NULL) {
        return NULL;
    }

    if (add(markov_chain->database, markov_node) == LINKED_LIST_ADD_FAILED) {
        return NULL;
    }

    return markov_chain->database->last;
}

Node *get_node_from_database(MarkovChain *markov_chain, data_ptr_t data_ptr) {
    assert(markov_chain != NULL);

    if (markov_chain->database == NULL) {
        return NULL;
    }

    Node *current_node = markov_chain->database->first;

    // Try to find the node inside the linked list
    while (current_node != NULL) {
        MarkovNode *current_markov_node = current_node->data;

        if (markov_chain->comp_func(current_markov_node->data, data_ptr) ==
            STRCMP_EQUAL) {
            return current_node;
        }

        current_node = current_node->next;
    }

    return NULL;
}

MarkovNodeFrequency *
increase_markov_node_frequency_size(MarkovNode *markov_node) {
    assert(markov_node != NULL);

    // Allocate frequencies list if it does not exist
    if (markov_node->frequencies_list == NULL) {
        markov_node->frequencies_list =
                (MarkovNodeFrequency *) malloc(
                        sizeof *markov_node->frequencies_list);

        if (markov_node->frequencies_list != NULL) {
            markov_node->frequencies_list_max_size = 1;
            markov_node->frequencies_list_size = 0;
            markov_node->frequencies_list->frequency = 0;
            markov_node->frequencies_list->markov_node = NULL;
        }

        return markov_node->frequencies_list;
    }

    // If it exists, resize it

    // Increase frequencies_list_size by one
    MarkovNodeFrequency *realloced_markov_node =
            (MarkovNodeFrequency *) realloc(markov_node->frequencies_list,
                                            (markov_node->frequencies_list_size +
                                             1)
                                            * (sizeof *markov_node
                                                    ->frequencies_list
                                            ));

    if (realloced_markov_node == NULL) {
        return NULL;
    }

    markov_node->frequencies_list = realloced_markov_node;

    markov_node->frequencies_list_max_size += 1;

    return markov_node->frequencies_list;
}

int get_node_from_frequencies_list(MarkovNode *first_node,
                                   MarkovNode *second_node) {
    assert(first_node != NULL);
    assert(second_node != NULL);

    if (first_node->frequencies_list == NULL) {
        return NOT_IN_ARRAY;
    }

    for (int i = 0; i < first_node->frequencies_list_size; ++i) {
        // The node in the frequencies array, and the second_node should be in
        // the same place in memory
        if (first_node->frequencies_list[i].markov_node == second_node) {
            return i;
        }
    }

    return NOT_IN_ARRAY;
}

bool
add_node_to_frequencies_list(MarkovNode *first_node, MarkovNode *second_node) {
    assert(first_node != NULL);
    assert(second_node != NULL);

    int node_idx = get_node_from_frequencies_list(first_node, second_node);
    if (node_idx != NOT_IN_ARRAY) {
        // node exists, increase its frequency
        first_node->frequencies_list[node_idx].frequency++;
        return true;
    }

    // node does not exist, add it
    if (increase_markov_node_frequency_size(first_node) == NULL) {
        // allocation error
        return false;
    }

    node_idx = first_node->frequencies_list_size;
    MarkovNodeFrequency *node_frequency =
            &first_node->frequencies_list[node_idx];

    node_frequency->frequency = 1;
    node_frequency->markov_node = second_node;

    first_node->frequencies_list_size++;

    return true;
}

void free_database(MarkovChain **ptr_chain) {
    if (*ptr_chain == NULL) {
        return;
    }

    if ((*ptr_chain)->database == NULL) {
        return;
    }

    Node *next_node = (*ptr_chain)->database->first;

    while (next_node != NULL) {
        Node *prev_node;
        MarkovNode *markov_node = next_node->data;

        assert(markov_node != NULL);
        assert(markov_node->data != NULL);

        // free the word itself
        (*ptr_chain)->free_data((void *) markov_node->data);
        // free the frequencies list
        if (markov_node->frequencies_list != NULL) {
            free(next_node->data->frequencies_list);
        }
        // free the markov node
        free(markov_node);

        prev_node = next_node;
        next_node = prev_node->next;

        // free the linked list node
        free(prev_node);
    }

    // free the database linked list
    free((*ptr_chain)->database);
    free(*ptr_chain);
    *ptr_chain = NULL;
}

int get_random_number(int max_number) {
    return rand() % max_number;
}

Node *get_node_in_index(LinkedList *list, int index) {
    Node *curr_node = list->first;
    for (int i = 0; i < index; ++i) {
        curr_node = curr_node->next;
    }
    return curr_node;
}

MarkovNode *get_first_random_node(MarkovChain *markov_chain) {
    assert(markov_chain != NULL);
    assert(markov_chain->database != NULL);

    Node *node;
    do {
        int random_index = get_random_number(markov_chain->database->size);
        node = get_node_in_index(markov_chain->database, random_index);
    } while (markov_chain->is_last(node->data->data));

    return node->data;
}

MarkovNode *get_next_random_node(MarkovNode *state_struct_ptr) {
    assert(state_struct_ptr != NULL);

    if (state_struct_ptr->frequencies_list_size == 0) {
        return NULL;
    }

    assert(state_struct_ptr->frequencies_list != NULL);

    // calculate wights sum
    int total_weight = 0;
    for (int i = 0; i < state_struct_ptr->frequencies_list_size; ++i) {
        total_weight += state_struct_ptr->frequencies_list[i].frequency;
    }

    int random_weight = get_random_number(total_weight);

    for (int i = 0; i < state_struct_ptr->frequencies_list_size; ++i) {
        if (random_weight < state_struct_ptr->frequencies_list[i].frequency) {
            return state_struct_ptr->frequencies_list[i].markov_node;
        }

        random_weight -= state_struct_ptr->frequencies_list[i].frequency;
    }

    return NULL;
}

void generate_tweet(MarkovChain *markov_chain, MarkovNode *first_node,
                    int max_length) {
    assert(markov_chain != NULL);

    if (first_node == NULL) {
        first_node = get_first_random_node(markov_chain);
    }

    MarkovNode *prev_node;
    prev_node = first_node;
    assert(first_node != NULL);
    assert(first_node->data != NULL);

    markov_chain->print_func(first_node->data);

    MarkovNode *next_node;
    // start i from 1 because we already have first node
    for (int i = 1; i < max_length; ++i) {
        next_node = get_next_random_node(prev_node);

        if (next_node == NULL) {
            break;
        }

        assert(next_node->data != NULL);
        markov_chain->print_func(next_node->data);

        if (markov_chain->is_last(next_node->data)) {
            break;
        }

        prev_node = next_node;
    }

    printf("\n");
}