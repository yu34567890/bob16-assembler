/*
 * minipp.c - implementation of the mini macro-preprocessor (see minipp.h)
 */

#include "minipp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* ------------------------------------------------------------------- *
 *  Small dynamic string buffer
 * ------------------------------------------------------------------- */

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} sb_t;

static void sb_init(sb_t *sb) {
    sb->cap = 4096;
    sb->len = 0;
    sb->data = (char *)malloc(sb->cap);
    sb->data[0] = '\0';
}

static void sb_ensure(sb_t *sb, size_t extra) {
    if (sb->len + extra + 1 > sb->cap) {
        while (sb->len + extra + 1 > sb->cap) sb->cap *= 2;
        sb->data = (char *)realloc(sb->data, sb->cap);
    }
}

static void sb_append(sb_t *sb, const char *text) {
    size_t l = strlen(text);
    sb_ensure(sb, l);
    memcpy(sb->data + sb->len, text, l);
    sb->len += l;
    sb->data[sb->len] = '\0';
}

static void sb_append_line(sb_t *sb, const char *text) {
    sb_append(sb, text);
    sb_append(sb, "\n");
}

static void sb_free(sb_t *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len = sb->cap = 0;
}

/* ------------------------------------------------------------------- *
 *  String helpers
 * ------------------------------------------------------------------- */

static char *xstrdup(const char *s) {
    size_t l = strlen(s);
    char *p = (char *)malloc(l + 1);
    memcpy(p, s, l + 1);
    return p;
}

static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

/* Strip a '@' or "//" style comment, respecting double-quoted strings. */
static char *strip_comment(const char *line) {
    char *out = xstrdup(line);
    int in_quotes = 0;
    for (char *p = out; *p; p++) {
        if (*p == '"') {
            in_quotes = !in_quotes;
        } else if (!in_quotes) {
            if (*p == '@' || *p == ';') {
                *p = '\0';
                break;
            }
            if (*p == '/' && *(p + 1) == '/') {
                *p = '\0';
                break;
            }
        }
    }
    return out;
}

static int is_word_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

/* Replace every whole-word occurrence of `name` in `text` with `value`.
 * Returns a newly-allocated string; caller frees. */
static char *replace_whole_word(const char *text, const char *name, const char *value) {
    size_t name_len = strlen(name);
    if (name_len == 0) return xstrdup(text);

    sb_t out;
    sb_init(&out);

    const char *p = text;
    while (*p) {
        const char *hit = strstr(p, name);
        if (!hit) {
            sb_append(&out, p);
            break;
        }
        int left_ok = (hit == text) || !is_word_char(*(hit - 1));
        int right_ok = !is_word_char(*(hit + name_len));

        if (left_ok && right_ok) {
            char *chunk = (char *)malloc((size_t)(hit - p) + 1);
            memcpy(chunk, p, (size_t)(hit - p));
            chunk[hit - p] = '\0';
            sb_append(&out, chunk);
            free(chunk);
            sb_append(&out, value);
            p = hit + name_len;
        } else {
            /* not a whole-word match; copy one char and keep scanning */
            char one[2] = { *(hit), '\0' };
            char *chunk = (char *)malloc((size_t)(hit - p) + 2);
            memcpy(chunk, p, (size_t)(hit - p));
            chunk[hit - p] = *hit;
            chunk[hit - p + 1] = '\0';
            sb_append(&out, chunk);
            free(chunk);
            (void)one;
            p = hit + 1;
        }
    }

    char *result = xstrdup(out.data);
    sb_free(&out);
    return result;
}

/* ------------------------------------------------------------------- *
 *  Line list (splitting a whole source buffer into lines)
 * ------------------------------------------------------------------- */

typedef struct {
    char **lines;
    int count;
    int cap;
} line_list;

static void ll_init(line_list *ll) {
    ll->cap = 64;
    ll->count = 0;
    ll->lines = (char **)malloc(sizeof(char *) * ll->cap);
}

static void ll_push(line_list *ll, const char *line) {
    if (ll->count >= ll->cap) {
        ll->cap *= 2;
        ll->lines = (char **)realloc(ll->lines, sizeof(char *) * ll->cap);
    }
    ll->lines[ll->count++] = xstrdup(line);
}

static void ll_free(line_list *ll) {
    for (int i = 0; i < ll->count; i++) free(ll->lines[i]);
    free(ll->lines);
    ll->lines = NULL;
    ll->count = ll->cap = 0;
}

static void split_lines(const char *source, line_list *out) {
    ll_init(out);
    const char *p = source;
    sb_t cur;
    sb_init(&cur);
    while (*p) {
        if (*p == '\r') { p++; continue; }
        if (*p == '\n') {
            ll_push(out, cur.data);
            cur.len = 0;
            cur.data[0] = '\0';
            p++;
            continue;
        }
        char one[2] = { *p, '\0' };
        sb_append(&cur, one);
        p++;
    }
    if (cur.len > 0) ll_push(out, cur.data);
    sb_free(&cur);
}

/* ------------------------------------------------------------------- *
 *  Token splitting: split on whitespace and/or commas
 * ------------------------------------------------------------------- */

typedef struct {
    char **items;
    int count;
    int cap;
} str_list;

static void sl_init(str_list *sl) {
    sl->cap = 8;
    sl->count = 0;
    sl->items = (char **)malloc(sizeof(char *) * sl->cap);
}

static void sl_push(str_list *sl, const char *s) {
    if (sl->count >= sl->cap) {
        sl->cap *= 2;
        sl->items = (char **)realloc(sl->items, sizeof(char *) * sl->cap);
    }
    sl->items[sl->count++] = xstrdup(s);
}

static void sl_free(str_list *sl) {
    for (int i = 0; i < sl->count; i++) free(sl->items[i]);
    free(sl->items);
    sl->items = NULL;
    sl->count = sl->cap = 0;
}

/* Splits `text` on whitespace and/or commas (either/both act as
 * separators); empty tokens are skipped. */
static void split_args(const char *text, str_list *out) {
    sl_init(out);
    sb_t cur;
    sb_init(&cur);
    const char *p = text;
    while (1) {
        if (*p == ' ' || *p == '\t' || *p == ',' || *p == '\0') {
            char *t = trim(cur.data);
            if (*t) sl_push(out, t);
            cur.len = 0;
            cur.data[0] = '\0';
            if (*p == '\0') break;
            p++;
            continue;
        }
        char one[2] = { *p, '\0' };
        sb_append(&cur, one);
        p++;
    }
    sb_free(&cur);
}

/* ------------------------------------------------------------------- *
 *  Defines table
 * ------------------------------------------------------------------- */

typedef struct {
    char *name;
    char *value;
} define_t;

typedef struct {
    define_t *items;
    int count;
    int cap;
} define_table;

static void dt_init(define_table *t) {
    t->cap = 16;
    t->count = 0;
    t->items = (define_t *)malloc(sizeof(define_t) * t->cap);
}

static int dt_find(define_table *t, const char *name) {
    for (int i = 0; i < t->count; i++)
        if (strcmp(t->items[i].name, name) == 0) return i;
    return -1;
}

static void dt_set(define_table *t, const char *name, const char *value) {
    int idx = dt_find(t, name);
    if (idx >= 0) {
        free(t->items[idx].value);
        t->items[idx].value = xstrdup(value);
        return;
    }
    if (t->count >= t->cap) {
        t->cap *= 2;
        t->items = (define_t *)realloc(t->items, sizeof(define_t) * t->cap);
    }
    t->items[t->count].name = xstrdup(name);
    t->items[t->count].value = xstrdup(value);
    t->count++;
}

static void dt_free(define_table *t) {
    for (int i = 0; i < t->count; i++) {
        free(t->items[i].name);
        free(t->items[i].value);
    }
    free(t->items);
    t->items = NULL;
    t->count = t->cap = 0;
}

/* Apply every known define (whole-word) to `text`. Returns newly
 * allocated string. */
static char *apply_defines(define_table *t, const char *text) {
    char *cur = xstrdup(text);
    for (int i = 0; i < t->count; i++) {
        char *next = replace_whole_word(cur, t->items[i].name, t->items[i].value);
        free(cur);
        cur = next;
    }
    return cur;
}

/* ------------------------------------------------------------------- *
 *  Functions table (% func ... % endfunc)
 * ------------------------------------------------------------------- */

typedef struct {
    char *name;
    int variadic;          /* 1 if declared with no argument names */
    str_list arg_names;    /* declared argument names (informational) */
    line_list body;        /* raw, comment-stripped body lines */
} func_t;

typedef struct {
    func_t *items;
    int count;
    int cap;
} func_table;

static void ft_init(func_table *t) {
    t->cap = 8;
    t->count = 0;
    t->items = (func_t *)malloc(sizeof(func_t) * t->cap);
}

static func_t *ft_find(func_table *t, const char *name) {
    for (int i = 0; i < t->count; i++)
        if (strcmp(t->items[i].name, name) == 0) return &t->items[i];
    return NULL;
}

static func_t *ft_add(func_table *t, const char *name) {
    if (t->count >= t->cap) {
        t->cap *= 2;
        t->items = (func_t *)realloc(t->items, sizeof(func_t) * t->cap);
    }
    func_t *f = &t->items[t->count++];
    f->name = xstrdup(name);
    f->variadic = 0;
    sl_init(&f->arg_names);
    /* f->body is deliberately left un-initialized here: the caller
     * (process_lines, %func handling) always assigns a freshly-built
     * line_list into f->body right after calling ft_add. */
    return f;
}

static void ft_free(func_table *t) {
    for (int i = 0; i < t->count; i++) {
        free(t->items[i].name);
        sl_free(&t->items[i].arg_names);
        ll_free(&t->items[i].body);
    }
    free(t->items);
    t->items = NULL;
    t->count = t->cap = 0;
}

/* ------------------------------------------------------------------- *
 *  Conditional (%if/%ifdef/%ifndef/%else/%endif) stack
 * ------------------------------------------------------------------- */

#define MAX_COND_DEPTH 64
#define MAX_INCLUDE_DEPTH 64

typedef struct {
    int parent_active; /* was the enclosing scope active? */
    int condition;      /* the branch condition's truth value */
    int taken;           /* has a true branch already been emitted? */
} cond_frame;

typedef struct {
    cond_frame stack[MAX_COND_DEPTH];
    int depth;
} cond_stack;

static void cs_init(cond_stack *cs) { cs->depth = 0; }

static int cs_active(cond_stack *cs) {
    if (cs->depth == 0) return 1;
    cond_frame *f = &cs->stack[cs->depth - 1];
    return f->parent_active && f->condition;
}

/* ------------------------------------------------------------------- *
 *  Global processing state
 * ------------------------------------------------------------------- */

typedef struct {
    define_table defines;
    func_table funcs;
    sb_t output;
    cond_stack conds;
    char *include_stack[MAX_INCLUDE_DEPTH];
    int include_depth;
    int error;
    char error_msg[512];
} pp_state;

static void pp_error(pp_state *st, const char *fmt, ...) {
    if (st->error) return; /* keep first error */
    st->error = 1;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(st->error_msg, sizeof(st->error_msg), fmt, ap);
    va_end(ap);
}

/* forward decl */
static void process_lines(pp_state *st, line_list *ll, const char *cur_dir);

/* ------------------------------------------------------------------- *
 *  Emitting a single body line inside a func expansion.
 *  call_args: the arguments given at the call site.
 *  current_idx/current_val: set when inside a %repeat (the "current"
 *  iteration argument, referenced bare as `arg`); current_val == NULL
 *  when not inside a repeat.
 * ------------------------------------------------------------------- */

static void emit_func_body_line(pp_state *st, const char *raw_line,
                                 str_list *call_args, const char *current_val) {
    char *line = xstrdup(raw_line);
    char *t = trim(line);

    if (strncmp(t, "put ", 4) == 0 || strcmp(t, "put") == 0) {
        const char *rest = (strlen(t) > 3) ? t + 4 : "";
        char *text = xstrdup(rest);

        /* Substitute positional arg0, arg1, ... first (longer/specific). */
        for (int i = 0; i < call_args->count; i++) {
            char name[32];
            snprintf(name, sizeof(name), "arg%d", i);
            char *next = replace_whole_word(text, name, call_args->items[i]);
            free(text);
            text = next;
        }
        /* Then substitute bare `arg` with the current repeat value, if any. */
        if (current_val) {
            char *next = replace_whole_word(text, "arg", current_val);
            free(text);
            text = next;
        }
        /* Finally apply top-level %defines. */
        char *final_text = apply_defines(&st->defines, text);
        free(text);
        sb_append_line(&st->output, final_text);
        free(final_text);

    } else if (strncmp(t, "raw ", 4) == 0 || strcmp(t, "raw") == 0) {
        const char *rest = (strlen(t) > 3) ? t + 4 : "";
        sb_append_line(&st->output, rest);

    } else if (t[0] == '%') {
        pp_error(st, "unsupported directive inside func body: '%s'", t);
    } else if (*t == '\0') {
        /* blank line inside body: ignore */
    } else {
        /* plain passthrough line: only %defines applied, no arg substitution */
        char *final_text = apply_defines(&st->defines, t);
        sb_append_line(&st->output, final_text);
        free(final_text);
    }

    free(line);
}

/* Expand a call to a previously-defined %func. */
static void expand_func_call(pp_state *st, func_t *f, str_list *call_args) {
    if (!f->variadic && call_args->count != f->arg_names.count) {
        pp_error(st, "func '%s' expects %d argument(s), got %d",
                  f->name, f->arg_names.count, call_args->count);
        return;
    }

    for (int i = 0; i < f->body.count && !st->error; i++) {
        char *line = xstrdup(f->body.lines[i]);
        char *t = trim(line);

        if (strcmp(t, "% repeat") == 0 || strcmp(t, "%repeat") == 0) {
            /* collect the repeat body until % endrepeat */
            line_list rbody;
            ll_init(&rbody);
            i++;
            int closed = 0;
            for (; i < f->body.count; i++) {
                char *rl = xstrdup(f->body.lines[i]);
                char *rt = trim(rl);
                if (strcmp(rt, "% endrepeat") == 0 || strcmp(rt, "%endrepeat") == 0) {
                    free(rl);
                    closed = 1;
                    break;
                }
                ll_push(&rbody, f->body.lines[i]);
                free(rl);
            }
            if (!closed) {
                pp_error(st, "func '%s': %%repeat without matching %%endrepeat", f->name);
                ll_free(&rbody);
                free(line);
                break;
            }
            for (int a = 0; a < call_args->count && !st->error; a++) {
                for (int r = 0; r < rbody.count && !st->error; r++) {
                    emit_func_body_line(st, rbody.lines[r], call_args, call_args->items[a]);
                }
            }
            ll_free(&rbody);

        } else if (strcmp(t, "% endrepeat") == 0 || strcmp(t, "%endrepeat") == 0) {
            pp_error(st, "func '%s': stray %%endrepeat", f->name);

        } else {
            emit_func_body_line(st, f->body.lines[i], call_args, NULL);
        }

        free(line);
    }
}

/* ------------------------------------------------------------------- *
 *  Directive parsing helpers
 * ------------------------------------------------------------------- */

/* Returns 1 and fills `word` with the first identifier-ish token of
 * `s`, and sets *rest to the remainder (trimmed). */
static int take_first_word(const char *s, char *word, size_t word_sz, const char **rest) {
    while (*s && isspace((unsigned char)*s)) s++;
    size_t i = 0;
    while (*s && !isspace((unsigned char)*s) && i + 1 < word_sz) {
        word[i++] = *s++;
    }
    word[i] = '\0';
    while (*s && isspace((unsigned char)*s)) s++;
    *rest = s;
    return i > 0;
}

static char *dirname_of(const char *path) {
    const char *slash = strrchr(path, '/');
    if (!slash) return xstrdup(".");
    size_t len = (size_t)(slash - path);
    char *d = (char *)malloc(len + 1);
    memcpy(d, path, len);
    d[len] = '\0';
    return d;
}

static char *join_path(const char *dir, const char *file) {
    if (file[0] == '/') return xstrdup(file);
    size_t dl = strlen(dir), fl = strlen(file);
    char *out = (char *)malloc(dl + fl + 2);
    memcpy(out, dir, dl);
    out[dl] = '/';
    memcpy(out + dl + 1, file, fl + 1);
    return out;
}

static char *read_whole_file(const char *path, pp_state *st) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        pp_error(st, "could not open include file '%s'", path);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz < 0) sz = 0;
    char *buf = (char *)malloc((size_t)sz + 1);
    size_t rd = fread(buf, 1, (size_t)sz, fp);
    buf[rd] = '\0';
    fclose(fp);
    return buf;
}

/* ------------------------------------------------------------------- *
 *  Main line-processing loop
 * ------------------------------------------------------------------- */

static void process_lines(pp_state *st, line_list *ll, const char *cur_dir) {
    for (int i = 0; i < ll->count && !st->error; i++) {
        char *stripped = strip_comment(ll->lines[i]);
        char *t = trim(stripped);

        if (*t == '\0') { free(stripped); continue; }

        int active = cs_active(&st->conds);

        if (t[0] == '%') {
            const char *after_pct = t + 1;
            char keyword[64];
            const char *rest;
            take_first_word(after_pct, keyword, sizeof(keyword), &rest);

            if (strcmp(keyword, "define") == 0) {
                if (active) {
                    char name[128];
                    const char *value;
                    take_first_word(rest, name, sizeof(name), &value);
                    if (name[0] == '\0') {
                        pp_error(st, "%%define missing a name");
                    } else {
                        dt_set(&st->defines, name, value);
                    }
                }

            } else if (strcmp(keyword, "include") == 0) {
                if (active) {
                    const char *q1 = strchr(rest, '"');
                    const char *q2 = q1 ? strchr(q1 + 1, '"') : NULL;
                    if (!q1 || !q2) {
                        pp_error(st, "%%include expects a \"quoted\" path");
                    } else {
                        size_t len = (size_t)(q2 - q1 - 1);
                        char *relpath = (char *)malloc(len + 1);
                        memcpy(relpath, q1 + 1, len);
                        relpath[len] = '\0';

                        char *full = join_path(cur_dir, relpath);
                        free(relpath);

                        int already = 0;
                        for (int k = 0; k < st->include_depth; k++)
                            if (strcmp(st->include_stack[k], full) == 0) already = 1;

                        if (already) {
                            pp_error(st, "circular %%include detected: '%s'", full);
                        } else if (st->include_depth >= MAX_INCLUDE_DEPTH) {
                            pp_error(st, "%%include nesting too deep (possible cycle)");
                        } else {
                            char *content = read_whole_file(full, st);
                            if (content) {
                                st->include_stack[st->include_depth++] = xstrdup(full);
                                line_list sub;
                                split_lines(content, &sub);
                                char *sub_dir = dirname_of(full);
                                process_lines(st, &sub, sub_dir);
                                free(sub_dir);
                                ll_free(&sub);
                                free(content);
                                st->include_depth--;
                                free(st->include_stack[st->include_depth]);
                            }
                        }
                        free(full);
                    }
                }

            } else if (strcmp(keyword, "if") == 0 || strcmp(keyword, "ifdef") == 0) {
                char name[128];
                const char *dummy;
                take_first_word(rest, name, sizeof(name), &dummy);
                if (st->conds.depth >= MAX_COND_DEPTH) {
                    pp_error(st, "%%if nesting too deep");
                } else {
                    int is_def = dt_find(&st->defines, name) >= 0;
                    cond_frame f;
                    f.parent_active = active;
                    f.condition = is_def;
                    f.taken = is_def;
                    st->conds.stack[st->conds.depth++] = f;
                }

            } else if (strcmp(keyword, "ifndef") == 0) {
                char name[128];
                const char *dummy;
                take_first_word(rest, name, sizeof(name), &dummy);
                if (st->conds.depth >= MAX_COND_DEPTH) {
                    pp_error(st, "%%if nesting too deep");
                } else {
                    int is_def = dt_find(&st->defines, name) >= 0;
                    cond_frame f;
                    f.parent_active = active;
                    f.condition = !is_def;
                    f.taken = !is_def;
                    st->conds.stack[st->conds.depth++] = f;
                }

            } else if (strcmp(keyword, "else") == 0) {
                if (st->conds.depth == 0) {
                    pp_error(st, "%%else without matching %%if");
                } else {
                    cond_frame *f = &st->conds.stack[st->conds.depth - 1];
                    f->condition = !f->taken;
                    f->taken = f->taken || f->condition;
                }

            } else if (strcmp(keyword, "endif") == 0) {
                if (st->conds.depth == 0) {
                    pp_error(st, "%%endif without matching %%if");
                } else {
                    st->conds.depth--;
                }

            } else if (strcmp(keyword, "func") == 0) {
                /* Split the whole declaration (name + arg list) on
                 * whitespace/commas so `% func psh, r` and
                 * `% func psh r` behave identically. */
                str_list decl;
                split_args(rest, &decl);
                char name[128];
                name[0] = '\0';
                if (decl.count > 0) {
                    strncpy(name, decl.items[0], sizeof(name) - 1);
                    name[sizeof(name) - 1] = '\0';
                }
                if (name[0] == '\0') {
                    sl_free(&decl);
                    pp_error(st, "%%func missing a name");
                    free(stripped);
                    continue;
                }
                /* remaining decl items (after the name) are arg names */
                str_list names;
                sl_init(&names);
                for (int k = 1; k < decl.count; k++) sl_push(&names, decl.items[k]);
                sl_free(&decl);

                /* Consume until matching % endfunc regardless of `active`,
                 * to keep the line index in sync. */
                line_list body;
                ll_init(&body);
                int closed = 0;
                int j = i + 1;
                for (; j < ll->count; j++) {
                    char *bl = strip_comment(ll->lines[j]);
                    char *bt = trim(bl);
                    if (strcmp(bt, "% endfunc") == 0 || strcmp(bt, "%endfunc") == 0) {
                        free(bl);
                        closed = 1;
                        break;
                    }
                    ll_push(&body, bl);
                    free(bl);
                }
                if (!closed) {
                    pp_error(st, "%%func '%s' missing matching %%endfunc", name);
                    ll_free(&body);
                    free(stripped);
                    break;
                }
                i = j; /* skip past consumed body + endfunc line */

                if (active) {
                    if (ft_find(&st->funcs, name)) {
                        pp_error(st, "func '%s' already defined", name);
                        ll_free(&body);
                        sl_free(&names);
                    } else {
                        func_t *f = ft_add(&st->funcs, name);
                        if (names.count == 0) {
                            f->variadic = 1;
                        } else {
                            f->variadic = 0;
                            for (int k = 0; k < names.count; k++)
                                sl_push(&f->arg_names, names.items[k]);
                        }
                        sl_free(&names);
                        f->body = body; /* transfer ownership */
                    }
                } else {
                    ll_free(&body);
                    sl_free(&names);
                }

            } else if (strcmp(keyword, "endfunc") == 0) {
                pp_error(st, "stray %%endfunc");

            } else if (strcmp(keyword, "repeat") == 0 || strcmp(keyword, "endrepeat") == 0) {
                pp_error(st, "%%%s used outside of a %%func body", keyword);

            } else {
                pp_error(st, "unknown directive '%%%s'", keyword);
            }

        } else {
            /* Not a directive: either `put`/`raw` (now valid at top level
             * too), a func call, or a plain line. */
            if (active) {
                char name[128];
                const char *rest;
                take_first_word(t, name, sizeof(name), &rest);

                if (strcmp(name, "put") == 0) {
                    /* Emit everything after the first "put " literally,
                     * with %defines applied (but no argN/arg substitution
                     * - there's no call context at top level). */
                    char *final_text = apply_defines(&st->defines, rest);
                    sb_append_line(&st->output, final_text);
                    free(final_text);

                } else if (strcmp(name, "raw") == 0) {
                    /* Emit everything after the first "raw " completely
                     * verbatim - no %define substitution at all. */
                    sb_append_line(&st->output, rest);

                } else {
                    func_t *f = name[0] ? ft_find(&st->funcs, name) : NULL;
                    if (f) {
                        str_list call_args;
                        split_args(rest, &call_args);
                        expand_func_call(st, f, &call_args);
                        sl_free(&call_args);
                    } else {
                        char *final_text = apply_defines(&st->defines, t);
                        sb_append_line(&st->output, final_text);
                        free(final_text);
                    }
                }
            }
        }

        free(stripped);
    }
}

/* ------------------------------------------------------------------- *
 *  Public API
 * ------------------------------------------------------------------- */

static minipp_result finish(pp_state *st) {
    minipp_result r;
    if (st->error) {
        r.ok = 1;
        r.text = NULL;
        r.error = xstrdup(st->error_msg);
    } else if (st->conds.depth != 0) {
        r.ok = 1;
        r.text = NULL;
        r.error = xstrdup("unterminated %if/%ifdef/%ifndef (missing %endif)");
    } else {
        r.ok = 0;
        r.text = xstrdup(st->output.data);
        r.error = NULL;
    }
    dt_free(&st->defines);
    ft_free(&st->funcs);
    sb_free(&st->output);
    for (int i = 0; i < st->include_depth; i++) free(st->include_stack[i]);
    return r;
}

minipp_result minipp_process(const char *source, const char *filename) {
    pp_state st;
    dt_init(&st.defines);
    ft_init(&st.funcs);
    sb_init(&st.output);
    cs_init(&st.conds);
    st.include_depth = 0;
    st.error = 0;
    st.error_msg[0] = '\0';

    char *dir = dirname_of(filename && filename[0] ? filename : "./x");

    line_list ll;
    split_lines(source, &ll);
    process_lines(&st, &ll, dir);
    ll_free(&ll);
    free(dir);

    return finish(&st);
}

minipp_result minipp_process_file(const char *filename) {
    pp_state st;
    dt_init(&st.defines);
    ft_init(&st.funcs);
    sb_init(&st.output);
    cs_init(&st.conds);
    st.include_depth = 0;
    st.error = 0;
    st.error_msg[0] = '\0';

    char *content = read_whole_file(filename, &st);
    if (!content) {
        return finish(&st);
    }

    st.include_stack[st.include_depth++] = xstrdup(filename);
    char *dir = dirname_of(filename);

    line_list ll;
    split_lines(content, &ll);
    process_lines(&st, &ll, dir);
    ll_free(&ll);
    free(dir);
    free(content);

    return finish(&st);
}

void minipp_free_result(minipp_result *result) {
    if (!result) return;
    free(result->text);
    free(result->error);
    result->text = NULL;
    result->error = NULL;
}
