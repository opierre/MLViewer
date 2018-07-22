import tensorflow as tf
import utils

import math

import numpy as np
import os
import sys

import shutil

import cv2

tf.app.flags.DEFINE_string('model_path', '', 'model path.')
tf.app.flags.DEFINE_integer('nb_images', 4, 'number of images to deal with.')
tf.app.flags.DEFINE_boolean('print_prob', True, 'print probabilities.')
tf.app.flags.DEFINE_string('images_path', '', 'images path.')

"""
                PATHS                              
"""
SAVED_IMAGES = './build-project/'
DR_SAVED_IMAGES = os.path.dirname(SAVED_IMAGES)

dash = '-' * 120

print(dash)
print('Program:\t\t\tvgg16.py\n'
  'Number of images:\t\t%d\n'
  'Print details:\t\t%r'
  % (tf.app.flags.FLAGS.nb_images, tf.app.flags.FLAGS.print_prob))
print(dash)
sys.stdout.flush()

"""
              	MODEL                             
"""
print ("\nOpening Model...")
sys.stdout.flush()
with open(tf.app.flags.FLAGS.model_path, mode='rb') as f:
  fileContent = f.read()
print ("Done!\n")
print((15), end ='', file=sys.stderr)
sys.stderr.flush()

graph_def = tf.GraphDef()
graph_def.ParseFromString(fileContent)

images = tf.placeholder("float", [None, 224, 224, 3])

"""
              	GRAPH                             
"""
print ("Loading GraphDef...")
sys.stdout.flush()
tf.import_graph_def(graph_def, input_map={ "images": images })
print ("Done!\n")

graph = tf.get_default_graph()
print((10), end ='', file=sys.stderr)
sys.stderr.flush()


"""
                 INPUTS
"""
if not os.path.exists(DR_SAVED_IMAGES):
  os.makedirs(DR_SAVED_IMAGES)

inputs = np.zeros((tf.app.flags.FLAGS.nb_images, 224, 224, 3))

i = 0
img_name = tf.app.flags.FLAGS.images_path.split(',')
for name in img_name:
  shutil.copyfile(name, SAVED_IMAGES+"own_"+str(i)+".png")
  inputs[i] = utils.load_image(SAVED_IMAGES+"own_"+str(i)+".png")

"""
                SESSION
"""
prob_save 	= []

print ("Running Output...")
sys.stdout.flush()

with tf.Session() as sess:
  init = tf.global_variables_initializer()
  sess.run(init)

  input_bake = inputs[:1]
  
  feed_dict = { images: input_bake }

  prob_tensor = graph.get_tensor_by_name("import/prob:0")

  prob = sess.run(prob_tensor, feed_dict=feed_dict)

  print((25), end ='', file=sys.stderr)
  sys.stderr.flush()
	
  for i in range (0, tf.app.flags.FLAGS.nb_images):
    input_loop = inputs[i:i+1]

    feed_dict = { images: input_loop }

    prob_tensor = graph.get_tensor_by_name("import/prob:0")

    prob = sess.run(prob_tensor, feed_dict=feed_dict)

    print((35/tf.app.flags.FLAGS.nb_images), end ='', file=sys.stderr)
    sys.stderr.flush()
    
    prob_save.append(prob[0])

  top1 = utils.print_prob(prob, tf.app.flags.FLAGS.print_prob)

  for i in range(0, len(prob_save)):
    sys.stdout.flush()
    img_fin = cv2.imread(SAVED_IMAGES+"own_"+str(i)+".png")
    cv2.putText(img_fin, str(top1[i]), (math.floor(img_fin.shape[0]*(0.05)), math.floor(img_fin.shape[1]*(0.7))),cv2.FONT_HERSHEY_DUPLEX, 2, (1, 224, 177), 3)
    sys.stdout.flush()
    cv2.imwrite(SAVED_IMAGES+"image_finish_"+str(i)+".png", img_fin)

    print((10/tf.app.flags.FLAGS.nb_images), end ='', file=sys.stderr)
    sys.stderr.flush()

    print ("Done!")
    sys.stdout.flush()
