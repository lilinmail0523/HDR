
import numpy as np
import cv2
import math

#Reinhard Global:
#http://www.cmap.polytechnique.fr/~peyre/cours/x2005signal/hdr_photographic.pdf
def PhotographcGlobal(Lw, delta = 1e-5, a = 0.36, gamma = 2.2):
    Lumin_Lw = 0.27 * Lw[:,:,0] + 0.67 * Lw[:,:,1] + 0.06 * Lw[:,:,2]
    Lw_avg = np.exp(np.mean(np.log(delta + Lumin_Lw)))
    Lm = (a / Lw_avg) * Lumin_Lw
    Ld = Lm * (1+ Lm / (np.max(Lm) ** 2))/ (1+Lm)

    ldr = np.zeros(Lw.shape)
    for channel in range(Lw.shape[2]):
        ldr[:,:,channel] = Lw[:,:,channel] * (Ld / Lumin_Lw)

    #Gamma correction
    ldr = np.clip(np.power(ldr, 1/gamma), 0, 1)

    return (ldr * 255).astype(np.uint8)

#Reinhard Local:
#http://www.cmap.polytechnique.fr/~peyre/cours/x2005signal/hdr_photographic.pdf
#parameter perference: HDR Toolbox
def PhotographcLocal(Lw, delta = 1e-6, a = 0.36, phi = 10, eps = 0.05, gamma = 2.2):
    Lumin_Lw = 0.27 * Lw[:,:,0] + 0.67 * Lw[:,:,1] + 0.06 * Lw[:,:,2]
    Lw_avg = np.exp(np.mean(np.log(delta + Lumin_Lw)))
    Lm = (a / Lw_avg) * Lumin_Lw


    rows, cols = Lm.shape

    alpha = 1 / (2*math.sqrt(2))
    ScaleLevel = 9

    V1 = np.zeros(( rows, cols, ScaleLevel))
    V = np.zeros((rows, cols,ScaleLevel))

    blur_prev = Lm
    for scale in range(ScaleLevel):
        s = (1.6 ** scale)
        sigma = alpha * s
        blur = cv2.GaussianBlur(Lm, (0, 0), sigmaX = sigma, sigmaY = sigma, borderType = cv2.BORDER_REPLICATE)
        vs = np.abs((blur - blur_prev) / (2 ** phi * a / s ** 2 + blur_prev))
        V1[:,:,scale] = blur
        V[:,:,scale] = vs
        blur_prev = blur
    
    
    smax = np.zeros((rows, cols), dtype = int)
    for i in range(rows):
        for j in range(cols):
            for scale in range(ScaleLevel):
                if V[i, j, scale] > eps and scale > 0:
                    smax[i, j] = scale - 1
                    break

        
    I, J = np.ogrid[:rows, :cols]
    Lsmax = V1[I, J,smax]


    Ld = Lm / (1 + Lsmax)

    ldr = np.zeros(Lw.shape)
    for channel in range(Lw.shape[2]):
        ldr[:,:,channel] = Lw[:,:,channel] * (Ld /Lumin_Lw)

    #Gamma correction
    ldr = np.clip(np.power(ldr, 1/gamma), 0, 1)

    return (ldr * 255).astype(np.uint8)



    

def ToneMapping(filename):
    hdr = cv2.imread(filename, -1)
    ldrGlobal = PhotographcGlobal(hdr)
    ldrLocal = PhotographcLocal(hdr)

    cv2.imwrite("ldrGlobal.png", ldrGlobal)
    cv2.imwrite("ldrLocal.png", ldrLocal)

if __name__ == "__main__":
    ToneMapping("HDR.hdr")