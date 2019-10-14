import numpy as np
import cv2
import sys

def quantize(image):
    R  = image[:,:,2]
    G  = image[:,:,1]
    B  = image[:,:,0]

    Q_R = np.floor(R/255*np.average(R))
    Q_G = np.floor(G/255*np.average(G))
    Q_B = np.floor(B/255*np.average(B))

    mr = Q_R.max()
    mg = Q_G.max()
    mb = Q_B.max()

    quantize_image = np.zeros(image.shape)
    quantize_image[:,:,2] = np.array(Q_R)
    quantize_image[:,:,1] = np.array(Q_G)
    quantize_image[:,:,0] = np.array(Q_B)
    cv2.imwrite('quantized.png', quantize_image)

if __name__ == '__main__':

    #file_name = sys.argv[1]
    file_name = 'test.jpg'

    image = cv2.imread(file_name)
    quantize_image = quantize(image)
