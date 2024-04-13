/*
 * Include the provided hashtable library.
 */
#include "hashtable.h"

/*
 * Include the header file.
 */
#include "philspel.h"

/*
 * Standard I/O and file routines.
 */
#include <stdio.h>

/*
 * General utility routines (including malloc()).
 */
#include <stdlib.h>

/*
 * Character utility routines.
 */
#include <ctype.h>

/*
 * String utility routines.
 */
#include <string.h>

/*
 * this hashtable stores the dictionary.  For this purpose you really
 * want to just use a set: "is a word in the dictionary or not", so
 * for storing data the keys should strings that represent valid words
 * and the associated data should also be the same string.
 */
HashTable *dictionary;

/*
 * the MAIN routine.  You can safely print debugging information
 * to standard error (stderr) and it will be ignored in the grading
 * process, in the same way which this does.
 */
int main(int argc, char **argv) {

  // make the dictionary
  dictionary = createHashTable(999, stringHash, stringEquals);
  readDictionary(argv[1]);

  // run processInput
  processInput();

  // free the dictionary
  freeTable(dictionary);
  return 0;
}

/*
 * You need to define this function. void *s can be safely casted
 * to a char * (NULL terminated string) which is done for you here for
 * convenience.
 */
unsigned int stringHash(void *s) {
  char *string = (char *)s;

  // credit to djb2 by Daniel J. Bernstein for this hash function from 1991
  unsigned long hash = 5381;
  int c;

  while ((c = *string++)){
    hash = ((hash << 5) + hash) + c;
  }

  return hash;
}

/*
 * You need to define this function.  It should return a nonzero
 * value if the two strings are identical (case sensitive comparison)
 * and 0 otherwise.
 */
int stringEquals(void *s1, void *s2) {
  char *string1 = (char *)s1;
  char *string2 = (char *)s2;
  // check if string 1 is equal to string 2
  if (strcmp(string1, string2) == 0){
    // return true if so
    return (1);
  }
  // otherwise return false
  else{
    return (0);
  }
}

/*
 * this function should read in every word in the dictionary and
 * store it in the dictionary.  You should first open the file specified,
 * then read the words one at a time and insert them into the dictionary.
 * Once the file is read in completely, exit.  You will need to allocate
 * (using malloc()) space for each word.  As described in the specs, you
 * can initially assume that no word is longer than 60 characters.  However,
 * for the final 20% of your grade, you cannot assumed that words have a bounded
 * length.  You can NOT assume that the specified file exists.  If the file does
 * NOT exist, you should print some message to standard error and call exit(0)
 * to cleanly exit the program.
 *
 * Since the format is one word at a time, with returns in between,
 * you can safely use fscanf() to read in the strings until you want to handle
 * arbitrarily long dictionary chacaters.
 */
void readDictionary(char *filename) {
  // initialization
  FILE *f;
  char *store;
  char *temp;
  store = (char*) malloc(9999*sizeof(char));

  // open the file
  f = fopen(filename, "r");

  // if the file doesn't exist
  if (f == NULL){
    // print an error and exit
    fprintf(stderr, "File doesn't exist\n");
    free(store);
    exit(0);
  }

  // read each line in the dictionary
  while (fscanf(f, "%s", store) != EOF) {
    // make a copy to be inserted into the dictionary
    temp = (char *)malloc(2*strlen(store)*sizeof(char));
    strcpy(temp, store);
    // add the key/value pair to the dictionary
    insertData(dictionary, temp, temp);
  }

  // free memory
  free(store);

  // close the file
  fclose(f);
}

/*
 * This should process standard input and copy it to standard output
 * as specified in the specs.  EG, if a standard dictionary was used
 * and the string "this is a taest of  this-proGram" was given to
 * standard input, the output to standard output (stdout) should be
 * "this is a teast [sic] of  this-proGram".  All words should be checked
 * against the dictionary as they are input, again with all but the first
 * letter converted to lowercase, and finally with all letters converted
 * to lowercase.  Only if all 3 cases are not in the dictionary should it
 * be reported as not being found, by appending " [sic]" after the
 * error.
 *
 * Since we care about preserving whitespace, and pass on all non alphabet
 * characters untouched, and with all non alphabet characters acting as
 * word breaks, scanf() is probably insufficent (since it only considers
 * whitespace as breaking strings), so you will probably have
 * to get characters from standard input one at a time.
 *
 * As stated in the specs, you can initially assume that no word is longer than
 * 60 characters, but you may have strings of non-alphabetic characters (eg,
 * numbers, punctuation) which are longer than 60 characters. For the final 20%
 * of your grade,  you can no longer assume words have a bounded length.
 */
void processInput() {
  // initialization
  int cha;
  char *arr;
  char *temp;
  int i = 0;
  int k = 0;
  int capacity = 61;

  // allocate space
  arr = (char*) malloc(capacity*sizeof(char));

  // while there are characters in standard input
  while ((cha = getchar()) != EOF){
    // check if the character is non-alphabetic
    if (!isalpha(cha)){
        // check if there is anything in the character array
        if (i > 0){
          // add a terminator to the string
          arr[i] = '\0';
          // print the word
          printf("%s", arr);

          // check if the word is in the dictionary
          if (findData(dictionary, arr) != NULL){
            k++;
          }

          // change all but the first letter to lowercase and check again
          for (int j = 1; arr[j] != '\0';j++){
            arr[j] = tolower(arr[j]);
          }
          if (findData(dictionary, arr) != NULL){
            k++;
          }

          // change the first letter to lowercase as well and check again
          arr[0] = tolower(arr[0]);
          if (findData(dictionary, arr) != NULL){
            k++;
          }
        }

		// check if k fulfills any conditions
        if (k >= 1){
            // if so, empty the array
            memset(arr, 0, strlen(arr));
        }

        // otherwise if there is something in the array, print out the word with " [sic]" and empty the array
        else if (k == 0 && i > 0){
            printf("%s", " [sic]");
            memset(arr, 0, strlen(arr));
        }

        // print the character
        printf("%c", (char) cha);

        // reset the index
        capacity = 61;
        i = 0;
        k = 0;
    }

    else{
        // check if space needs to be increased
        if (i >= capacity - 10){
		    // reallocate space to fit more
		    capacity += 10;
		    arr = realloc(arr, capacity*sizeof(char));
	    }

	    // store the character if it's not a number or whitespace
	    arr[i] = (char) cha;

	    // increase i by 1
	    i++;
    }
  }

  // print out what's left in the array at the end of the file
  // add a terminator to the string
  arr[i] = '\0';
  // print the word
  printf("%s", arr);

  // check if the word is in the dictionary
  if (findData(dictionary, arr) != NULL){
    k++;
  }

  // change all but the first letter to lowercase and check again
  for (int j = 1; arr[j] != '\0';j++){
    arr[j] = tolower(arr[j]);
  }
  if (findData(dictionary, arr) != NULL){
    k++;
  }

  // change the first letter to lowercase as well and check again
  arr[0] = tolower(arr[0]);
  if (findData(dictionary, arr) != NULL){
    k++;
  }

  // if there is something in the array, print out the word with " [sic]"
  else if (k == 0 && i > 0){
    printf("%s", " [sic]");
  }

  // free memory
  free(arr);
}