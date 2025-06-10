# Feed-Forward Neural Network in C

A graphical, modular, feed-forward neural network created around the MNIST handwritten digit dataset.

This project implements a fully configurable neural network from scratch in C, supporting any number of layers and neurons per layer. It is trained using backpropagation and gradient descent, with no external machine learning libraries.

An SDL2-based interface is included to:
- Draw digits in a 28×28 grid
- View live predictions from the trained model
- Visualize the network’s learned weight matrices

## Running the Program

### Windows
1. Clone the repository:
   ```bash
   git clone https://github.com/nikhilfriedman/Graphical_Feed_Forward_Neural_Network.git
   cd Graphical_Feed_Forward_Neural_Network
2. Run the program:
   ```bash
   network.exe

### Linux/macOS (or if you want to recompile)
1. Make sure SDL2 and make is installed
2. Clone the repository and build with make:
   ```bash
   git clone https://github.com/nikhilfriedman/Graphical_Feed_Forward_Neural_Network.git
   cd Graphical_Feed_Forward_Neural_Network
   make
3. Run the compiled program:
   ```bash
   ./network

## Requirements
- SDL2 (for graphical interface)
- C compiler (e.g., gcc or clang)
- MNIST dataset (can be found zipped inside the data/mnist directory)

## License
This project is open-source and available under the MIT License.
