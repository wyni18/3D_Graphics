/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
 -- NOTE: this function is optional and not part of the API, but you may want to use it within the display function.
*/
	*framebuffer = new char [3 * sizeof(char) * width * height];
	if (framebuffer == NULL) {
		return GZ_FAILURE;
	}

	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, int xRes, int yRes)
{
/* create a display:
  -- allocate memory for indicated resolution
  -- pass back pointer to GzDisplay object in display
*/
	*display = new GzDisplay;
	if(*display == NULL)
	{
		return GZ_FAILURE;
	}

	if(xRes > MAXXRES)
	{
		xRes = MAXXRES;
	}
	else if(xRes < 0)
	{
		xRes = 0;
	}
	(*display)->xres = xRes;

	if(yRes > MAXYRES)
	{
		yRes = MAXYRES;
	}
	else if(yRes < 0)
	{
		yRes = 0;
	}
	(*display)->yres = yRes;
	
	(*display)->fbuf = new GzPixel[xRes * yRes];

	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
	delete[] display->fbuf;
	delete display;

	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes)
{
/* pass back values for a display */
	if(display == NULL)
	{
		return GZ_FAILURE;
	}

	*xRes = display->xres;
	*yRes = display->yres;

	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */
	if(display == NULL)
	{
		return GZ_FAILURE;
	}

	for(int i = 0; i < display->xres * display->yres; i++)
	{
		display->fbuf[i].red = 5890;
		display->fbuf[i].green = 0;
		display->fbuf[i].blue = 0;
		display->fbuf[i].alpha = 1;
		display->fbuf[i].z = 0;

	}

	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	if (display == NULL)
	{
		return GZ_FAILURE;
	}
	if(i < 0 || i > (display->xres) || j < 0 || j > (display->yres))
	{
		return GZ_FAILURE;
	}

	if(r < 0)
	{
		r = 0;
	}
	if(r>4095)
	{
		r = 4095;
	}

	if(g < 0)
	{
		g = 0;
	}
	if(g>4095)
	{
		g = 4095;
	}

	if(b < 0)
	{
		b = 0;
	}
	if(b>4095)
	{
		b = 4095;
	}

	int temp = ARRAY(i,j);
	display->fbuf[temp].red = r;
	display->fbuf[temp].green = g;
	display->fbuf[temp].blue = b;
	display->fbuf[temp].alpha = a;
	display->fbuf[temp].z = z;

	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	if(display == NULL)
	{
		return GZ_FAILURE;
	}
	if (i < 0 || i > display->xres || j < 0 ||j > display->yres)
	{
		return GZ_FAILURE;
	}

	GzPixel temp = display->fbuf[ARRAY(i,j)];
	*r = temp.red;
	*g = temp.green;
	*b = temp.blue;
	*a = temp.alpha;
	*z = temp.z;

	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file -- "P6 %d %d 255\r" */
	if(display == NULL)
	{
		return GZ_FAILURE;
	}

	fprintf(outfile, "P6 %d %d 255\r", display->xres, display->yres);

	for (int i = 0; i < (display->xres) * (display->yres); i++)
	{
		GzPixel temp = display->fbuf[i];
        char Red = temp.red >> 4;
        char Green = temp.green >> 4;
        char Blue = temp.blue >> 4;
        fprintf(outfile, "%c%c%c", Red, Green, Blue);
    }

	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

	/* write pixels to framebuffer: 
		- Put the pixels into the frame buffer
		- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
		- Not red, green, and blue !!!
	*/
	for (int i = 0; i < (display->xres) * (display->yres); i++)
	{
		GzPixel temp = display->fbuf[i];
		char Red = temp.red >> 4;
        char Green = temp.green >> 4;         
		char Blue = temp.blue >> 4;

		framebuffer[i * 3] = Blue;   
        framebuffer[i * 3 + 1] = Green;   
        framebuffer[i * 3 + 2] = Red;
    }

	return GZ_SUCCESS;
}