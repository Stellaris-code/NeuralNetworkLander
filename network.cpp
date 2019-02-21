#include "genann.h"

#include "network.hpp"

void nn_init(neural_net& net, int inputs, int hidden_layers, int hidden_neurons, int outputs)
{
    if (net.nn)
        nn_free(net);

    net.nn = genann_init(inputs, hidden_layers, hidden_neurons, outputs);
    net.inputs = new double[inputs];
    net.outputs = net.nn->output;
}

void nn_free(neural_net &net)
{
    genann_free(net.nn);
    delete[] net.inputs;
}

neural_net nn_clone(const neural_net &net)
{
    neural_net clone;
    nn_init(clone, net.nn->inputs, net.nn->hidden_layers, net.nn->hidden, net.nn->outputs);

    return clone;
}

void nn_run(neural_net &net)
{
    net.outputs = genann_run(net.nn, net.inputs);
}
