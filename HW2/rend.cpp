#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"

using namespace std;

short	ctoi(float color);


int GzNewRender(GzRender **render, GzDisplay *display)
{
/* 
- malloc a renderer struct
- keep closed until BeginRender inits are done
- span interpolator needs pointer to display for pixel writes
- check for legal class GZ_Z_BUFFER_RENDER
*/
	*render = new GzRender;
	if (*render == NULL) 
	{
		return GZ_FAILURE;
	}
	(*render)->display = display;

	return GZ_SUCCESS;
}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	//GzFreeDisplay(render->display);
	delete render;
	return GZ_SUCCESS;
}


int GzBeginRender(GzRender	*render)
{
/* 
- set up for start of each frame - init frame buffer
*/
	if (render == NULL) 
	{
		return GZ_FAILURE;
	}
	if (render->display == NULL) 
	{
		return GZ_FAILURE;
	}
	GzInitDisplay(render->display);
	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer *valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	for (int i = 0; i < numAttributes; i++)
	{
		if(nameList[i] == GZ_RGB_COLOR)
		{
			GzColor* pointer = (GzColor*)(valueList[i]);
			render->flatcolor[0] = pointer[i][0];
			render->flatcolor[1] = pointer[i][1];
			render->flatcolor[2] = pointer[i][2];
		}
	}
	return GZ_SUCCESS;
}


int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
	GzPointer *valueList) 
/* numParts - how many names and values */
{
/* 
- pass in a triangle description with tokens and values corresponding to
      GZ_NULL_TOKEN:		do nothing - no values
      GZ_POSITION:		3 vert positions in model space
- Invoke the scan converter and return an error code
*/
	for (int i = 0; i < numParts; i++)
	{
		if (nameList[i] == GZ_POSITION)
		{
			// achieve the coordinates
			GzCoord* coord = (GzCoord*) valueList[i];
			
			// sort by Y coordinate
			for (int ii = 0; ii < 2; ii++) {
				int k = ii;
				for (int j = ii + 1; j < 3; j++) {
					if (coord[k][Y] > coord[j][Y]) {
						k = j;
					}
				}
				if (k != ii) {
					float tempX = coord[ii][X];
					float tempY = coord[ii][Y];
					float tempZ = coord[ii][Z];
					coord[ii][X] = coord[k][X];
					coord[ii][Y] = coord[k][Y];
					coord[ii][Z] = coord[k][Z];
					coord[k][X] = tempX;
					coord[k][Y] = tempY;
					coord[k][Z] = tempZ;
				}
			}

			// get vertex coordinates
			Vertex *vert = new Vertex[3];
			vert[0].xc = coord[0][X];
			vert[1].xc = coord[1][X];
			vert[2].xc = coord[2][X];
			vert[0].yc = coord[0][Y];
			vert[1].yc = coord[1][Y];
			vert[2].yc = coord[2][Y];
			vert[0].zc = coord[0][Z];
			vert[1].zc = coord[1][Z];
			vert[2].zc = coord[2][Z];

			Edge *edge = new Edge[3];

			int type;
			// setup edges and sort edges;
			edge[0].start = vert[0];
			edge[0].end = vert[1];
			edge[1].start = vert[1];
			edge[1].end = vert[2];
			edge[2].start = vert[0];
			edge[2].end = vert[2];
			if (vert[0].yc == vert[1].yc)
			{
				type = 0;   // top horizontal 
			}
			else if(vert[1].yc == vert[2].yc)
			{
				type = 1; // bottom horizontal
			}
			else
			{
				edge[0].slopeX = (edge[0].end.xc - edge[0].start.xc)/(edge[0].end.yc - edge[0].start.yc);
				edge[2].slopeX = (edge[2].end.xc - edge[2].start.xc)/(edge[2].end.yc - edge[2].start.yc);

				if(edge[0].slopeX < edge[2].slopeX) // L triangle
				{
					type = 2;
				}
				else // R triangle
				{
					type = 3;
				}
			}

			// advance edges
			float dY = ceil(vert[0].yc) - vert[0].yc;
			edge[0].slopeX = (edge[0].end.xc - edge[0].start.xc)/(edge[0].end.yc - edge[0].start.yc);
			edge[1].slopeX = (edge[1].end.xc - edge[1].start.xc)/(edge[1].end.yc - edge[1].start.yc);
			edge[2].slopeX = (edge[2].end.xc - edge[2].start.xc)/(edge[2].end.yc - edge[2].start.yc);
			edge[0].slopeZ = (edge[0].end.zc - edge[0].start.zc)/(edge[0].end.yc - edge[0].start.yc);
			edge[1].slopeZ = (edge[1].end.zc - edge[1].start.zc)/(edge[1].end.yc - edge[1].start.yc);
			edge[2].slopeZ = (edge[2].end.zc - edge[2].start.zc)/(edge[2].end.yc - edge[2].start.yc);

			edge[0].curr.xc = edge[0].start.xc + edge[0].slopeX * dY;
			edge[0].curr.yc = edge[0].start.yc + dY;
			edge[0].curr.zc = edge[0].start.zc + edge[0].slopeZ * dY;
	
			edge[2].curr.xc = edge[2].start.xc + edge[2].slopeX * dY;
			edge[2].curr.yc = edge[2].start.yc + dY;
			edge[2].curr.zc = edge[2].start.zc + edge[2].slopeZ * dY;

			// top part
			// span setups and advances
			float dX, sZ, xValue, yValue, zValue;
			GzIntensity r;
			GzIntensity g; 
			GzIntensity b;
			GzIntensity alpha;
			GzDepth fb;
			while (edge[0].curr.yc < edge[0].end.yc)
			{
				if (type == 1 || type == 2) // L or bottom horizontal triangle	
		        {
					dX = ceil(edge[0].curr.xc) - edge[0].curr.xc;
					sZ = (edge[2].curr.zc - edge[0].curr.zc)/(edge[2].curr.xc - edge[0].curr.xc);
					xValue = edge[0].curr.xc + dX;
					yValue = edge[0].curr.yc;
					zValue = edge[0].curr.zc + dX * sZ;
				}
				else if(type == 3) // R triangle
				{
					dX = ceil(edge[2].curr.xc) - edge[2].curr.xc;
					sZ = (edge[0].curr.zc - edge[2].curr.zc)/(edge[0].curr.xc - edge[2].curr.xc);
					xValue = edge[2].curr.xc + dX;
					yValue = edge[2].curr.yc;
					zValue = edge[2].curr.zc + dX * sZ;
				}

				
				//scan one horizontal line
				while (((type == 1 || type == 2) && xValue < edge[2].curr.xc) || (type == 3 && xValue < edge[0].curr.xc))
				{
					int t = (int)xValue;
					int j = (int)edge[0].curr.yc;
					//check the value of z
					if (zValue < 0)
					{
						continue;
					}
			
					GzGetDisplay(render->display, t, j, &r, &g, &b, &alpha, &fb);
					if (fb == 0 || zValue < fb) 
					{
						GzPutDisplay(render->display, t, j, ctoi(render->flatcolor[0]), ctoi(render->flatcolor[1]), ctoi(render->flatcolor[2]), 0, (GzDepth)zValue);
					}
			
					xValue++;
					zValue = zValue + sZ;
				}
				edge[0].curr.xc = edge[0].curr.xc + edge[0].slopeX;
				edge[0].curr.yc++;
				edge[0].curr.zc = edge[0].curr.zc + edge[0].slopeZ;
				edge[2].curr.xc = edge[2].curr.xc + edge[2].slopeX;
				edge[2].curr.yc++;
				edge[2].curr.zc = edge[2].curr.zc + edge[2].slopeZ;
			}

			// bottom part
			dY = ceil(vert[1].yc) - vert[1].yc;
			edge[1].curr.xc = edge[1].start.xc + edge[1].slopeX * dY;
			edge[1].curr.yc = edge[1].start.yc + dY;
			edge[1].curr.zc = edge[1].start.zc + edge[1].slopeZ * dY;

			while (edge[1].curr.yc < edge[1].end.yc)
			{
				if (type == 2) //L triangle	
		        {
					dX = ceil(edge[1].curr.xc) - edge[1].curr.xc;
					sZ = (edge[2].curr.zc - edge[1].curr.zc)/(edge[2].curr.xc - edge[1].curr.xc);
					xValue = edge[1].curr.xc + dX;
					yValue = edge[1].curr.yc;
					zValue = edge[1].curr.zc + dX * sZ;
				}
				else if(type == 0 || type == 3) // R triangle top horizontal
				{
					dX = ceil(edge[2].curr.xc) - edge[2].curr.xc;
					sZ = (edge[1].curr.zc - edge[2].curr.zc)/(edge[1].curr.xc - edge[2].curr.xc);
					xValue = edge[2].curr.xc + dX;
					yValue = edge[2].curr.yc;
					zValue = edge[2].curr.zc + dX * sZ;
				}
				while ((type == 2 && xValue < edge[2].curr.xc) || ((type == 3 || type == 0) && xValue < edge[1].curr.xc))
				{
					int t = (int)xValue;
					int j = (int)edge[1].curr.yc;
					if (zValue < 0) 
					{
						continue;
					}
			
					GzGetDisplay(render->display, t, j, &r, &g, &b, &alpha, &fb);
					//test value of z
					if (fb == 0 || zValue < fb) 
					{
						GzPutDisplay(render->display, t, j, ctoi(render->flatcolor[0]), ctoi(render->flatcolor[1]), ctoi(render->flatcolor[2]), 0, (GzDepth)zValue);
					}
			
					xValue++;
					zValue = zValue + sZ;
				}
				
				edge[1].curr.xc = edge[1].curr.xc + edge[1].slopeX;
				edge[1].curr.yc++;
				edge[1].curr.zc = edge[1].curr.zc + edge[1].slopeZ;
				edge[2].curr.xc = edge[2].curr.xc + edge[2].slopeX;
				edge[2].curr.yc++;
				edge[2].curr.zc = edge[2].curr.zc + edge[2].slopeZ;
			}

		}
		else if (nameList[i] == GZ_NULL_TOKEN)
		{
			//do nothing
		}
	}
	return GZ_SUCCESS;
}


short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}
