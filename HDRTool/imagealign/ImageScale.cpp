#include "ImageScale.h"

#include "Transform.h"
#include <string.h>
#include <math.h>

/*namespace QImageScale {
    struct QImageScaleInfo;
}*/
 

static void qt_qimageScaleAARGB(QImageScaleInfo *isi, unsigned int *dest, int dxx, int dyy, int dx, int dy, int dw, int dh, int dow, int sow);

    

Image<unsigned char>* ImageScale::scaled(Image<unsigned char>* image, unsigned int h, unsigned int w) const
{
    //, Qt::TransformationMode mode
	/*QSize newSize = size();
    newSize.scale(s, aspectMode);
    if (newSize == size())
        return *this;
*/
    Transform wm = Transform::fromScale((float)w / image->getWidth(), (float)h / image->getHeight());
    Image<unsigned char>* img = transformed(wm);
    return img;
}


Image<unsigned char>* ImageScale::smoothScaled(Image<unsigned char>* source, int w, int h) 
{
    Image<unsigned char>* src = source;
    //src = src.convertToFormat(QImage::Format_RGB32);
    return qSmoothScaleImage(src, w, h);
}

QImageScaleInfo* ImageScale::qimageCalcScaleInfo(Image<unsigned char> *img, int sw, int sh, int dw, int dh, char aa)
{
    QImageScaleInfo *isi;
    int scw, sch;

    scw = dw * img->getWidth() / sw;
    sch = dh * img->getHeight() / sh;

    isi = new QImageScaleInfo;
    if(!isi)
        return 0;
    memset(isi, 0, sizeof(QImageScaleInfo));

    isi->xup_yup = (abs(dw) >= sw) + ((abs(dh) >= sh) << 1);

    isi->xpoints = qimageCalcXPoints(img->getWidth(), scw);
    if(!isi->xpoints)
        return 0;
    isi->ypoints = qimageCalcYPoints((unsigned int *)img.scanLine(0), img.bytesPerLine() / 4, img->getHeight(), sch);
    if (!isi->ypoints)
        return 0;
    if(aa) {
        isi->xapoints = qimageCalcApoints(img->getWidth(), scw, isi->xup_yup & 1);
        if(!isi->xapoints)
            return 0;
        isi->yapoints = qimageCalcApoints(img->getHeight(), sch, isi->xup_yup & 2);
        if(!isi->yapoints)
            return 0;
    }
    return(isi);
} 

Image<unsigned char>* ImageScale::qSmoothScaleImage(Image<unsigned char>* src, int dw, int dh)
{
    Image<unsigned char>* buffer;
    
    int w = src->getWidth();
    int h = src->getHeight();
    QImageScaleInfo *scaleinfo =
        qimageCalcScaleInfo(src, w, h, dw, dh, true);
    if (!scaleinfo)
        return buffer;

    buffer = new Image<unsigned char>(3, dh, dw);//(dw, dh, src.format());
    

    qt_qimageScaleAARGB(scaleinfo, (unsigned int *)buffer.scanLine(0), 0, 0, 0, 0, dw, dh, dw, src.bytesPerLine() / 4);

    //qimageFreeScaleInfo(scaleinfo);
    return buffer;
} 






Image<unsigned char> *ImageScale::transformed(const Transform &matrix) const
{
	// source image data
	int ws = width();
	int hs = height();
	
	// target image data
	int wd;
	int hd;

    // compute size of target image
    Transform mat = trueMatrix(matrix, ws, hs);
    hd = round(abs(mat.m22()) * hs);
    wd = round(abs(mat.m11()) * ws);
    //hd = int(qAbs(mat.m22()) * hs + 0.9999);
    //wd = int(qAbs(mat.m11()) * ws + 0.9999);
    return smoothScaled(*this, wd, hd);
}







// FIXME: replace with qRed, etc... These work on pointers to pixels, not
// pixel values
#define A_VAL(p) (qAlpha(*p))
#define R_VAL(p) (qRed(*p))
#define G_VAL(p) (qGreen(*p))
#define B_VAL(p) (qBlue(*p))

#define INV_XAP                   (256 - xapoints[x])
#define XAP                       (xapoints[x])
#define INV_YAP                   (256 - yapoints[dyy + y])
#define YAP                       (yapoints[dyy + y])

unsigned int** ImageScale::qimageCalcYPoints(unsigned int *src,
                                              int sw, int sh, int dh)
{
    unsigned int **p;
    int i, j = 0;
    int val, inc, rv = 0;

    if(dh < 0){
        dh = -dh;
        rv = 1;
    }
    p = new unsigned int* [dh+1];

    int up = abs(dh) >= sh;
    val = up ? 0x8000 * sh / dh - 0x8000 : 0;
    inc = (sh << 16) / dh;
    for(i = 0; i < dh; i++){
        p[j++] = src + max(0, val >> 16) * sw;
        val += inc;
    }
    if(rv){
        for(i = dh / 2; --i >= 0; ){
            unsigned int *tmp = p[i];
            p[i] = p[dh - i - 1];
            p[dh - i - 1] = tmp;
        }
    }
    return(p);
}

int* ImageScale::qimageCalcXPoints(int sw, int dw)
{
    int *p, i, j = 0;
    int val, inc, rv = 0;

    if(dw < 0){
        dw = -dw;
        rv = 1;
    }
    p = new int[dw+1];

    int up = abs(dw) >= sw;
    val = up ? 0x8000 * sw / dw - 0x8000 : 0;
    inc = (sw << 16) / dw;
    for(i = 0; i < dw; i++){
        p[j++] = max(0, val >> 16);
        val += inc;
    }

    if(rv){
        for(i = dw / 2; --i >= 0; ){
            int tmp = p[i];
            p[i] = p[dw - i - 1];
            p[dw - i - 1] = tmp;
        }
    }
   return(p);
}

int* ImageScale::qimageCalcApoints(int s, int d, int up)
{
    int *p, i, j = 0, rv = 0;

    if(d < 0){
        rv = 1;
        d = -d;
    }
    p = new int[d];

    /* scaling up */
    if(up){
        int val, inc;

        val = 0x8000 * s / d - 0x8000;
        inc = (s << 16) / d;
        for(i = 0; i < d; i++){
            int pos = val >> 16;
            if (pos < 0)
                p[j++] = 0;
            else if (pos >= (s - 1))
                p[j++] = 0;
            else
                p[j++] = (val >> 8) - ((val >> 8) & 0xffffff00);
            val += inc;
        }
    }
    /* scaling down */
    else{
        int val, inc, ap, Cp;
        val = 0;
        inc = (s << 16) / d;
        Cp = ((d << 14) / s) + 1;
        for(i = 0; i < d; i++){
            ap = ((0x100 - ((val >> 8) & 0xff)) * Cp) >> 8;
            p[j] = ap | (Cp << 16);
            j++;
            val += inc;
        }
    }
    if(rv){
        int tmp;
        for(i = d / 2; --i >= 0; ){
            tmp = p[i];
            p[i] = p[d - i - 1];
            p[d - i - 1] = tmp;
        }
    }
    return(p);
}

/*QImageScaleInfo* QImageScale::qimageFreeScaleInfo(QImageScaleInfo *isi)
{
    if(isi){
        delete[] isi->xpoints;
        delete[] isi->ypoints;
        delete[] isi->xapoints;
        delete[] isi->yapoints;
        delete isi;
    }
    return 0;
}*/


/* scale by area sampling - IGNORE the ALPHA byte*/
static void qt_qimageScaleAARGB(QImageScaleInfo *isi, unsigned int *dest,
                                int dxx, int dyy, int dx, int dy, int dw,
                                int dh, int dow, int sow)
{
    unsigned int *sptr, *dptr;
    int x, y, end;
    unsigned int **ypoints = isi->ypoints;
    int *xpoints = isi->xpoints;
    int *xapoints = isi->xapoints;
    int *yapoints = isi->yapoints;

    end = dxx + dw;
    
    /* fully optimized (i think) - onyl change of algorithm can help */
    /* if we're scaling down horizontally & vertically */
    
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, Cy, i, j;
        unsigned int *pix;
        int r, g, b, rx, gx, bx;
        int xap, yap;

        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                sptr = ypoints[dyy + y] + xpoints[x];
                pix = sptr;
                sptr += sow;
                rx = R_VAL(pix) * xap;
                gx = G_VAL(pix) * xap;
                bx = B_VAL(pix) * xap;
                pix++;
                for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                    rx += R_VAL(pix) * Cx;
                    gx += G_VAL(pix) * Cx;
                    bx += B_VAL(pix) * Cx;
                    pix++;
                }
                if(i > 0){
                    rx += R_VAL(pix) * i;
                    gx += G_VAL(pix) * i;
                    bx += B_VAL(pix) * i;
                }

                r = (rx >> 5) * yap;
                g = (gx >> 5) * yap;
                b = (bx >> 5) * yap;

                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix = sptr;
                    sptr += sow;
                    rx = R_VAL(pix) * xap;
                    gx = G_VAL(pix) * xap;
                    bx = B_VAL(pix) * xap;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += R_VAL(pix) * Cx;
                        gx += G_VAL(pix) * Cx;
                        bx += B_VAL(pix) * Cx;
                        pix++;
                    }
                    if(i > 0){
                        rx += R_VAL(pix) * i;
                        gx += G_VAL(pix) * i;
                        bx += B_VAL(pix) * i;
                    }

                    r += (rx >> 5) * Cy;
                    g += (gx >> 5) * Cy;
                    b += (bx >> 5) * Cy;
                }
                if(j > 0){
                    pix = sptr;
                    sptr += sow;
                    rx = R_VAL(pix) * xap;
                    gx = G_VAL(pix) * xap;
                    bx = B_VAL(pix) * xap;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        rx += R_VAL(pix) * Cx;
                        gx += G_VAL(pix) * Cx;
                        bx += B_VAL(pix) * Cx;
                        pix++;
                    }
                    if(i > 0){
                        rx += R_VAL(pix) * i;
                        gx += G_VAL(pix) * i;
                        bx += B_VAL(pix) * i;
                    }

                    r += (rx >> 5) * j;
                    g += (gx >> 5) * j;
                    b += (bx >> 5) * j;
                }

                *dptr = qRgb(r >> 23, g >> 23, b >> 23);
                dptr++;
            }
        }
    
}



