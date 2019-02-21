/*
genetic_operations.hpp

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
#ifndef GENETIC_OPERATIONS_HPP
#define GENETIC_OPERATIONS_HPP

#include "network.hpp"
#include <vector>

class PlayField;

neural_net crossover(const neural_net& parent_1, const neural_net& parent_2);
neural_net mutate(const neural_net& net, double mutation_factor = 0.1);

std::vector<const PlayField*> select(const std::vector<const PlayField*>& fields, size_t amount_to_select);

std::vector<neural_net> breed(const neural_net& parent_1, const neural_net& parent_2, size_t children_count);

#endif // GENETIC_OPERATIONS_HPP
