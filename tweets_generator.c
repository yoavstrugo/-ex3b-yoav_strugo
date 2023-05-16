#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

#include "markov_chain.h"

#define USAGE_FORMAT "Usage: %s [seed] [num_of_tweets] \
[text_corpus] ?[num_of_words]\n"
#define ERROR_OPEN_FILE_FMT "Error: Failed to open file %s.\n"


#define ARG_COUNT_WITH_NUM_OF_WORD    5
#define ARG_COUNT_WITHOUT_NUM_OF_WORD 4

#define PROGRAM_NAME_ARG_INDEX  0
#define SEED_ARG_INDEX          1
#define TWEET_COUNT_ARG_INDEX   2
#define TEXT_CORPUS_ARG_INDEX   3
#define WORD_COUNT_ARG_INDEX    4

#define READ_ALL_WORDS          (-1)
#define MAX_SENTENCE_LENGTH     1000
#define MAX_TWEET_LENGTH 20

#define DECIMAL_BASE            10

/**
 * @brief Fills the database of the given markov chain, from the words in
 * the given file
 * @param fp the file's pointer
 * @param words_to_read How many words to read? READ_ALL_WORDS to read the
 * whole file
 * @param markov_chain Point to the markov chain
 * @return true if memory allocation failed, false on success.
 */
bool fill_database(FILE *fp, int words_to_read, MarkovChain *markov_chain);

/**
 * @brief Parses the arguments and sets the respective variables. prints a
 * respective message if openning file failed.
 * @param argc
 * @param argv
 * @return true if file opening failed, false otherwise
 */
bool parse_arguments(int argc, char *argv[], unsigned int *seed, int
*num_of_tweets, int *num_of_words, FILE **text_corpus_fp);

/*
 * @brief Prints the usage message for the program
 * @param program_name The program's name. should be in argv[0]
 */
void usage (char *program_name);

/**
 * @brief Generates the specified amount of tweets from the markov chain.
 * @param num_of_tweets
 * @param markov_chain
 */
void generate_tweets(int num_of_tweets, MarkovChain *markov_chain);

bool ends_with_dot(const char *string);

void print_word(const char *word);

/**
 * @brief The main function of the program. The program will generate random
 * tweets, using the tweets in text corpus.
 * This function is mainly responsible for command-line argument parsing.
 * and the program's main flow.
 * @return EXIT_SUCCESS if everything succeeded, EXIT_FAILURE otherwise with
 * a helpful error message.
 */
int main(int argc, char *argv[]) {
    int num_of_tweets, num_of_words;
    unsigned int seed;
    FILE *text_corpus_fp;
    MarkovChain *markov_chain;

    /** input validation */
    if (argc != ARG_COUNT_WITHOUT_NUM_OF_WORD
        && argc != ARG_COUNT_WITH_NUM_OF_WORD) {
        usage(argv[PROGRAM_NAME_ARG_INDEX]);
        return EXIT_FAILURE;
    }

    /** we have to check that the file could open */
    if (parse_arguments(argc, argv, &seed, &num_of_tweets, &num_of_words,
                        &text_corpus_fp)) {
        return EXIT_FAILURE;
    }

    /** main program flow, fill database then generate tweets */

    // initialize random
    srand(seed);

    markov_chain = new_markov_chain((print_func_t) print_word,
                                    (comp_func_t) strcmp,
                                    (copy_func_t) strdup,
                                    (free_data_t) free,
                                    (is_last_t) ends_with_dot);

    if (fill_database(text_corpus_fp, num_of_words, markov_chain)) {
        printf(ALLOCATION_ERROR_MASSAGE);
        return true;
    }

    generate_tweets(num_of_tweets, markov_chain);

    // free the database and the chain
    free_database(&markov_chain);
    free(markov_chain);

    fclose(text_corpus_fp);
    return EXIT_SUCCESS;
}

void generate_tweets(int num_of_tweets, MarkovChain *markov_chain) {
    for (int i = 0; i < num_of_tweets; ++i) {
        printf("Tweet %d: ", i + 1);
        generate_tweet(markov_chain, NULL, MAX_TWEET_LENGTH);
    }
}

/**
 * @brief Prints the usage message for the program
 * @param program_name The program's name. should be in argv[0]
 */
void usage (char *program_name)
{
  fprintf (stdout, USAGE_FORMAT, basename (program_name));
}


char *strdup(const char *str) {
    char *duplicated_str = (char *) malloc(strlen(str) + 1);
    if (duplicated_str == NULL) {
        return NULL;
    }

    strcpy(duplicated_str, str);
    return duplicated_str;
}

bool ends_with_dot(const char *string) {
    if (string[strlen(string) - 1] == '.') {
        return true;
    }

    return false;
}

void print_word(const char *word) {
    printf("%s", word);

    if (!ends_with_dot(word))
    {
        printf(" ");
    }
}


bool
add_sentence_to_database(MarkovChain *markov_chain, char *sentence_buffer,
                         int *words_to_read) {
    char *word_pointer = strtok(sentence_buffer, " ");
    Node *prev_word = NULL;

    while (word_pointer != NULL && ((*words_to_read > 0) || (*words_to_read ==
                                                             READ_ALL_WORDS))) {
        Node *current_node = add_to_database(markov_chain, word_pointer);
        if (current_node == NULL) {
            return true;
        }

        if (prev_word != NULL) {
            add_node_to_frequencies_list(prev_word->data, current_node->data);
        }

        prev_word = current_node;
        word_pointer = strtok(NULL, " ");

        if (*words_to_read != READ_ALL_WORDS) {
            (*words_to_read)--;
        }
    }

    return false;
}

bool fill_database(FILE *fp, int words_to_read, MarkovChain *markov_chain) {
    char sentence_buffer[MAX_SENTENCE_LENGTH + 1];

    while (fgets(sentence_buffer, MAX_SENTENCE_LENGTH, fp) != NULL &&
           ((words_to_read > 0) || words_to_read == READ_ALL_WORDS)) {
        if (sentence_buffer[strlen(sentence_buffer) - 1] == '\n') {
            sentence_buffer[strlen(sentence_buffer) - 1] = '\0';
        }

        if (add_sentence_to_database(markov_chain, sentence_buffer,
                                     &words_to_read)) {
            return true;
        }
    }

    return false;
}

bool parse_arguments(int argc, char *argv[], unsigned int *seed, int
*num_of_tweets, int *num_of_words, FILE **text_corpus_fp) {
    char *end_ptr, *text_corpus_path;
    // we don't check the following arguments, because we can assume they are
    // valid
    *seed = (unsigned int) strtol(argv[SEED_ARG_INDEX], &end_ptr,
                                  DECIMAL_BASE);
    *num_of_tweets =
            (int) strtol(argv[TWEET_COUNT_ARG_INDEX], &end_ptr, DECIMAL_BASE);

    *num_of_words = READ_ALL_WORDS;

    if (argc == ARG_COUNT_WITH_NUM_OF_WORD) {
        *num_of_words =
                (int) strtol(argv[WORD_COUNT_ARG_INDEX], &end_ptr,
                             DECIMAL_BASE);
    }

    text_corpus_path = argv[TEXT_CORPUS_ARG_INDEX];
    *text_corpus_fp = fopen(text_corpus_path, "r");

    /** we have to check that the file could open */
    if (*text_corpus_fp == NULL) {
        printf(ERROR_OPEN_FILE_FMT, text_corpus_path);
        return true;
    }

    return false;
}