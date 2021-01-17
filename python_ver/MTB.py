import cv2
import numpy as np




def bitmap(img, bits):
    med = np.median(img)
    tb =  img > med
    eb = np.abs(img - med) >= bits
    return tb, eb

#use np.roll to shift image
def ShiftImg(Img, dy, dx):
    img = np.roll(Img, dy, axis = 0)
    img = np.roll(img, dx, axis = 1)
    if dy >0:
        img[:dy, :] = 0
    elif dy < 0:
        img[dy:,:] = 0
    if dx >0:
        img[:,:dx] = 0
    elif dx <0:
        img[:, dx:] = 0
    return img

def GetOffset(GroundImage, Image2Shift, levels, bits = 5):
    offset = [0, 0]
    if levels > 0:
        h, w = GroundImage.shape
        ShrinkGroundImage = cv2.resize(GroundImage,(h//2, w//2))
        ShrinkImage2Shift = cv2.resize(Image2Shift,(h//2, w//2))
        offset = GetOffset(ShrinkGroundImage,ShrinkImage2Shift, levels - 1)


    minerror = GroundImage.shape[0] * GroundImage.shape[1]

    GDbitmap,Exbitmap = bitmap(GroundImage, bits)
    Imgbitmap,ImgExbitmap = bitmap(Image2Shift, bits)
    for i in range(-1,2):
        for j in range (-1, 2):
            yOffset = offset[0]*2 + i
            xOffset = offset[1]*2 + j

            ImgbitmapSft = ShiftImg(Imgbitmap, yOffset, xOffset)
            ImgExbitmapSft = ShiftImg(ImgExbitmap, yOffset, xOffset)


            diff = np.logical_xor(GDbitmap ,ImgbitmapSft)
            diff = np.logical_and(diff, Exbitmap)
            diff = np.logical_and(diff, ImgExbitmapSft)
            error_cnt = np.sum(diff) 


            if minerror > error_cnt:
                minerror = error_cnt
                Shifts = [xOffset, yOffset]

    offset = Shifts
    return Shifts




def MTB(ImgList):
    #Read Image
    gImgs = [cv2.imread(ImgPath, 0) for ImgPath in ImgList]
    cImgs = [cv2.imread(ImgPath, 1) for ImgPath in ImgList]
    #Threshold (t = median of image) bitmap/Exclusive bitmap

    offset = [0, 0]


    cv2.imwrite('Aligned_' + ImgList[len(gImgs)//2], cImgs[len(gImgs)//2])
    for i in range(len(gImgs)):
        
        offset = GetOffset(gImgs[len(gImgs)//2],gImgs[i], 4)

        print('Image: ', ImgList[i], ' offsets = ', offset)
        cImgs[i] = ShiftImg(cImgs[i], offset[1], offset[0])
        cv2.imwrite('Aligned_' + ImgList[i], cImgs[i])

if __name__ == '__main__':

    with open('image_list.txt', 'r') as f:
        ImgList = [line.strip() for line in f]
    print(ImgList)
    MTB(ImgList)