//cs371 Fall 2013
//program: haze.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
//haze, fog, media in air and other material
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "vector.h"
#include "log.h"
#include <math.h>

#define XHUGE (1.0e8)
#define epsilon (0.0000001)

extern Global g;

extern void mediaGetLightContrib(Vec p, Vec rgb);
void hazePolySetup(Haze *haze, int i);
int hazeIntersect(Haze *haze, Ray *ray, Flt *dist, int pnum);
void hazeBox(Haze *haze, Ray *ray, Flt t, Vec col);

void testForHaze(Ray *ray, Flt t, Vec col)
{
	Haze *haze = g.hazeHead;
	while (haze) {
		hazeBox(haze, ray, t, col);
		haze = haze->next;
	}
}

void checkColorDiff_r(Vec prergb, Vec rgb, Vec start, Vec end, Vec col, Flt wt)
{
	//Is there a big difference in color?
	Vec vdif;
	VecSub(rgb, prergb, vdif);
	Flt cdif = VecLen(vdif);
	if (cdif < 0.004 || wt < 0.001) {
		col[0] += rgb[0] * wt;
		col[1] += rgb[1] * wt;
		col[2] += rgb[2] * wt;
		return;
	}
	//printf("cdif: %lf ", cdif); fflush(stdout);
	//get midpoint color and average with endpoints.
	//   start------->mid------->end
	//          c1          c2
	//
	Vec midpt, midrgb;
	VecAdd(start, end, midpt);
	VecS(0.5, midpt, midpt);
	mediaGetLightContrib(midpt, midrgb);
	checkColorDiff_r(prergb, midrgb, start, midpt, col, wt*0.5);
	checkColorDiff_r(midrgb, rgb,    midpt, end,   col, wt*0.5);
}

void hazeBox(Haze *haze, Ray *ray, Flt t, Vec col)
{
	int i, found;
	Flt dist;
	Flt fhaze, hlen;
	Ray tray;
	Flt z1,z2,top,bottom,midspot,pct;

	//Log("haze_box()...\n");
	//Does the ray intersect the boundry of the haze? Where?
	haze->distMin = XHUGE;
	haze->distMax = -XHUGE;
	VecCopy(ray->o, tray.o);
	VecCopy(ray->d, tray.d);
	found=0;
	for (i=0; i<6; i++) {
		if (hazeIntersect(haze, &tray, &dist, i)) {
			if (dist < haze->distMin) haze->distMin = dist;
			if (dist > haze->distMax) haze->distMax = dist;
			if (++found == 2)
				break;
		}
	}
	//found could be 1 if ray starts inside the haze box.
	//Are we inside the haze bounding box?
	if (haze->distMax == haze->distMin) {
		//Log("ray starts inside the haze bb\n");
		haze->distMin = 0.0;
	}
	//Does this ray hit an object?
	if (haze->distMax > t)
		haze->distMax = t;
	//Ray intersected the haze bounding box.
	hlen = haze->distMax - haze->distMin;
	Flt step = haze->mediaStep;
	Vec mrgbt={0,0,0};
	if (hlen <= 0.0)
		return;
	Flt de = haze->mediaDensity * haze->mediaStep;
	//if (hlen > 0.0) {
	if (haze->media) {
		//All media color comes from lights.
		//Step across the ray length checking light sources.
		Vec mstart;
		RayPoint(ray, haze->distMin, mstart);
		Vec mstep;
		mstep[0] = ray->d[0] * step;
		mstep[1] = ray->d[1] * step;
		mstep[2] = ray->d[2] * step;
		Vec rgb, prergb;
		//how many steps?
		int nsteps = hlen / step;
		if (haze->mediaAdaptive) {
			Vec pmstart;
			for (int i=0; i<nsteps; i++) {
				mediaGetLightContrib(mstart, rgb);
				if (i > 0) {
					//Is there a big difference in color?
					Vec col={0,0,0};
					checkColorDiff_r(prergb, rgb, pmstart, mstart, col, 1.0);
					VecCopy(col, rgb);
				}
				VecCopy(mstart, pmstart);  //save previous point
				VecCopy(rgb, prergb);      //save previous color
				VecAdd(rgb, mrgbt, mrgbt); //accum color
				mstart[0] += mstep[0];
				mstart[1] += mstep[1];
				mstart[2] += mstep[2];
			}
			col[0] += mrgbt[0] * de;
			col[1] += mrgbt[1] * de;
			col[2] += mrgbt[2] * de;
		} else {
			for (int i=0; i<nsteps; i++) {
			//	if (mediaGetLightContrib(mstart, rgb)) {
			//		col[0] += rgb[0] * de;
			//		col[1] += rgb[1] * de;
			//		col[2] += rgb[2] * de;
			//		//VecAdd(rgb, mrgbt, mrgbt);
			//	}
				mediaGetLightContrib(mstart, rgb);
				col[0] += rgb[0] * de;
				col[1] += rgb[1] * de;
				col[2] += rgb[2] * de;
				mstart[0] += mstep[0];				
				mstart[1] += mstep[1];				
				mstart[2] += mstep[2];				
			}
		}
		//Flt dense = haze->media_density;
		//VecComb(dense, rgbt, 1.0-dense, col, col);
	}
	if (haze->fog) {
		z1 = haze->distMin * ray->d[1] + ray->o[1];
		z2 = haze->distMax * ray->d[1] + ray->o[1];
		bottom = haze->boundryMin[1];
		top = haze->boundryMax[1];
		midspot = z2 + (fabs(z1 - z2) / 2.0) - bottom;
		//Spot is at what portion of total haze height.
		pct = 1.0 - (midspot / (top - bottom));
		if (haze->linear != 1.0)
			hlen *= (pow(pct, haze->linear));
		else
			hlen *= pct;
	}
	//not here.
	//Flt de = haze->media_density * haze->media_step;
	//if (haze->media) {
	//	//media is affected by fog value also.
	//	fhaze = 1.0 - pow(1.0 - de, hlen);
	//	VecComb(fhaze, mrgbt, 1.0-fhaze, col, col);
	//}
	if (haze->density > 0.0) {
		fhaze = 1.0 - pow(1.0 - haze->density, hlen);
		VecComb(fhaze, haze->color, 1.0-fhaze, col, col);
	}
}

int hazeIntersect(Haze *haze, Ray *ray, Flt *dist, int pnum)
{
	int i, j, l;
	int qi, qj, ri, rj, c1, c2;
	Flt n, d, t,m,b;
	bobPoint V;

	//Log("haze_intersect()...\n");
	n = VecDot(ray->o, haze->polyNormal[pnum]) + haze->polyD[pnum];
	d = VecDot(ray->d, haze->polyNormal[pnum]);
	//check for ray in plane of polygon
	if (ABS(d) < epsilon)
		return 0;
	t = -n/d;
	if (t < epsilon)
		return 0;
	RayPoint(ray,t,V);
	c1 = (int)haze->polyP1[pnum];
	c2 = (int)haze->polyP2[pnum];
	l = 0;
	for (i = 0; i < 4; i++) {
		//j = (i + 1) % 4; //mod by 4
		j = (i + 1) & 0x03; //also mod by 4
		if (haze->polyPoint[pnum][i][c2] == haze->polyPoint[pnum][j][c2])
			continue;  //ignore horizontal lines
		qi = qj = 0;
		if (haze->polyPoint[pnum][i][c2] < V[c2]) qi = 1;
		if (haze->polyPoint[pnum][j][c2] < V[c2]) qj = 1;
		if (qi == qj)
			continue;
		ri = rj = 0;
		if (haze->polyPoint[pnum][i][c1] < V[c1]) ri = 1;
		if (haze->polyPoint[pnum][j][c1] < V[c1]) rj = 1;
		if (ri & rj) {
			l++;
			continue;
		}
		if ((ri|rj) == 0)
			continue;
		// more difficult acceptance...
		m = (haze->polyPoint[pnum][j][c2] - haze->polyPoint[pnum][i][c2]) /
		    (haze->polyPoint[pnum][j][c1] - haze->polyPoint[pnum][i][c1]);
		b = (haze->polyPoint[pnum][j][c2] - V[c2]) -
		    m * (haze->polyPoint[pnum][j][c1] - V[c1]);
		if ((-b/m) < epsilon)
			l++;
	}
	//if ((l % 2) == 0) return 0;
	if ((l & 1) == 0)
		return 0;
	*dist = t;
	//Log("hit at %lf\n",*dist);
	return 1;
}

void setup_haze(void)
{
	int i;
	Haze *haze = g.hazeHead;
	//There could be multiple haze boxes or areas.
	while (haze) {
		if (haze->density < 0.0) {
			haze = haze->next;
			continue;
		}
		if (haze->type == HAZE_TYPE_BORDERS) {
			if (haze->reach[0] <= 0.0 ||
				haze->reach[1] <= 0.0 ||
				haze->reach[2] <= 0.0) {
				//no volume to this haze area.
				haze = haze->next;
				continue;
			}
			for (i=0; i<3; i++) {
				haze->boundryMin[i] = haze->center[i] - haze->reach[i];
				haze->boundryMax[i] = haze->center[i] + haze->reach[i];
			}
			//
			//corners of bounding box
			haze->p[0][0] = haze->boundryMin[0];
			haze->p[0][1] = haze->boundryMin[1];
			haze->p[0][2] = haze->boundryMax[2];
			haze->p[1][0] = haze->boundryMin[0];
			haze->p[1][1] = haze->boundryMax[1];
			haze->p[1][2] = haze->boundryMax[2];
			haze->p[2][0] = haze->boundryMax[0];
			haze->p[2][1] = haze->boundryMax[1];
			haze->p[2][2] = haze->boundryMax[2];
			haze->p[3][0] = haze->boundryMax[0];
			haze->p[3][1] = haze->boundryMin[1];
			haze->p[3][2] = haze->boundryMax[2];
			//
			haze->p[4][0] = haze->boundryMin[0];
			haze->p[4][1] = haze->boundryMin[1];
			haze->p[4][2] = haze->boundryMin[2];
			haze->p[5][0] = haze->boundryMin[0];
			haze->p[5][1] = haze->boundryMax[1];
			haze->p[5][2] = haze->boundryMin[2];
			haze->p[6][0] = haze->boundryMax[0];
			haze->p[6][1] = haze->boundryMax[1];
			haze->p[6][2] = haze->boundryMin[2];
			haze->p[7][0] = haze->boundryMax[0];
			haze->p[7][1] = haze->boundryMin[1];
			haze->p[7][2] = haze->boundryMin[2];
			//sides of bounding box
			for (i=0; i<3; i++) {
				haze->polyPoint[0][0][i] = haze->p[0][i];
				haze->polyPoint[0][1][i] = haze->p[3][i];
				haze->polyPoint[0][2][i] = haze->p[7][i];
				haze->polyPoint[0][3][i] = haze->p[4][i];
				//
				haze->polyPoint[1][0][i] = haze->p[2][i];
				haze->polyPoint[1][1][i] = haze->p[1][i];
				haze->polyPoint[1][2][i] = haze->p[5][i];
				haze->polyPoint[1][3][i] = haze->p[6][i];
				//
				haze->polyPoint[2][0][i] = haze->p[1][i];
				haze->polyPoint[2][1][i] = haze->p[0][i];
				haze->polyPoint[2][2][i] = haze->p[4][i];
				haze->polyPoint[2][3][i] = haze->p[5][i];
				//
				haze->polyPoint[3][0][i] = haze->p[3][i];
				haze->polyPoint[3][1][i] = haze->p[2][i];
				haze->polyPoint[3][2][i] = haze->p[6][i];
				haze->polyPoint[3][3][i] = haze->p[7][i];
				//
				haze->polyPoint[4][0][i] = haze->p[1][i];
				haze->polyPoint[4][1][i] = haze->p[2][i];
				haze->polyPoint[4][2][i] = haze->p[3][i];
				haze->polyPoint[4][3][i] = haze->p[0][i];
				//
				haze->polyPoint[5][0][i] = haze->p[4][i];
				haze->polyPoint[5][1][i] = haze->p[7][i];
				haze->polyPoint[5][2][i] = haze->p[6][i];
				haze->polyPoint[5][3][i] = haze->p[5][i];
			}
			for (i=0; i<6; i++) {
				hazePolySetup(haze, i);
			}
		}
		haze = haze->next;
	}
}

void hazePolySetup(Haze *haze, int i) {
	Vec P1,P2;
	//
	VecSub(haze->polyPoint[i][0], haze->polyPoint[i][1], P1);
	VecSub(haze->polyPoint[i][2], haze->polyPoint[i][1], P2);
	VecCross(P1, P2, haze->polyNormal[i]);
	VecNormalize(haze->polyNormal[i]);
	//
	if (ABS(haze->polyNormal[i][0]) >= ABS(haze->polyNormal[i][1]) &&
	    ABS(haze->polyNormal[i][0]) >= ABS(haze->polyNormal[i][2])) {
	    haze->polyP1[i] = 1;
	    haze->polyP2[i] = 2;
	} else if (ABS(haze->polyNormal[i][1]) >= ABS(haze->polyNormal[i][0]) &&
	    ABS(haze->polyNormal[i][1]) >= ABS(haze->polyNormal[i][2])) {
	    haze->polyP1[i] = 0;
	    haze->polyP2[i] = 2;
	} else {
	    haze->polyP1[i] = 0;
	    haze->polyP2[i] = 1;
	}
	haze->polyD[i] = - VecDot(haze->polyNormal[i], haze->polyPoint[i][0]);
}


