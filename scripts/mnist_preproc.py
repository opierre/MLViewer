import cv2
import numpy as np
import math
from scipy import ndimage

import os
import sys
import argparse

from math import *

import shutil


SAVED_IMAGES = './build-project/'
DR_SAVED_IMAGES = os.path.dirname(SAVED_IMAGES)

#######################################################################
#                               Main                                  #
#######################################################################
def main(result):

  dash = '-' * 120

  print(dash)
  print('Program:\t\t\tmnist_preproc.py\n'
        'Number of images:\t\t%d\n'
        'Threshold value:\t\t%d'
        % (result.nb_images, result.value_th))
  print(dash)

  print('\nRunning threshold...', end='')
  sys.stdout.flush()

  if not os.path.exists(DR_SAVED_IMAGES):
    os.makedirs(DR_SAVED_IMAGES)

  
  """
                      Pre-processing on picture(s)                              
  """

  i = 0

  img_name = result.images_path.split(',')

  for name in img_name:

    shutil.copyfile(name, SAVED_IMAGES+"own_"+str(i)+".png")
    print((15/result.nb_images), end ='', file=sys.stderr)
    sys.stderr.flush()

    gray = cv2.imread(SAVED_IMAGES+"own_"+str(i)+".png", 0)
    print((40/result.nb_images), end ='', file=sys.stderr)
    sys.stderr.flush()

    im_gray = cv2.GaussianBlur(gray, (5, 5), 0)
    print((20/result.nb_images), end ='', file=sys.stderr)
    sys.stderr.flush()

    ret, im_th = cv2.threshold(im_gray, result.value_th, 255, cv2.THRESH_BINARY_INV)
    print((2/result.nb_images), end ='', file=sys.stderr)
    sys.stderr.flush()

    cv2.imwrite(SAVED_IMAGES+"image_"+str(i)+".png", im_th)
    print((22/result.nb_images), end ='', file=sys.stderr)
    sys.stderr.flush()

    i += 1

  print('\nDone!')
#  sys.stdout.flush()

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('--nb_images', type=int, default=4, dest='nb_images', help='number of images to deal with.')
  parser.add_argument('--images_path', default='', dest='images_path', help='images path.')
  parser.add_argument('--value_th', type=int, default=127, dest='value_th', help='threshold value.')
  result = parser.parse_args()
  main(result)
