#pragma once
#include "io.h"

int pickProblem (Data *data, int *picked) {
    int cur = (*picked) + 1;
    for (; cur<data->n_queries; cur++) {
        if (data->queries[cur].type == group_analyse) {
            *picked = cur;
            return cur;
        }
    }
    return -1;
}

