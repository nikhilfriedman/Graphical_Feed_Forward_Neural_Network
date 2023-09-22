#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

typedef struct Dimension { // 2-dimensional point
    int row;               // rows 
    int col;               // columns
} Dimension;

typedef struct Button {
    int x;
    int y;
    int w;
    int h;
} Button;

typedef struct CharNode {   // linked list node
    struct CharNode * next; // next node in list
    char * val;             // node value
} CharNode;

typedef struct Node {
    struct Node * next;
    double val;
} Node;

typedef struct Thread_Data {
    struct Network * network;
    double ** data;
    int num_features;
    int num_samples;
} Thread_Data;

typedef struct Weight {
    struct Weight * next;
    struct Neuron * to_neuron;
    double val;
    double gradient;
} Weight;

typedef struct Neuron {
    struct Neuron * next;
    struct Weight * weights;
    double activation;
    double bias;
    double delta;
} Neuron;

typedef struct Layer {
    struct Layer * next;
    struct Layer * prev;
    struct Neuron * neurons;
    int size;
} Layer;

typedef struct Network {
    struct Layer * layers;
    int size;
    int current_epoch;
} Network;

// I/O stream
#define DATA_DIR  "data/mnist/"     // directory of our data
#define DATA_FILE "mnist_train.csv" // file name of our data
#define TEST_FILE "mnist_test.csv"  // location of our test data set
#define FILE_BUFF 40                // string size buffer for filepath
#define LINE_BUFF 10000             // line size buffer for data

// statistics
#define MAIN_SEED     97                     // seed (my favorite number)
#define RAND_NORM     0.1                    // max distance from 0 for random numbers
#define PI            3.14159265358979323846 // pi

// network structure
#define DATA_TOO_BIG   1                            // data is too big for our input layer
#define DATA_TOO_SMALL 2                            // data is too small for our input layer
#define DATA_GOOD_SIZE 3                            // data is the right size
#define NETWORK_DIR    "data/network architecture/" // directory of network states
#define NETWORK_FILE   "last_state.csv"             // last state (weights/biases) of network
#define BATCH_SIZE     64                           // samples per batch
#define LEARNING_RATE  0.001                        // learning rate for weights/biases
#define REGULAR_TERM   0.001                        // regularization term (lambda)

// graphics
#define WINDOW_TITLE   "Feed-Forward Neural Network" // title of our window
#define WINDOW_ICON    "textures/icon.bmp"           // icon for our window
#define NEURON_ICON    "textures/circle.bmp"         // icon for neuron
#define WINDOW_HEIGHT  600                           // window height
#define WINDOW_WIDTH   1000                          // window width
#define DEFAULT_FONT   "textures/bebas_neue.ttf"     // font
#define DRAWING_SIZE   28                            // drawing surface
#define MAX_LAYER_SIZE 21                            // tallest a layer can be to draw it

#endif