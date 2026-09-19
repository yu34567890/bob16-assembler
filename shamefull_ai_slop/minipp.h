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
 *   % endfunc                     number of call-site arguments (variadic).
 *                                 Declared names can be used directly in
 *                                 `put` lines (whole-word), e.g. `dst`
 *                                 and `src` below both work, as do the
 *                                 positional arg0/arg1 forms - use
 *                                 whichever reads better:
 *
 *                                     % func mov, dst, src
 *                                     put MOV dst, src
 *                                     % endfunc
 *
 *   % repeat
 *   ...body...
 *   % endrepeat                  Only valid inside a % func body. Runs the
 *                                 body once per argument supplied at the
 *                                 call site.
 *
 *   put <text with arg/argN>     Emits the rest of the line literally.
 *                                 Usable both at top level and inside a
 *                                 % func body. Inside a func body, whole-
 *                                 word occurrences of `arg` (current
 *                                 repeat argument) or `arg0`, `arg1`, ...
 *                                 (positional call args) are substituted
 *                                 first. In all cases, known %defines are
 *                                 then applied. Only the first "put " is
 *                                 consumed as the directive, so
 *                                 `put put foo` emits `put foo`.
 *
 *   raw <text>                   Emitted completely verbatim - no %define
 *                                 substitution, no arg substitution.
 *                                 Usable at top level and inside a % func
 *                                 body. Only the first "raw " is consumed.
 *
 *   %fncall NAME args...         Explicitly invoke another already-defined
 *                                 %func (works both at top level and
 *                                 inside another %func's body). Unlike a
 *                                 bare call (see below), this form is
 *                                 recognized even inside a %func body.
 *                                 When used inside a %func body, the
 *                                 argument text first gets the same
 *                                 arg/argN substitution `put` gets, so
 *                                 the outer call's real arguments are
 *                                 forwarded to the nested call, e.g.
 *                                 `%fncall inner arg0`. %defines are then
 *                                 applied before the call is made.
 *                                 Recursive/self-referential %fncall
 *                                 chains are limited to a nesting depth
 *                                 of 256 to catch infinite recursion.
 *
 * Each %func carries its own global usage counter, starting at 0 and
 * incremented by 1 every time it is invoked (bare call or %fncall). The
 * literal token `COUNTER` is substituted with the current count - as a
 * plain substring match, even mid-identifier (e.g. "labelCOUNTER:" ->
 * "label3:") - in `put` lines and plain passthrough lines of that func's
 * body. `raw` lines are exempt (raw is always fully verbatim).
 *
 * Calling a function (NO leading '%'):
 *
 *   datafill 1, 2, 3             Arguments may be separated by spaces
 *   datafill 1 2 3                and/or commas, interchangeably.
 *
 * Comments: '@', ';' or "//" to end of line are stripped, outside of the
 * directive parsing itself.
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
