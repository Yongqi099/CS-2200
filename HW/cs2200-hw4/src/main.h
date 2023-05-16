/**
 * Name: SAMPLE
 * GTID: 123456789
 * added to pass autograder
 */

/**
 * @file main.h
 * @author Andrej Vrtanoski & Charles Snider
 * @version 1.0
 * @date 2022-02-12
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "arraylist.h"
#include "arraylist_tests.h"

// Due to a bug in the original Solution this length was 16, but it should be 15. Either value will be accepted
int dictionary_length = 15; // Problem 1: student src will have this set to 17
char *dictionary[] = {
    "this",
    "rocks",
    "tea",
    "juice",
    "is",
    "secret",
    "nothing",
    "correct",
    "more",
    "cs2200",
    "than",
    "tunnel",
    "hot",
    "momo",
    "leaf"
};

int main(int argc, char *argv[]);
char *generateMessage();