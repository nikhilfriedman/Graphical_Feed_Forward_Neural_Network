#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "definitions.h"
#include <stdbool.h>

double    gaussian_random     (double mean, double standard_deviation);
double    xavier              (int num_input_neurons, int num_output_neurons);
double    relu                (double x);
double    relu_derivative     (double x);
void      free_list           (Node * n);
void      free_weights        (Weight * w);
void      free_neurons        (Neuron * n);
void      free_layers         (Layer * l);
void      free_network        (Network * n);
Node    * create_node         (int val);
Weight  * create_weight       (int input_neurons, int output_neurons);
Neuron  * create_neuron       (void);
Layer   * create_layer        (int size);
Network * create_network      (int layer_sizes[], int layers);
void      reset_network       (Network * network);
void      get_input_activation(Network * network, double input[], int size);
void      softmax_output      (Layer * output_layer);
void      activate_network    (Network * network);
double    mse                 (Network * network, double truth[], int truth_size);
double    forward_pass        (Network * network, double truth[], int truth_size, double input[], int input_size);
void      backward_pass       (Network * network, double truth[], int truth_size, double learning_rate, double lambda, int batch_size);
void      apply_gradient      (Network * network, double learning_rate);
void      epoch               (Network * network, double ** data, int num_features, int num_samples);
double    test_network        (Network * network, double ** test, int num_features, int num_samples);
void      print_network       (Network * network);
void      save_network_to_file(Network * network);

#endif