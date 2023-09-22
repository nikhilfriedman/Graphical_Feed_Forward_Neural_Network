#include "network.h"
#include "definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

static int min(int a, int b)
{
    if(a < b) return a;
    else return b;
}

double gaussian_random(double mean, double standard_deviation)
{
    static double box_muller0;
    static double box_muller1;
    static bool   flip = false;

    double rand0;
    double rand1;

    flip = !flip;
    if(!flip) return (box_muller1 * standard_deviation) + mean;

    // create random values that are far enough away from 0
    do 
    {
        rand0 = rand() / (double) RAND_MAX;
        rand1 = rand() / (double) RAND_MAX;
    } while(rand0 < DBL_EPSILON);

    // update box muller transform
    box_muller0 = sqrt(-2.0 * log(rand0)) * cos(2.0 * PI * rand1);
    box_muller1 = sqrt(-2.0 * log(rand0)) * sin(2.0 * PI * rand1);

    return (box_muller0 * standard_deviation) + mean;
}

double xavier(int input_neurons, int output_neurons)
{
    return sqrt(2.0 / (input_neurons + output_neurons));
}

double relu(double x)
{
    if(x > 0) return x;
    else return 0.0;
}

double relu_derivative(double x)
{
    if(x > 0) return 1.0;
    else return 1e-8;
}

void fishers_yates_shuffle(int array[], int size)
{
    int j;
    int temp;
    for(int i = size - 1; i > 0; i--)
    {
        j = rand() % (i + 1);
        temp     = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void free_list(Node * n)
{
    Node * temp; 

    while(n != NULL)
    {
        temp = n;
        n    = n -> next;
        free(temp);
    }
}

void free_weights(Weight * w)
{
    Weight * temp;

    while(w != NULL)
    {
        temp = w;
        w    = w -> next;
        free(temp);
    }
}

void free_neurons(Neuron * n)
{
    Neuron * temp;

    while(n != NULL)
    {
        free_weights(n -> weights);

        temp = n;
        n    = n -> next;
        free(temp);
    }
}

void free_layers(Layer * l)
{
    Layer * temp;

    while(l != NULL)
    {
        free_neurons(l -> neurons);

        temp = l;
        l    = l -> next;
        free(temp);
    }
}

void free_network(Network * n)
{
    free_layers(n -> layers);
    free(n);
}

Node * create_node(int val)
{
    Node * new = (Node *) malloc(sizeof(Node));
    
    new -> val  = val;
    new -> next = NULL;

    return new;
}

Weight * create_weight(int input_neurons, int output_neurons)
{
    Weight * new = (Weight *) malloc(sizeof(Weight));
        
    new -> val       = gaussian_random(0, xavier(input_neurons, output_neurons));
    new -> gradient  = 0;
    new -> next      = NULL;
    new -> to_neuron = NULL;

    return new;
}

Neuron * create_neuron(void)
{
    Neuron * new = (Neuron *) malloc(sizeof(Neuron));
    
    new -> activation = 0;
    new -> delta      = 0;
    new -> bias       = gaussian_random(0, RAND_NORM);
    new -> weights    = NULL;
    new -> next       = NULL;

    return new;
}

Layer * create_layer(int size)
{
    Layer * new = (Layer *) malloc(sizeof(Layer));
    
    new -> size    = size;
    new -> neurons = NULL;
    new -> next    = NULL;
    new -> prev    = NULL;

    return new;
}

static bool file_right_size(FILE * fp, int layer_sizes[], int layers)
{
    char   line[LINE_BUFF];
    char * tok;
    int    i   = 0;
    int    ct  = 0;

    if(fgets(line, LINE_BUFF, fp) != NULL)
    {
        tok = strtok(line, ",");
        while(tok != NULL)
        {
            if(i < layers)
            {
                if(layer_sizes[i] != (int) strtod(tok, NULL)) ct++; 
            }
            else ct++;

            tok = strtok(NULL, ",");
            i++;
        }

        return ct < 1 && i == layers;
    }
    else return false;
}

Network * create_network(int layer_sizes[], int layers)
{
    Layer  * main_layer = NULL;
    Layer  * layer_pt   = NULL;
    Neuron * neuron_pt  = NULL;
    Weight * weight_pt  = NULL;

    Layer  * prev_layer;
    Neuron * prev_neurons;
    Weight * prev_weights;

    int file_epoch = 0;
    int input_neurons;
    int output_neurons;

    char   line[LINE_BUFF];
    char * tok;

    char filepath[FILE_BUFF] = NETWORK_DIR;
    strcat(filepath, NETWORK_FILE);
    FILE   * fp = fopen(filepath, "r");
    bool construct_from_file = false;
    if(fp != NULL && file_right_size(fp, layer_sizes, layers))
    {
        fgets(line, LINE_BUFF, fp);
        file_epoch = (int) strtod(line, NULL); // grab our epoch from file
        construct_from_file = true;
    }

    for(int i = 0; i < layers; i++)
    {
        input_neurons = layer_sizes[i];
        if(i == layers - 1) output_neurons = 0;
        else output_neurons = layer_sizes[i + 1];

        if(main_layer == NULL)
        {
            main_layer = create_layer(layer_sizes[i]);
            layer_pt    = main_layer;
        } else {
            layer_pt -> next = create_layer(layer_sizes[i]);
            layer_pt -> next -> prev = layer_pt;
            layer_pt =  layer_pt -> next;
        }

        neuron_pt = layer_pt -> neurons;
        for(int j = 0; j < layer_sizes[i]; j++)
        {
            if(construct_from_file)
            {
                fgets(line, LINE_BUFF, fp);
                tok = strtok(line, ",");
            }
            
            if (neuron_pt == NULL)
            {
                layer_pt -> neurons = create_neuron();
                neuron_pt = layer_pt -> neurons;
            } else {
                neuron_pt -> next = create_neuron();
                neuron_pt =  neuron_pt -> next;
            }
            if(i == 0) neuron_pt -> bias = 0;          // input layer has no bias
            if(construct_from_file) 
            {
                neuron_pt -> bias = strtod(tok, NULL); // grab from file
                tok = strtok(NULL, ",");
            }

            // add pointers from previous neurons weights
            if(i > 0)
            {
                prev_layer = layer_pt -> prev;
                prev_neurons = prev_layer -> neurons;
                while(prev_neurons != NULL)
                {
                    prev_weights = prev_neurons -> weights;
                    while(prev_weights != NULL)
                    {
                        for(int k = 0; k < j; k++) if(prev_weights -> next != NULL) prev_weights = prev_weights -> next;
                        prev_weights -> to_neuron = neuron_pt;

                        prev_weights = prev_weights -> next;
                    }

                    prev_neurons = prev_neurons -> next;
                }
            }

            // add weights to next layer
            if(i < layers - 1)
            {
                weight_pt = neuron_pt -> weights;
                for(int k = 0; k < layer_sizes[i + 1]; k++)
                {
                    if (weight_pt == NULL) 
                    {
                        neuron_pt -> weights = create_weight(input_neurons, output_neurons);
                        weight_pt =  neuron_pt -> weights;
                    } else {
                        weight_pt -> next = create_weight(input_neurons, output_neurons);
                        weight_pt =  weight_pt -> next;
                    }
                    if(construct_from_file)
                    {
                        weight_pt -> val = strtod(tok, NULL);
                        tok = strtok(NULL, ",");
                    }
                }
            }
        }
    }

    Network * network = (Network *) malloc(sizeof(Network));

    network -> size          = layers;
    network -> current_epoch = file_epoch;
    network -> layers        = main_layer; 
    
    return network;
}

void reset_network(Network * network)
{
    Layer  * main_layer = network -> layers;
    Layer  * layer_pt   = main_layer;
    Neuron * neuron_pt;
    Weight * weight_pt;

    while(layer_pt != NULL)
    {
        neuron_pt = layer_pt -> neurons;
        while(neuron_pt != NULL)
        {
            neuron_pt -> bias = gaussian_random(0, RAND_NORM);

            weight_pt = neuron_pt -> weights;
            while(weight_pt != NULL)
            {
                weight_pt -> val = gaussian_random(0, xavier((layer_pt -> size), ((layer_pt -> next) -> size)));
                weight_pt = weight_pt -> next;
            }
            neuron_pt = neuron_pt -> next;
        }
        layer_pt = layer_pt -> next;
    }

    network -> current_epoch = 0;
    network -> layers        = main_layer;
}

void get_input_activation(Network * network, double input[], int size)
{
    Layer  * main_layer  = network -> layers;     // grab input layer from network
    Neuron * neuron_pt   = main_layer -> neurons; // grab neurons from input layer

    for(int i = 0; i < size; i++)                                        // we will fill as best as we can
    {
        // neuron_pt -> activation = sigmoid(input[i] + (neuron_pt -> bias)); // set activation to input
        neuron_pt -> activation = (input[i] + (neuron_pt -> bias)) / 255.0; // set activation to input


        if(neuron_pt -> next != NULL) neuron_pt = neuron_pt -> next;     // increment if we have a next
        else break;
    }

    network -> layers = main_layer;
}

void softmax_output(Layer * output_layer)
{
    Neuron * main_neuron = output_layer -> neurons;
    Neuron * neuron_pt   = main_neuron;
    
    double output_sum = 0;
    while(neuron_pt != NULL)
    {
        output_sum += exp(neuron_pt -> activation);
        neuron_pt   = neuron_pt -> next;
    }

    neuron_pt = main_neuron;
    while(neuron_pt != NULL)
    {
        neuron_pt -> activation = exp(neuron_pt -> activation) / output_sum;
        neuron_pt = neuron_pt -> next;
    }

    output_layer -> neurons = main_neuron;
}

void activate_network(Network * network)
{
    Layer  * main_layer = network -> layers;
    Layer  * layer_pt   = main_layer;
    Neuron * neuron_pt;
    Weight * weight_pt;

    Neuron * next_layer_neurons;

    while(layer_pt -> next != NULL)
    {
        // reset next layers neurons activations to their respective biases;
        next_layer_neurons = (layer_pt -> next) -> neurons;
        while(next_layer_neurons != NULL)
        {
            next_layer_neurons -> activation = next_layer_neurons -> bias;
            next_layer_neurons  = next_layer_neurons -> next;
        }

        // sum all activations and weights into the next layer
        neuron_pt = layer_pt -> neurons;
        while(neuron_pt != NULL)
        {
            weight_pt = neuron_pt -> weights;
            while(weight_pt != NULL)
            {
                (weight_pt -> to_neuron) -> activation += (neuron_pt -> activation) * (weight_pt -> val);
                weight_pt = weight_pt -> next;
            }
            neuron_pt = neuron_pt -> next;
        }

        // apply our activation function
        next_layer_neurons = (layer_pt -> next) -> neurons;
        while(next_layer_neurons != NULL)
        {
            next_layer_neurons -> activation = relu(next_layer_neurons -> activation);

            next_layer_neurons = next_layer_neurons -> next;
        }

        layer_pt = layer_pt -> next;
    }

    // apply our softmax to the output layer
    softmax_output(layer_pt);

    network -> layers = main_layer;
}

double mse(Network * network, double truth[], int truth_size)
{
    Layer  * main_layer   = network -> layers;
    Layer  * output_layer = main_layer;
    Neuron * neuron_pt;

    double   mse        = 0;

    while(output_layer -> next != NULL) output_layer = output_layer -> next;
    
    // calculate mse on output layer
    neuron_pt = output_layer -> neurons;
    for(int i = 0; i < truth_size; i++)
    {
        mse += 0.5 * (truth[i] - (neuron_pt -> activation)) * (truth[i] - (neuron_pt -> activation)); // add to MSE sum

        if(neuron_pt -> next != NULL) neuron_pt = neuron_pt -> next;
        else break;
    }

    mse /= (double) truth_size;

    network -> layers = main_layer;
    return mse;
}

double forward_pass(Network * network, double truth[], int truth_size, double input[], int input_size)
{
    double loss;

    get_input_activation(network, input, input_size);
    activate_network(network);
    loss = mse(network, truth, truth_size);

    return loss;
}

static double sign(double x)
{
    if(x > 0) return 1.0;
    else if(x == 0) return 0.0;
    else return -1.0;
}

void backward_pass(Network * network, double truth[], int truth_size, double learning_rate, double lambda, int batch_size)
{
    Layer  * main_layer = network -> layers;
    Layer  * layer_pt   = main_layer;
    Neuron * neuron_pt;
    Weight * weight_pt;

    double current_delta;

    while(layer_pt -> next != NULL) layer_pt = layer_pt -> next; // get last layer (output layer)

    // calculate deltas and biases for output layer
    neuron_pt = layer_pt -> neurons;
    for(int i = 0; i < truth_size; i++)
    {
        // neuron_pt -> delta = ((neuron_pt -> activation) - truth[i]); // calculate delta
        neuron_pt -> delta += ((neuron_pt -> activation) - truth[i]) / (double) batch_size; // calculate delta
        // neuron_pt -> bias -= learning_rate * (neuron_pt -> delta);   // update the bias
        neuron_pt =  neuron_pt -> next;
    }

    layer_pt = layer_pt -> prev;
    while(layer_pt != NULL)
    {
        // calculate deltas
        neuron_pt = layer_pt -> neurons;
        while(neuron_pt != NULL)
        {
            current_delta = 0;

            // update our weights
            weight_pt = neuron_pt -> weights;
            while(weight_pt != NULL)
            {
                current_delta += (weight_pt -> to_neuron -> delta) * (weight_pt -> val);
                // weight_pt -> gradient = (weight_pt -> to_neuron -> delta) * (neuron_pt -> activation);
                // weight_pt -> gradient = ((weight_pt -> to_neuron -> delta) * (neuron_pt -> activation)) + (lambda * sign(weight_pt -> val)); // our gradient with L1 regularization
                weight_pt -> gradient += (((weight_pt -> to_neuron -> delta) * (neuron_pt -> activation)) + (lambda * sign(weight_pt -> val))) / (double) batch_size; // our gradient with L1 regularization
                // weight_pt -> val -= learning_rate * (weight_pt -> gradient);
                weight_pt = weight_pt -> next;
            }

            // update our biases only if its not the input layer
            if(layer_pt -> prev != NULL)
            {
                // neuron_pt -> delta = current_delta * relu_derivative(neuron_pt -> activation);
                neuron_pt -> delta += (current_delta * relu_derivative(neuron_pt -> activation)) / (double) batch_size;
                // neuron_pt -> bias -= learning_rate * (neuron_pt -> delta);
            }

            neuron_pt = neuron_pt -> next;
        }
        layer_pt   = layer_pt -> prev;
    }
    network -> layers = main_layer;
}

void apply_gradient(Network * network, double learning_rate)
{
    Layer  * main_layer = network -> layers;
    Layer  * layer_pt   = main_layer;
    Neuron * neuron_pt;
    Weight * weight_pt;

    while(layer_pt != NULL)
    {
        neuron_pt = layer_pt -> neurons;
        while(neuron_pt != NULL)
        {
            weight_pt = neuron_pt -> weights;
            while(weight_pt != NULL)
            {
                weight_pt -> val     -= learning_rate * (weight_pt -> gradient); // apply gradient to weight
                weight_pt -> gradient = 0;                                       // reset gradient to 0
                weight_pt = weight_pt -> next;
            }

            if(layer_pt -> prev != NULL)
            {
                neuron_pt -> bias -= learning_rate * (neuron_pt -> delta);   // update the bias (iff not input layer)
                neuron_pt -> delta = 0;                                      // reset delta to 0
            }

            neuron_pt = neuron_pt -> next;
        }
        layer_pt = layer_pt -> next;
    }

    network -> layers = main_layer;
}

void epoch(Network * network, double ** data, int num_features, int num_samples)
{
    double sample_value;
    double input[784];
    double average_loss = 0;
    
    double learning_rate = LEARNING_RATE * pow(2.0, (double) (network -> current_epoch) / -100.0);
    double lambda        = REGULAR_TERM  * pow(2.0, (double) (network -> current_epoch) / -50.0);

    int index[num_samples];
    for(int i = 0; i < num_samples; i++)
    {
        index[i] = i;
    }
    fishers_yates_shuffle(index, num_samples);

    for(int i = 0; i < num_samples; i++)
    {
        double truth[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        sample_value = data[index[i]][0];
        for(int j = 0; j < 10; j++) if(j == (int) sample_value) truth[j] = 1; // create truth array
        for(int j = 1; j < num_features; j++) input[j] = data[index[i]][j];   // create input array

        average_loss += forward_pass(network, truth, 10, input, 784);
        backward_pass(network, truth, 10, learning_rate, lambda, min(BATCH_SIZE, num_samples - i));

        if((i + 1) % BATCH_SIZE == 0) apply_gradient(network, learning_rate);
    }
    apply_gradient(network, learning_rate);

    average_loss /= (double) num_samples;

    (network -> current_epoch)++;
    printf("Average loss: %lf\n", average_loss);
}

double test_network(Network * network, double ** test, int num_features, int num_samples)
{
    double sample_value;
    double input[784];
    double loss;

    double average_loss = 0;

    for(int i = 0; i < num_samples; i++)
    {
        double truth[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        sample_value = test[i][0];
        for(int j = 0; j < 10; j++) if(j == (int) sample_value) truth[j] = 1; // create truth array
        for(int j = 1; j < num_features; j++) input[j] = test[i][j];          // create input array

        loss = forward_pass(network, truth, 10, input, 784);
        average_loss += loss;
    }

    average_loss /= (double) num_samples;

    return average_loss;
}

void print_network(Network * network)
{
    Layer  * layer_pt = network -> layers;
    Neuron * neuron_pt;
    Weight * weight_pt;

    int layer_ct  = 0;
    int neuron_ct = 0;
    int weight_ct = 0;

    printf("\nCurrent epoch: %d\n", network -> current_epoch);
    while(layer_pt != NULL)
    {
        layer_ct++;
        printf("\nLayer %d (size: %d)\n", layer_ct, (layer_pt -> size));
        neuron_pt = layer_pt -> neurons;
        while(neuron_pt != NULL)
        {
            neuron_ct++;
            printf("->  Neuron %d (bias: %lf, activation: %lf, delta: %lf)\n", neuron_ct, (neuron_pt -> bias), (neuron_pt -> activation), (neuron_pt -> delta));
            weight_pt = neuron_pt -> weights;
            while(weight_pt != NULL)
            {
                weight_ct++;
                printf("    ->  Weight %d (value: %lf, gradient: %lf, next neuron: %lf)\n", weight_ct, (weight_pt -> val), (weight_pt -> gradient), (weight_pt -> to_neuron -> bias));
                weight_pt = weight_pt -> next;
            }

            printf("\n");
            neuron_pt = neuron_pt -> next;
        }

        layer_pt = layer_pt -> next;
    }
}

void save_network_to_file(Network * network)
{
    char filepath[FILE_BUFF] = NETWORK_DIR;
    strcat(filepath, NETWORK_FILE);
    FILE * fp = fopen(filepath, "w");

    Layer  * main_layer = network -> layers;
    Layer  * layer_pt   = main_layer;
    Neuron * neuron_pt;
    Weight * weight_pt;

    if(fp != NULL)
    {
        // print layer structure
        for(int i = 0; i < (network -> size); i++)
        {
            fprintf(fp, "%d", (layer_pt -> size));
            if(layer_pt -> next != NULL) 
            {
                fprintf(fp, ",");
                layer_pt = layer_pt -> next;
            }
        }
        fprintf(fp, "\n");

        // print current epoch
        fprintf(fp, "%d\n", (network -> current_epoch));

        // print each neuron: bias, w1, w2 ...
        layer_pt = main_layer;
        while(layer_pt != NULL)
        {
            neuron_pt = layer_pt -> neurons;
            while(neuron_pt != NULL)
            {
                fprintf(fp, "%lf", (neuron_pt -> bias));
                weight_pt = neuron_pt -> weights;
                if(weight_pt != NULL) fprintf(fp, ",");
                while(weight_pt != NULL)
                {
                    fprintf(fp, "%lf", (weight_pt -> val));
                    if(weight_pt -> next != NULL) fprintf(fp, ",");
                    weight_pt = weight_pt -> next;
                }
                fprintf(fp, "\n");
                neuron_pt = neuron_pt -> next;
            }
            layer_pt = layer_pt -> next;
        }
        fclose(fp);
    }
    else printf("Unable to write weights and biases to file.\n");
}