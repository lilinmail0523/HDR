# HDR

The aim of this project was to implement the photographic imaging techniques, including HDR imaging and tone mapping. The HDR image was created by followed steps:

1. Images alignment by Median Threshold Bitmap
2. HDR radiance map recovering by [Paul E. Debevec method](http://www.pauldebevec.com/Research/HDR/debevec-siggraph97.pdf)
3. Tone mapping by [Reinhard's global and local operators](http://www.cmap.polytechnique.fr/~peyre/cours/x2005signal/hdr_photographic.pdf)

# Results:
## Response Curve:
<center><img src="https://github.com/lilinmail0523/HDR/blob/master/python_ver/RC.png" width="50%" height="50%" /></center>

## Radiance Map:
![RadianceMap](https://github.com/lilinmail0523/HDR/blob/master/python_ver/RadianceMap.png)

## Tone mapping :
### Global:
<center><img src="https://github.com/lilinmail0523/HDR/blob/master/python_ver/ldrGlobal.png" width="75%" height="75%" /></center>



### Local:

<center><img src="https://github.com/lilinmail0523/HDR/blob/master/python_ver/ldrLocal.png" width="75%" height="75%" /></center>
