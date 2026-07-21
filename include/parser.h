/*
 * ================================
 * CS200 Chatbot Memory Manager
 * Skeleton File
 * ================================
 */

#ifndef PARSER_H
#define PARSER_H

/*
 * parser_process
 *
 * Executes the full parsing and moderation pipeline.
 *
 * Ownership Contract:
 *  - On success, returns a heap-allocated buffer.
 *  - The returned buffer is owned by the parser module.
 *  - The caller MUST release it using parser_free().
 *  - On failure or blocked input, returns NULL and
 *    guarantees no memory leaks.
 */
char *parser_process(const char *raw);

/*
 * parser_free
 *
 * Destructor for parser-managed memory.
 * This is the ONLY valid way to release memory
 * returned by parser_process().
 *
 * Safe to call with NULL.
 */
void parser_free(char *text);


/* ===== Pipeline Stages ===== */

char *parse_input(const char *input);
void parser_trim(char *text);
void parser_normalize(char *text);
int  parser_validate(const char *text);

void censor_text(char *text);
int  contains_blocked_word(const char *text);
int  parser_check_policy(const char *text);
void parser_apply_policy(char *text);

/*
 * Debug helper (no side effects).
 */
void parser_debug_dump(const char *stage, const char *text);

#endif