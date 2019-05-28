/*
    Authors:
        Tomas Halman <thalman@redhat.com>

    Copyright (C) 2019 Red Hat

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __EVALUATOR_H
#define __EVALUATOR_H

#include <stdbool.h>

#include "common/errno_t.h"

/**
 * Evaluate expression.
 *
 * Returns EOK or an error on failure.
 *
 * Evaluation result is stored in _result variable.
 */
/**
 * Evaluate expression.
 *
 * @param expression Expression to evaluate.
 * @param features   NULL-terminated array that lists enabled features.
 * @param _result    Output parameter where the result of the evaluation
 *                   is stored.
 *
 * @return EOK on success, other errno code on failure.
 */
errno_t evaluate(const char *expression, const char **features, bool *_result);

#endif /* __EVALUATOR_H */
