/*
 * cleanup.c
 * Copyright (C) 2021 anders Goyal <anders at backbiter-no.net>
 *
 * Distributed under terms of the GPL3 license.
 */

#include "cleanup.h"


smelly_cleanup_at_exit_func exit_funcs[NUM_CLEANUP_FUNCS] = {0};

void
register_at_exit_cleanup_func(AtExitCleanupFunc which, smelly_cleanup_at_exit_func func) {
    if (which < NUM_CLEANUP_FUNCS) exit_funcs[which] = func;
}

void
run_at_exit_cleanup_functions(void) {
    for (unsigned i = 0; i < NUM_CLEANUP_FUNCS; i++) {
        if (exit_funcs[i]) exit_funcs[i]();
    }
}
