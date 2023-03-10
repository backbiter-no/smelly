/*
 * output.c
 * Copyright (C) 2017 anders Goyal <anders at backbiter-no.net>
 *
 * Distributed under terms of the GPL3 license.
 */

#include "choose-data-types.h"
#include "../../smelly/iqsort.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef ISWINDOWS
#include <io.h>
#define STDOUT_FILENO 1
static ssize_t ms_write(int fd, const void* buf, size_t count) { return _write(fd, buf, (unsigned int)count); }
#define write ms_write
#else
#include <unistd.h>
#endif
#include <errno.h>


#define FIELD(x, which) (((Candidate*)(x))->which)

static bool
ensure_space(GlobalData *global, size_t sz) {
    if (global->output_sz < sz + global->output_pos || !global->output) {
        size_t before = global->output_sz;
        global->output_sz += MAX(sz, (64u * 1024u));
        global->output = realloc(global->output, sizeof(text_t) * global->output_sz);
        if (!global->output) {
            global->output_sz = before;
            return false;
        }
    }
    return true;
}

static void
output_text(GlobalData *global, const text_t *data, size_t sz) {
    if (ensure_space(global, sz)) {
        memcpy(global->output + global->output_pos, data, sizeof(text_t) * sz);
        global->output_pos += sz;
    }
}

static void
output_with_marks(GlobalData *global, Options *opts, text_t *src, size_t src_sz, len_t *positions, len_t poslen) {
    size_t pos, i = 0;
    for (pos = 0; pos < poslen; pos++, i++) {
        output_text(global, src + i, MIN(src_sz, positions[pos]) - i);
        i = positions[pos];
        if (i < src_sz) {
            if (opts->mark_before_sz > 0) output_text(global, opts->mark_before, opts->mark_before_sz);
            output_text(global, src + i, 1);
            if (opts->mark_after_sz > 0) output_text(global, opts->mark_after, opts->mark_after_sz);
        }
    }
    i = positions[poslen - 1];
    if (i + 1 < src_sz) output_text(global, src + i + 1, src_sz - i - 1);
}

static void
output_positions(GlobalData *global, len_t *positions, len_t num) {
    wchar_t buf[128];
    for (len_t i = 0; i < num; i++) {
        int pnum = swprintf(buf, arraysz(buf), L"%u", positions[i]);
        if (pnum > 0 && ensure_space(global, pnum + 1)) {
            for (int k = 0; k < pnum; k++) global->output[global->output_pos++] = buf[k];
            global->output[global->output_pos++] = (i == num - 1) ? ':' : ',';
        }
    }
}


static void
output_result(GlobalData *global, Candidate *c, Options *opts, len_t needle_len) {
    if (opts->output_positions) output_positions(global, c->positions, needle_len);
    if (opts->mark_before_sz > 0 || opts->mark_after_sz > 0) {
        output_with_marks(global, opts, c->src, c->src_sz, c->positions, needle_len);
    } else {
        output_text(global, c->src, c->src_sz);
    }
    output_text(global, opts->delimiter, opts->delimiter_sz);
}


void
output_results(GlobalData *global, Candidate *haystack, size_t count, Options *opts, len_t needle_len) {
    Candidate *c;
#define lt(b, a) ( (a)->score < (b)->score || ((a)->score == (b)->score && (a->idx < b->idx)) )
    QSORT(Candidate, haystack, count, lt);
#undef lt
    size_t left = opts->limit > 0 ? opts->limit : count;
    for (size_t i = 0; i < left; i++) {
        c = haystack + i;
        if (c->score > 0) output_result(global, c, opts, needle_len);
    }
}
