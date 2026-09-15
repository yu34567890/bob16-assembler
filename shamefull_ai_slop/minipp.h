/*
 * minipp.h - A tiny, embeddable macro preprocessor for assemblers.
 *
 * Supported syntax:
 *
 *   % define NAME value          Simple text substitution (object-like only).
 *                                 Every later whole-word occurrence of NAME
 *                                 is replaced with `value`.
 *
 *   % include "path/to/file"     Textually includes another file at this
 *                                 point (recursive includes are detected
 *                                 and rejected).
 *
 *   % if NAME
 *   % ifdef NAME
 *   % ifndef NAME
 *   % else
 *   % endif                      Standard conditional inclusion. `% if` and
 *                                 `% ifdef` are equivalent (both just test
 *                                 "is NAME defined"). `% ifndef` is the
 *                                 negation. Nesting is supported.
 *
 *   % func NAME arg1, arg2, ...  Defines a macro-function. If no args are
 *   ...body...                    named, the function accepts an arbitrary
 *   % endfunc                     number of call-site arguments.
 *
 *   % repeat
 *   ...body...
 *   % endrepeat                  Only valid inside a % func body. Runs the
 *                                 body once per argument supplied at the
 *                                 call site.
 *
 *   put <text with arg/argN>     Whole-word occurrences of `arg` (current
 *                                 repeat argument) or `arg0`, `arg1`, ...
 *                                 (positional call args) are substituted
 *                                 with their actual text, then the line is
 *                                 emitted.
 *
 *   raw <text>                   Emitted verbatim, no substitution at all.
 *
 * Calling a function (NO leading '%'):
 *
 *   datafill 1, 2, 3             Arguments may be separated by spaces
 *   datafill 1 2 3                and/or commas, interchangeably.
 *
 * Comments: '@' or "//" to end of line are stripped (GNU-as style), outside
 * of the directive parsing itself.
 *
 * -----------------------------------------------------------------------
 * Public API
 * -----------------------------------------------------------------------
 */

#ifndef MINIPP_H
#define MINIPP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *text;    /* heap-allocated NUL-terminated output; owned by caller */
    int   ok;      /* 0 on success, non-zero if an error occurred          */
    char *error;   /* heap-allocated error message, or NULL if ok == 0     */
} minipp_result;

/*
 * Preprocess `source` (a NUL-terminated string containing the whole file).
 * `filename` is used only for error messages / relative %include resolution
 * (pass NULL or "" if not applicable).
 *
 * Returns a minipp_result. On success, result.ok == 0 and result.text holds
 * the fully expanded output (caller must free with minipp_free_result).
 * On failure, result.ok != 0, result.text is NULL, and result.error
 * describes the problem.
 */
minipp_result minipp_process(const char *source, const char *filename);

/*
 * Convenience wrapper: reads `filename` from disk, preprocesses it
 * (supporting %include of sibling files), and returns the result the
 * same way as minipp_process.
 */
minipp_result minipp_process_file(const char *filename);

/* Frees the text/error fields inside a minipp_result. Safe to call even
 * if the fields are NULL. */
void minipp_free_result(minipp_result *result);

#ifdef __cplusplus
}
#endif

#endif /* MINIPP_H */
