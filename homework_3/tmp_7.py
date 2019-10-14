import numpy as np
import cv2
import sys

def median_cut(img, K):
    K = int(K)
    Z = img.reshape((-1, 3))

    Z = np.float32(Z)

    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 10, 1.0)

    ret, label, center = cv2.kmeans(Z,K,None,criteria,10,cv2.KMEANS_RANDOM_CENTERS)

    center = np.uint8(center)
    res = center[label.flatten()]
    res2 = res.reshape((img.shape))

    return res2

if __name__ == '__main__':

    filename = sys.argv[1]
    try:
        K = sys.argv[2]
    except:
        K = 2

    image = cv2.imread(filename)
    out = median_cut(image, K)
    cv2.imshow("output image k={}".format(K), out)
    cv2.waitKey()
    cv2.destroyAllWindows()

