# MLViewer

User interface for handwritten digits recognition and VGG16 neural networks using Qt Creator and TensorFlow.

## Getting Started

Follow these instructions to get a copy of the project and make it run on your local machine (only tested with Ubuntu 18.04). 

### Prerequisites

You need to have these tools installed on your Ubuntu machine:

* Python 3 (*pre-installed on Ubuntu 16.04 & 18.04*)
* [TensorFlow](http://www.tensorflow.org/install/install_linux) 
* [Qt Creator](http://doc.qt.io/qt-5/linux.html)

### Settings

Once you have cloned and opened this project with Qt Creator, you have to disable the *Shadow Build* in the Projects tab. Before running the app, you also have to export a handwritten digits recognition model using the [MNISTER repository](https://github.com/polivier2/MNISTER). A VGG16 trained neural network can be found using this [repository](https://github.com/ry/tensorflow-vgg16). Then, you have to make sure you have the following directory tree:

```
./WORKSPACE
	|-- icons
	|	|-- chip_256.png
	|	|-- image.png
	|	|-- number-blocks.png
	|-- MNIST
	|	|-- own_0.png
	|	|-- own_1.png
	|	|-- own_2.png
	|	|-- own_3.png
	|	|-- own_4.png
	|	|-- SAVED_MODEL
	|   |     	|-- 1
	|   |		|	|-- variables
	|   |		|		|-- variables.data
	|   |		|		|-- variables.index
	|   |		|	|-- saved_model.pb
	|   |		|-- graph_for_model_1_xxxxxxxx
	|   |		|	|-- events.out.tfevents.xxxxxxxxxx.user
	|-- scripts
	|	|-- base.py
	|	|-- init_test.py
	|	|-- mnist.py
	|	|-- mnist_load.py
	|	|-- mnist_preproc.py
	|	|-- synset.txt
	|	|-- utils.py
	|	|-- vgg16.py
	|-- VGG16
	|	|-- cat.jpg
	|	|-- weasel.png
	|	|-- vgg16.pb
	|-- logwindow.cpp/.h
	|-- main.cpp
	|-- mlviewer.cpp/.h
	|-- MLViewer
	|-- MLViewer.pro
	|-- README.md
	|-- stylesheet.qss	
```

## Running the app

### MNIST

### VGG16


## TODO

- Load every type of model: allow user to specify input and output layers name of trained model
- Zoom on small pictures
- Reset only the output for MNIST 

## Authors

* **Pierre OLIVIER** - [polivier2](https://github.com/polivier2)
