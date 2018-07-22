import cv2
import numpy as np
import math
from scipy import ndimage

import os
import sys
import argparse

import numpy

from init_test import *
from mnist import *

import tensorflow as tf

from math import *

tf.app.flags.DEFINE_string('model_path', '', 'model path.')
tf.app.flags.DEFINE_integer('nb_images', 4, 'number of images to deal with.')
tf.app.flags.DEFINE_boolean('digits_per_img', True, 'many digits per image (True) or just one digit (False).')
tf.app.flags.DEFINE_boolean('print_prob', True, 'print probabilities.')

SAVED_IMAGES = './build-project/'

#######################################################################
#                       Operation on pictures                         #
#######################################################################  
def getBestShift(img):
    cy,cx = ndimage.measurements.center_of_mass(img)

    rows,cols = img.shape
    shiftx = np.round(cols/2.0-cx).astype(int)
    shifty = np.round(rows/2.0-cy).astype(int)

    return shiftx,shifty

def shift(img,sx,sy):
    rows,cols = img.shape
    M = np.float32([[1,0,sx],[0,1,sy]])
    shifted = cv2.warpAffine(img,M,(cols,rows))
    return shifted


#######################################################################
#                          Tools Functions                            #
####################################################################### 
def maxs(x):
    first_max = np.amax(x) 
    y = np.argmax(x)
    x = np.delete(x, y)
    new_max = np.amax(x) 
    new_index = np.argmax(x)
    if (new_index >= y):
      new_index += 1
    return first_max, new_max, new_index

#######################################################################
#                               Main                                  #
#######################################################################
def main(_):

  dash = '-' * 120

  export_path = os.path.join(
        tf.compat.as_bytes(''),
        tf.compat.as_bytes(str(tf.app.flags.FLAGS.model_path)))

  print(dash)
  print('Program:\t\t\tmnist_load.py\n'
        'Model path:\t\t%s\n'
        'More than one digit per image:\t%r\n'
        'Number of images:\t\t%d\n'
        'Print results:\t\t%r'
        % (tf.app.flags.FLAGS.model_path, tf.app.flags.FLAGS.digits_per_img,
        tf.app.flags.FLAGS.nb_images, tf.app.flags.FLAGS.print_prob))
  print(dash)

  print('\nLoading data...')
  sys.stdout.flush()

  """
                      Pre-processing on picture(s)                              
  """
  if (tf.app.flags.FLAGS.digits_per_img == True):
  
    img = cv2.imread(SAVED_IMAGES+"own_0.png")
    print((18), end ='', file=sys.stderr)
    sys.stderr.flush()

  
    im_th = cv2.imread(SAVED_IMAGES+"image_0.png", 0)
    print((5), end ='', file=sys.stderr)
    sys.stderr.flush()
    
  
    _, ctrs, hier = cv2.findContours(im_th, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    print((1), end ='', file=sys.stderr)
    sys.stderr.flush()    
  
    images = np.zeros((len(ctrs),784))
  
    rects = [cv2.boundingRect(ctr) for ctr in ctrs]    
  
    i = 0
    for rect in rects:

      cv2.rectangle(img, (rect[0], rect[1]), (rect[0] + rect[2], rect[1] + rect[3]), (67, 54, 8), 3)

      leng = int(rect[3] * 1.6)
      pt1 = int(rect[1] + rect[3] // 2 - leng // 2)
      pt2 = int(rect[0] + rect[2] // 2 - leng // 2)
      roi = im_th[pt1:pt1+leng, pt2:pt2+leng]

      cv2.imwrite(SAVED_IMAGES+"image_out_"+str(i+1)+".png", roi)
      roi = cv2.resize(roi, (28, 28), interpolation=cv2.INTER_AREA)
      cv2.imwrite(SAVED_IMAGES+"image_out_"+str(i+2)+".png", roi)
      roi = cv2.dilate(roi, (3, 3))

      cv2.imwrite(SAVED_IMAGES+"image_out_"+str(i)+".png", roi)

      flatten = roi.flatten() / 255.0

      images[i] = flatten
      i+=1
    print((18), end ='', file=sys.stderr)
    sys.stderr.flush()

  else:

    images = np.zeros((tf.app.flags.FLAGS.nb_images,784))

    i = 0
    for no in range(0, tf.app.flags.FLAGS.nb_images):
      gray = cv2.imread(SAVED_IMAGES+"image_"+str(no)+".png", 0)

      gray = cv2.resize(gray, (28, 28))

      while np.sum(gray[0]) == 0:
        gray = gray[1:]

      while np.sum(gray[:,0]) == 0:
        gray = np.delete(gray,0,1)

      while np.sum(gray[-1]) == 0:
        gray = gray[:-1]

      while np.sum(gray[:,-1]) == 0:
        gray = np.delete(gray,-1,1)

      rows,cols = gray.shape

      if rows > cols:
        factor = 20.0/rows
        rows = 20
        cols = int(round(cols*factor))
        gray = cv2.resize(gray, (cols,rows))
      else:
        factor = 20.0/cols
        cols = 20
        rows = int(round(rows*factor))
        gray = cv2.resize(gray, (cols,rows))

      colsPadding = (int(math.ceil((28-cols)/2.0)),int(math.floor((28-cols)/2.0)))
      rowsPadding = (int(math.ceil((28-rows)/2.0)),int(math.floor((28-rows)/2.0)))
      gray = np.lib.pad(gray,(rowsPadding,colsPadding),'constant')

      shiftx,shifty = getBestShift(gray)
      shifted = shift(gray,shiftx,shifty)
      gray = shifted

      cv2.imwrite(SAVED_IMAGES+"image_out_"+str(no)+".png", gray)

      flatten = gray.flatten() / 255.0

      images[i] = flatten
      i+=1
    print((45), end ='', file=sys.stderr)
    sys.stderr.flush()

  print('Done!\n')

  """
                           Running Session                              
  """
  print('Starting session...')
  sys.stdout.flush()
  with tf.Session() as sess:
    print('Done!\n')
    print('Loading trained model...')
    sys.stdout.flush()

    saved_model = tf.saved_model.loader.load(sess, [tf.saved_model.tag_constants.TRAINING], export_path)
    print('Done!\n')
    print((14), end ='', file=sys.stderr)
    sys.stderr.flush()
    
    graph = tf.get_default_graph()
    x = graph.get_tensor_by_name("Placeholder_x:0")
    prob = graph.get_tensor_by_name("dropout_/keep_prob:0")
    classes = graph.get_operation_by_name('get_pred_/classe').outputs[0]
    perc = graph.get_operation_by_name('get_pred_/perc').outputs[0]
    sess.run(tf.tables_initializer())
    print((1), end ='', file=sys.stderr)
    sys.stderr.flush()

    print('Running session... \n')

    if (tf.app.flags.FLAGS.print_prob == True):
      print(dash)
      print('{:^20s}{:^22s}{:^22s}{:^22s}'.format("Prediction","Probability (%)","2nd Prediction","Probability (%)"))
      print(dash)

    images_save = images

    y_save    = np.zeros(len(images_save))
    first_max = np.zeros(len(images_save))
    new_index = np.zeros(len(images_save))
    new_max   = np.zeros(len(images_save))

    """
                        Processing
    """  
    for k in range(0, len(images_save)):

      images = np.reshape(images_save[k], [-1, 784])

      y = sess.run((classes, perc),feed_dict={x: images, prob:1.0})

      y_save[k] = y[0]

      first_max[k], new_max[k], new_index[k] = maxs(y[1])
      print((1/len(images_save)), end ='', file=sys.stderr)
      sys.stderr.flush()

    if (tf.app.flags.FLAGS.print_prob == True):
      for k in range(0, len(images_save)):
        print('{:^27.0f}{:^29.2f}{:^36.0f}{:^31.2f}'.format(y_save[k], first_max[k]*100.0, new_index[k], new_max[k]*100.0))
        print(dash)

    print('\nDone!')
    
    if (tf.app.flags.FLAGS.digits_per_img == True):
      j = 0
      for rect in rects: 
        cv2.putText(img, str(int(y_save[j])), (rect[0], rect[1]-10),cv2.FONT_HERSHEY_DUPLEX, 2, (67, 54, 8), 3)
        j += 1
      cv2.imwrite(SAVED_IMAGES+"image_finish_0.png", img)
    else:
        for k in range(0, len(images_save)):
          img_fin = cv2.imread(SAVED_IMAGES+"own_"+str(k)+".png")
          cv2.putText(img_fin, str(int(y_save[k])), (math.floor(img_fin.shape[0]*(0.05)), math.floor(img_fin.shape[1]*(0.8))),cv2.FONT_HERSHEY_DUPLEX, 2, (67, 54, 8), 3)
          cv2.imwrite(SAVED_IMAGES+"image_finish_"+str(k)+".png", img_fin)
    

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  FLAGS, unparsed = parser.parse_known_args()
  tf.app.run(main=main, argv=[sys.argv[0]] + unparsed)
