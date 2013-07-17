#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

/* The following is the size of a buffer to contain any error messages
   encountered when the regular expression is compiled. */

#define MAX_ERROR_MSG 0x1000

/* Compile the regular expression described by "regex_text" into
   "r". */

static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
	char error_message[MAX_ERROR_MSG];
	regerror (status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
        return 1;
    }
    return 0;
}

/*
  Match the string in "to_match" against the compiled regular
  expression in "r".
 */

static int match_regex (regex_t * r, const char * to_match)
{
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 1;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) {
            return nomatch;
	} else {
	  return 0;
	}
}

int main(int argc, char ** argv)
{
  int hola;
    regex_t r;
    const char * regex_text;
    const char * find_text;
    if (argc != 3) {
        regex_text = "(/media|/mnt)";
        find_text = "/home 455719.0MB 144368.0MB /mnt 2707.0MB 0.0MB /sdc1 0.0MB 0.0MB /sdd1 1880.0MB 41.0MB";
    }
    else {
        regex_text = argv[1];
        find_text = argv[2];
    }
    printf ("Trying to find '%s' in '%s'\n", regex_text, find_text);
    compile_regex(& r, regex_text);
    hola = match_regex(& r, find_text);
    printf ("%d\n",hola);
    regfree (& r);
    return 0;
}
