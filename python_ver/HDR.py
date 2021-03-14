import cv2
import numpy as np
from random import randint
import matplotlib.pyplot as plt
import math

from mpl_toolkits.axes_grid1 import make_axes_locatable






#reference from debevec's HDR method (gsolve.m)
def gsolve(Z, B, l, w):
    n = 256
    A = np.zeros((Z.shape[0] * Z.shape[1] + n + 1, n + Z.shape[0]))
    b = np.zeros((A.shape[0],1))

    #Include the data-fitting equations
    k = 0
    for i in range(Z.shape[0]):
        for j in range(Z.shape[1]):
            wij = w[Z[i, j] ]
            A[k, Z[i, j]] = wij
            A[k, n+i] = -wij
            b[k, 0] = wij * B[j]
            k = k+1

    A[k, 128] = 1
    k = k+1

    #include the smoothness equations
    for i in range (n-2):
        A[k, i] = l * w[i+1]
        A[k, i+1] = -2 * l * w[i+1]
        A[k, i+2] = l * w[i+1]
        k= k+1
    #solve the system using SVD
    x = np.linalg.lstsq(A, b)[0]

    g = x[0:n]
    lE = x[n:x.shape[0]]

    return g, lE

def PlotResponseCurve(gList):
    plt.figure(figsize=(8, 8))
    blueline, = plt.plot(gList[0], range(256),color = 'blue')
    greenline, = plt.plot(gList[1], range(256),color = 'green')
    redline, = plt.plot(gList[2], range(256),color = 'red')
    plt.suptitle('Response Curve')
    plt.ylabel('Pixel Value (Zij)')
    plt.xlabel('Log Exposure (Ei*(delta t)j)')
    plt.legend(handles=[blueline, greenline, redline], labels = ['Blue', 'Green', 'Red'], loc = 'best', fontsize = 'x-large')
    plt.savefig('RC.png')


def PlotRadianceMap(lnE, filename):
    fig = plt.figure(figsize=(12, 5))
    ax1,ax2, ax3 = fig.subplots(1, 3)
    for ax, color, index in zip((ax1, ax2, ax3), ('Blue', 'Green', 'Red'), (0,1,2)):
        im = ax.imshow(lnE[:,:,index], cmap='jet')
        ax.set_axis_off()
        divider = make_axes_locatable(ax)
        cax = divider.append_axes("right", size="7%", pad="2%")
        ax.set_title(color, fontsize = 20)
        cbar = fig.colorbar(im, cax = cax)
        cbar.ax.tick_params(labelsize=8)
    fig.savefig(filename)


#Numpy radiance writer
def RadianceSave(HDR, filename):
    image = np.zeros(HDR.shape)
    image[:,:,0] = HDR[:,:,2]
    image[:,:,1] = HDR[:,:,1]
    image[:,:,2] = HDR[:,:,0]
    f = open(filename + ".hdr", "wb")
    f.write(b"#?RADIANCE\n# Made with Python & Numpy\nFORMAT=32-bit_rle_rgbe\n\n")
    title = ("-Y {0} +X {1}\n".format(image.shape[0], image.shape[1]))
    f.write(title.encode(encoding='UTF-8'))

    brightest = np.maximum(np.maximum(image[...,0], image[...,1]), image[...,2])
    mantissa = np.zeros_like(brightest)
    exponent = np.zeros_like(brightest)
    np.frexp(brightest, mantissa, exponent)

    ## 0-255 , 256 will have noisy in the result image
    scaled_mantissa = mantissa * 255.0 / brightest
    rgbe = np.zeros((image.shape[0], image.shape[1], 4), dtype=np.uint8)
    rgbe[...,0:3] = np.around(image[...,0:3] * scaled_mantissa[...,None])
    rgbe[...,3] = np.around(exponent + 128)

    rgbe.flatten().tofile(f)
    f.close()

      
def HDR(ImgList, ExposureList):
    #Read Image
    cImgs = [cv2.imread("Aligned_"+ImgPath, 1) for ImgPath in ImgList]    


    #Samping 100 points
    SampleSize = 100
    Row = cImgs[0].shape[0]
    Col = cImgs[0].shape[1]
    Channels = cImgs[0].shape[2]

    Z = np.zeros((SampleSize, len(cImgs), 3), dtype = int)
    for i in range(SampleSize):
        y = randint(0, Row - 1)
        x = randint(0, Col - 1)
        for j in range(len(cImgs)):
            Z[i][j] = cImgs[j][y][x]

    #Read exposure 
    ExpList = [float(i) for i in ExposureList]
    B = np.log(ExpList)

    #Calculate weighting function w(z)
    w = [z if z <= 0.5*255 else 255 - z for z in range(256)]

    #lamda l : smooth factor
    l = 50


    gList = []
    lEList = []
    for channel in range (Channels):
        Z_channel = Z[:,:,channel]
        g, lE = gsolve(Z_channel, B, l, w)
        gList.append(g)
        lEList.append(lE)


    #Constructing HDR rafiance map
    
    E = np.zeros(cImgs[0].shape)
    #w nparray for indices 
    w = np.array(w)
    for channel in range(Channels):
        w_sum = np.zeros((Row, Col))
        ln_rad_sum = np.zeros((Row, Col))
        for index, img in enumerate(cImgs):
            pix = img[:,:,channel].flatten()
            w_pix = w[pix].reshape(Row, Col)
            g_pix = (gList[channel][pix] - B[index]).reshape(Row, Col)
            ln_rad_sum += w_pix * g_pix
            w_sum += w_pix

        E[:,:,channel] = np.exp(ln_rad_sum / (w_sum + 1e-8))
    print(np.log(E).max())

    PlotRadianceMap(np.log(E), 'RadianceMap.png')

    PlotResponseCurve(gList)
    RadianceSave(E, "HDR")







if __name__ == '__main__':
    with open('image_list.txt', 'r') as f:
        ImgList = [line.strip() for line in f]
    with open('exposure.txt', 'r') as f:
        ExposureList = [line.strip() for line in f]

    HDR(ImgList, ExposureList)
