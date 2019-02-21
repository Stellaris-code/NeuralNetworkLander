/*
genetic_operations.cpp

Copyright (c) 14 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "genetic_operations.hpp"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>

#include <functional>

#include "playfield.hpp"

#include "genann.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

neural_net crossover(const neural_net &parent_1, const neural_net &parent_2)
{
#if 0
    assert(parent_1.nn->total_weights == parent_2.nn->total_weights);

    int split_point_1 = rand() % parent_1.nn->total_weights;
    int split_point_2 = rand() % parent_1.nn->total_weights;

    // split point are ][
    int lower_split_point = min(split_point_1, split_point_2);
    int upper_split_point = max(split_point_1, split_point_2);

    neural_net new_net = nn_clone(parent_1);

    memcpy(new_net.nn->weight, parent_1.nn->weight, lower_split_point * sizeof(double));
    memcpy(new_net.nn->weight + lower_split_point, parent_2.nn->weight + lower_split_point, (upper_split_point - lower_split_point) * sizeof(double));
    memcpy(new_net.nn->weight + upper_split_point, parent_1.nn->weight + upper_split_point, (parent_1.nn->total_weights - upper_split_point) * sizeof(double));

    return new_net;
#else

    assert(parent_1.nn->total_weights == parent_2.nn->total_weights);

    neural_net new_net = nn_clone(parent_1);

    // randomly choose the genes of one of the parents
    for (size_t i { 0 }; i < new_net.nn->total_weights; ++i)
    {
        if (rand() % 2)
            new_net.nn->weight[i] = parent_1.nn->weight[i];
        else
            new_net.nn->weight[i] = parent_2.nn->weight[i];
    }

    return new_net;
#endif
}

neural_net mutate(const neural_net &net, double mutation_factor)
{
#if 1
    double mutation_probabiblity = 0.1;

    // no mutation
    if ((double)rand() / RAND_MAX < mutation_probabiblity)
        return net;

    int mutated_gene = rand() % net.nn->total_weights;

    neural_net mutated = net;
    mutated.nn->weight[mutated_gene] = (double)rand() / RAND_MAX - 0.5;

    return mutated;
#else
    double mutation_probabiblity = 1.0/net.nn->total_weights;

    neural_net mutated = net;

    for (size_t i { 0 }; i < net.nn->total_weights; ++i)
    {
        if ((double)rand() / RAND_MAX <= mutation_probabiblity)
            mutated.nn->weight[i] += 1.0 - 2*((double)rand() / RAND_MAX);
    }

    return mutated;
#endif
}

std::vector<neural_net> breed(const neural_net &parent_1, const neural_net &parent_2, size_t children_count)
{
    std::vector<neural_net> offspring;

    for (size_t i { 0 }; i < children_count; ++i)
        offspring.push_back(mutate(crossover(parent_1, parent_2)));

    return offspring;
}

std::vector<const PlayField *> select(const std::vector<const PlayField *> &fields, size_t amount_to_select)
{
    std::vector<int> selected_individuals;

    for (size_t counter { 0 }; counter < amount_to_select; ++counter)
    {
        double total_fitness = 0;
        for (size_t i { 0 }; i < fields.size(); ++i)
        {
            if (std::find(selected_individuals.begin(), selected_individuals.end(), i) != selected_individuals.end())
                continue;
            total_fitness += fields[i]->score();
        }

        std::vector<double> normalized_fitnesses;
        normalized_fitnesses.reserve(fields.size());
        for (size_t i { 0 }; i < fields.size(); ++i)
        {
            if (std::find(selected_individuals.begin(), selected_individuals.end(), i) != selected_individuals.end())
                continue;
            normalized_fitnesses.emplace_back(fields[i]->score()/total_fitness);
        }

        std::sort(normalized_fitnesses.begin(), normalized_fitnesses.end(), std::greater<>{});

        for (size_t i { 1 }; i < normalized_fitnesses.size(); ++i)
        {
            // accumulate fitnesses
            normalized_fitnesses[i] += normalized_fitnesses[i-1];
        }

        double random_selector = (double)rand() / RAND_MAX;

        for (size_t i { 0 }; i < normalized_fitnesses.size(); ++i)
        {
            if (normalized_fitnesses[i] >= random_selector)
            {
                selected_individuals.emplace_back(i);
                break;
            }
        }
    }

    std::vector<const PlayField*> selected_fields;
    selected_fields.reserve(selected_individuals.size());
    for (auto index : selected_individuals)
        selected_fields.emplace_back(fields[index]);

    assert(selected_fields.size() == amount_to_select);

    return selected_fields;
}
