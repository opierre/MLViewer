import numpy as np

import cv2

synset = [l.strip() for l in open('./scripts/synset.txt').readlines()]

dash = '-' * 120

def load_image(path):
  img = cv2.imread(path)
  img = img / 255.0
  assert (0 <= img).all() and (img <= 1.0).all()

  short_edge = min(img.shape[:2])
  yy = int((img.shape[0] - short_edge) / 2)
  xx = int((img.shape[1] - short_edge) / 2)
  crop_img = img[yy : yy + short_edge, xx : xx + short_edge]

  resized_img = cv2.resize(crop_img, (224, 224))
  return resized_img


def print_prob(prob, print_details):
  top1 = []

  for i in range (0, len(prob)):
    pred = np.argsort(prob[i])[::-1]

    top1.append(synset[pred[0]])

    if (print_details == True):
      print(dash)
      print('{:^22s}{:^22s}{:^22s}'.format("Prediction 1","Prediction 2","Prediction 3"))
      print(dash)

      top5 = [synset[pred[i]] for i in range(5)]
      print('{:^27s}{:^30s}{:<30s}'.format(top5[0], top5[1], top5[2]))
      print(dash)
  return top1
