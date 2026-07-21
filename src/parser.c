/*
 * ================================
 * CS200 Chatbot Memory Manager
 * Parser / Moderation Layer
 * ================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"



/*
 * parse_input
 *
 * Creates an owned, cleaned copy of raw input.
 */
char *parse_input(const char *input)
{

    if(input == NULL)
    {
        return NULL;
    }

    char* userinput = malloc(strlen(input) + 1);

    if(userinput == NULL)
    {
        return NULL;
    }

    strcpy(userinput,input);

    parser_trim(userinput);
    parser_normalize(userinput);
    parser_trim(userinput);

    return userinput;
}


/*
 * parser_trim
 *
 * Removes surrounding whitespace in-place.
 */
void parser_trim(char *text)
{

    if(text == NULL)
    {
        return;
    }

    size_t start = 0;
    size_t end = strlen(text);

    while(start < strlen(text) && isspace(text[start]))
    {
        start++;
    }

    while(end > start && isspace(text[end - 1]))
    {
        end--;
    }

    size_t length = end - start;

    memmove(text,text + start,length);

    text[length] = '\0';
}


/*
 * parser_normalize
 *
 * Normalizes internal formatting.
 */
void parser_normalize(char *text)
{

    if(text == NULL) 
    {
        return;
    }

    int i = 0;
    int j = 0;
    int space = 0;

    while(text[i])
    {
        if(isprint(text[i]) == 0)
        {
            i++;
            continue;
        }
        if(isspace(text[i]) != 0)
        {
            if(space == 0)
            {
                text[j++] = ' ';
                space = 1;
            }
        }
        else
        {
            text[j++] = text[i];
            space = 0;
        }
        i++;
    }
    text[j] = '\0';
}


/*
 * parser_validate
 *
 * Checks whether text is structurally valid.
 */
int parser_validate(const char *text)
{

    if(text == NULL)
    {
        return -1;
    }

    if(strlen(text) == 0)
    {
        return 0;
    }

    for(int i = 0; text[i]; i++)
    {
        if(isprint(text[i]) == 0)
        {
            return 0;
        }
    }
    return 1;
}


/*
 * censor_text
 *
 * Applies censorship rules in-place.
 */
void censor_text(char *text)
{
    if(text == NULL)
    {
        return;
    }

    char* lowertext = strdup(text);

    if(lowertext == NULL)
    {
        return;
    }

    for(int i = 0; lowertext[i]; i++)
    {
        lowertext[i] = tolower(lowertext[i]);
    }

    char* p = strstr(lowertext,"cheating");

    int index = 0;
    while(p)
    {
        index = p - lowertext;
        memcpy(text + index, "********", 8);
        p = strstr(lowertext + index + 8,"cheating");
    }
    free(lowertext);
}


/*
 * contains_blocked_word
 *
 * Detects prohibited content.
 */
int contains_blocked_word(const char *text)
{

    if(text == NULL)
    {
        return 0;
    }

    const char* blocked[] = {"plagiarism","plagiarise","plagiarising",NULL};

    for(int i = 0; blocked[i]; i++)
    {
        char* lowertext = strdup(text);
        char* lowerword = strdup(blocked[i]);

        for(int j = 0; lowertext[j]; j++)
        {
            lowertext[j] = tolower(lowertext[j]);
        }

        if(strstr(lowertext,lowerword))
        {
            free(lowertext);
            free(lowerword);
            return 1;
        }

        free(lowertext);
        free(lowerword);
    }
    return 0;
}


/*
 * parser_check_policy
 *
 * Decides whether a message is allowed.
 */
int parser_check_policy(const char *text)
{

    if(text == NULL)
    {
        return -1;
    }

    if(contains_blocked_word(text))
    {
        return 0;
    }

    return 1;
}


/*
 * parser_apply_policy
 *
 * Applies non-blocking transformations.
 */
void parser_apply_policy(char *text)
{

    if(text == NULL)
    {
        return;
    }

    if(parser_check_policy(text))
    {
        censor_text(text);
        parser_trim(text);
        parser_normalize(text);
        parser_trim(text);
    }
}


/*
 * parser_process
 *
 * Executes the full pipeline.
 */
char *parser_process(const char *raw)
{

    if(raw == NULL)
    {
        return NULL;
    }

    char* input = parse_input(raw);

    if(input == NULL)
    {
        return NULL;
    }

    if(parser_validate(input) <= 0) 
    {
        parser_free(input);
        return NULL;
    }

    if(parser_check_policy(input) <= 0)
    {
        parser_free(input);
        return NULL;
    }

    parser_apply_policy(input);

    return input;
}


/*
 * parser_free
 *
 * Destructor for parser-managed memory.
 */
void parser_free(char *text)
{

    free(text);
}


/*
 * parser_debug_dump
 *
 * Displays intermediate state (debug only).
 */
void parser_debug_dump(const char *stage, const char *text)
{

    if(stage == NULL || text == NULL)
    {
        return;
    }

    printf("[%s] : \"%s\"\n",stage,text);
}