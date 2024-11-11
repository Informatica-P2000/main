#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define false 0
#define true 1

#define REDTEXT 1
#define GREENTEXT 2
#define YELLOWTEXT 3
#define BLUETEXT 4
#define MAGENTATEXT 5
#define CYANTEXT 6
#define WHITETEXT 7

#define REDGFS 17
#define GREENGFS 18
#define YELLOWGFS 19
#define BLUEGFS 20
#define MAGENTAGFS 21
#define CYANGFS 22
#define WHITEGFS 23


#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

//Create reference to video memory
__at (0x5000) char VIDMEM[];
char* vidmem = VIDMEM;

unsigned short yAdrLUT[75];
unsigned char pwr2LUT[6];

unsigned char pwr2 (unsigned char e){
    unsigned char res = 1;
    for (int i = 0; i < e; i++) res <<= 1;
    return res;
}

void startGraphics(void){
    for (int i = 0; i < 6; i++) pwr2LUT[i] = pwr2(i);
    for (int i = 0; i < 75; i++) yAdrLUT[i] = 0x5000 + 80 * (i/3) + (i%3);
    for (int i = 0; i < 1919; i++) vidmem[i] = 32;
    for (int i = 0; i <= 1760; i += 80) vidmem[i] = WHITEGFS;

    return;
}

//Sets a given pixel. Returns 0 on change, 1 on no change, 2 on out-of-bounds
//Wt = white (1 if pixel will be set to white, 0 if set to black)
unsigned char setPixel(unsigned char x, unsigned char y, unsigned char wt){
    if (x < 2 || x >= 160 || y >= 75) return 2;

    //unsigned short yadrLUTy = yAdrLUT[y];
    unsigned char* charAdr  = (unsigned char*) (yAdrLUT[y] & 0xFFFC) + (x >> 1);
    unsigned char  prevChar = *charAdr;
    unsigned char  charXY   = 0;
    unsigned char  pixelNum = (x % 2) + 2 * (yAdrLUT[y] & 3);

    if (prevChar >= 32 && prevChar <= 63){
    //if ((prevChar & (unsigned char) 0xE0) == (unsigned char) 0x20){
        if ((((prevChar - 32) >> pixelNum) & 1) == wt)
            return 1;

        if (wt) charXY = prevChar + pwr2LUT[pixelNum];
        else charXY = prevChar - pwr2LUT[pixelNum];

        if (charXY > 63) charXY += 32;
    }
    if (prevChar >= 96 && prevChar <= 127){
        if ((((prevChar - 64) >> pixelNum) & 1) == wt)
            return 1;

        if (wt) charXY = prevChar + pwr2LUT[pixelNum];
        else charXY = prevChar - pwr2LUT[pixelNum];
        
        if (charXY < 96) charXY -= 32;
    }

    *charAdr = charXY;

    return 0;
}

void drawLine(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char wt){
    signed char dx = abs(x1 - x0);
    signed char sx = x0 < x1 ? 1 : -1;

    signed char dy = -abs(y1 - y0);
    signed char sy = y0 < y1 ? 1 : -1;

    int error = dx + dy;
    int e2 ;

    while(1){
        setPixel(x0, y0, wt);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * error;

        if (e2 - dy >= 0){
            error += dy;
            x0 += sx;
        }

        if (e2 - dx < 0){
            error += dx;
            y0 += sy;
        }
    }

    return;
}

//TODO: Colors
void drawLineColor(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char color){
    signed char dx = abs(x1 - x0);
    signed char sx = x0 < x1 ? 1 : -1;

    signed char dy = -abs(y1 - y0);
    signed char sy = y0 < y1 ? 1 : -1;

    int error = dx + dy;
    int e2 ;

    unsigned char xUp;

    * (unsigned char*) (yAdrLUT[y0] & 0xFFFC + (x0 >> 1) - 1) == color;
    * (unsigned char*) (yAdrLUT[y0] & 0xFFFC + (x0 >> 1) + 1) == WHITEGFS;
    * (unsigned char*) (yAdrLUT[y1] & 0xFFFC + (x1 >> 1) - 1) == color;
    * (unsigned char*) (yAdrLUT[y1] & 0xFFFC + (x1 >> 1) + 1) == WHITEGFS;

    while(1){
        setPixel(x0, y0, true);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * error;

        if (e2 - dy >= 0){
            error += dy;
            x0 += sx;
            xUp = true;
        }

        if (e2 - dx < 0){
            error += dx;
            y0 += sy;
            xUp = false;
        }
    }

    return;
}

void horzLine(unsigned char x0, unsigned char x1, unsigned char y, unsigned char wt){
    for (unsigned char i = x0; i <= x1; i++){
        setPixel(i, y, wt);
    }
    for (unsigned char i = x1; i <= x0; i++){
        setPixel(i, y, wt);
    }
}

void horzLineColor(unsigned char x0, unsigned char x1, unsigned char y, unsigned char color){
    unsigned char xmin = MIN(x0, x1);
    unsigned char xmax = MAX(x0, x1);

    * (unsigned char*) ((yAdrLUT[y] & 0xFFFC) + (xmin >> 1) - 1) = color;
    * (unsigned char*) ((yAdrLUT[y] & 0xFFFC) + (xmax >> 1) + 1) = WHITEGFS;

    for (unsigned char i = xmin; i <= xmax; i++){
        setPixel(i, y, true);
    }
}

void vertLine(unsigned char x, unsigned char y0, unsigned char y1, unsigned char wt){
    for (unsigned char i = y0; i <= y1; i++){
        setPixel(x, i, wt);
    }
    for (unsigned char i = y1; i <= y0; i++){
        setPixel(x, i, wt);
    }
}

void vertLineColor(unsigned char x, unsigned char y0, unsigned char y1, unsigned char color){
    unsigned char ymin = MIN(y0, y1);
    unsigned char ymax = MAX(y0, y1);

    for (unsigned int i = (yAdrLUT[ymin] & 0xFFFC) + (x >> 1) - 1; i <= (yAdrLUT[ymax] & 0xFFFC) + (x >> 1) - 1; i += 80){
        * (unsigned char*) i = color;
    }

    vertLine(x, y0, y1, true);

    for (unsigned int i = (yAdrLUT[ymin] & 0xFFFC) + (x >> 1) + 1; i <= (yAdrLUT[ymax] & 0xFFFC) + (x >> 1) + 1; i += 80)
        * (unsigned char*) i = WHITEGFS;
}

void rectangle(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char wt){
    horzLine(x0, x1, y0, wt);
    horzLine(x0, x1, y1, wt);
    vertLine(x0, y0, y1, wt);
    vertLine(x1, y0, y1, wt);
}

void rectangleColor(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char color){
    unsigned char xmin = MIN(x0, x1);
    unsigned char xmax = MAX(x0, x1);
    unsigned char ymin = MIN(y0, y1);
    unsigned char ymax = MAX(y0, y1);

    sprintf(vidmem + 1760, "ymin: %d, ymax: %d", ymin, ymax);

    for (unsigned int i = (yAdrLUT[ymin] & 0xFFFC) + (xmin >> 1) - 1; i <= (yAdrLUT[ymax] & 0xFFFC) + (xmin >> 1) - 1; i += 80){
        * (unsigned char*) i = color;
    }

    rectangle(x0, y0, x1, y1, true);

    for (unsigned int i = (yAdrLUT[ymin] & 0xFFFC) + (xmax >> 1) + 1; i <= (yAdrLUT[ymax] & 0xFFFC) + (xmax >> 1) + 1; i += 80)
        * (unsigned char*) i = WHITEGFS;
}

void fillRectangle(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char wt){

    unsigned char xmin = MIN(x0, x1);
    unsigned char xmax = MAX(x0, x1);
    unsigned char ymin = MIN(y0, y1);
    unsigned char ymax = MAX(y0, y1);

    unsigned char xminOl = xmin % 2;
    unsigned char xmaxOl = xmax % 2;

    if (xminOl) vertLine(xmin, ymin, ymax, wt);
    if (!xmaxOl) vertLine(xmax, ymin, ymax, wt);

    unsigned char yminOl = yAdrLUT[ymin] & 3;
    unsigned char ymaxOl = yAdrLUT[ymax] & 3;

    switch (yminOl)
    {
    case 1:
        horzLine(xmin, xmax, ymin, wt);
        horzLine(xmin, xmax, ymin + 1, wt);
        break;

    case 2:
        horzLine(xmin, xmax, ymin, wt);
        break;
    
    default:
        break;
    }

    switch (ymaxOl)
    {
    case 0:
        horzLine(xmin, xmax, ymax, wt);
        break;

    case 1:
        horzLine(xmin, xmax, ymax, wt);
        horzLine(xmin, xmax, ymax - 1, wt);
        break;
    
    default:
        break;
    }

    for (unsigned int yAdr = yAdrLUT[ymin + 2] & 0xFFFC; yAdr <= (yAdrLUT[ymax - 2] & 0xFFFC); yAdr += 80){
        for (unsigned int xAdr = (1 + xmin) >> 1; xAdr < (xmax + 1) >> 1; xAdr++){
            unsigned char* p_char = (unsigned char*) (yAdr + xAdr);
            if (wt) *p_char = 127;
            else *p_char = 32;
        }
    }
}

void fillRectangleColor(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char color){
    unsigned char xmin = MIN(x0, x1);
    unsigned char xmax = MAX(x0, x1);
    unsigned char ymin = MIN(y0, y1);
    unsigned char ymax = MAX(y0, y1);

    for (unsigned int i = (yAdrLUT[ymin] & 0xFFFC) + (xmin >> 1) - 1; i <= (yAdrLUT[ymax] & 0xFFFC) + (xmin >> 1) - 1; i += 80)
        * (unsigned char*) i = color;

    fillRectangle(x0, y0, x1, y1, true);

    for (unsigned int i = (yAdrLUT[ymin] & 0xFFFC) + (xmax >> 1) + 1; i <= (yAdrLUT[ymax] & 0xFFFC) + (xmax >> 1) + 1; i += 80)
        * (unsigned char*) i = WHITEGFS;
}

void drawText(unsigned char x, unsigned char y, char* text, unsigned char textLen, unsigned char dblH, unsigned char color){
    unsigned char* startAdr = (unsigned char*) ((yAdrLUT[y] & 0xFFFC) + (x >> 1));

    if (dblH) {*(startAdr - 2) = color; *(startAdr - 1) = 0xD;}
    else *(startAdr - 1) = color;

    for (unsigned char i = 0; i < textLen; i++){
        *(startAdr + i) = text[i];
    }
    
    if (dblH) {{*(startAdr + textLen) = 0xC; *(startAdr + textLen + 1) = WHITEGFS;}}
    else {*(startAdr + textLen) = WHITEGFS;}
}

void circle(unsigned char xm, unsigned char ym, unsigned char r, unsigned char wt){
    unsigned char t1 = r / 16;
    unsigned char x = r;
    unsigned char y = 0;

    while (x - y >= 0){
        setPixel(xm + x, ym + y, wt);
        setPixel(xm - x, ym + y, wt);
        setPixel(xm + x, ym - y, wt);
        setPixel(xm - x, ym - y, wt);
        setPixel(xm + y, ym + x, wt);
        setPixel(xm - y, ym + x, wt);
        setPixel(xm + y, ym - x, wt);
        setPixel(xm - y, ym - x, wt);
        y += 1;
        t1 += y;
        char t2 = t1 - x;
        if (t2 >= 0){
            t1 = t2;
            x -= 1;
        }
    }

    return;
}

void fillCircle(unsigned char xm, unsigned char ym, unsigned char r, unsigned char wt){
    unsigned char t1 = r / 16;
    unsigned char x = r;
    unsigned char y = 0;

    while (x - y >= 0){
        horzLine(xm - x, xm + x, ym + y, wt);
        horzLine(xm - x, xm + x, ym - y, wt);
        horzLine(xm - y, xm + y, ym + x, wt);
        horzLine(xm - y, xm + y, ym - x, wt);
        y += 1;
        t1 += y;
        char t2 = t1 - x;
        if (t2 >= 0){
            t1 = t2;
            x -= 1;
        }
    }

    return;
}

int main(void) {

    startGraphics();

    unsigned short *p_counter = (unsigned short*) 0x6010;
    short startTime = *p_counter;

    fillCircle(10, 10, 7, true);

    drawLine(10, 10, 40, 50, true);
    horzLine(10, 50, 10, true);

    rectangleColor(8, 35, 20, 50, REDGFS);

    circle(50, 35, 10, true);

    vertLineColor(30, 0, 20, MAGENTAGFS);
    vertLine(60, 0, 20, true);

    fillRectangleColor(55, 10, 70, 30, CYANGFS);

    drawLine(55, 10, 70, 30, false);
    drawLine(70, 10, 55, 30, false);

    vertLineColor(25, 35, 45, GREENGFS);

    drawText(60, 50, "hai", 3, true, YELLOWTEXT);
    drawText(47, 35, "Mooi", 4, false, BLUETEXT);

    /*
    for (unsigned char x = 2; x < 75; x++)
        for (unsigned char y = 0; y < 60; y++)
            setPixel(x, y, true);

    //while(0) {} // set infinite loop
    */

    short endTime = *p_counter;

    sprintf(vidmem + 1840, "done in %d * 0.02 secs", (endTime - startTime));

    return 0;
}