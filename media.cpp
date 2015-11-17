//cs371 Fall 2013
//program: media.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
#include "defs.h"
#include "vector.h"
#include "extern.h"

Media media;
void media_adaptive(Vec P, Color acol, Color colx1, Color colx2, unsigned int level);
void setup_media(void);
void media_poly_setup(int i);


void test_for_media_between_points(Vec xP, Vec I, Flt t, Color col, int hitYN)
{
	int i, found, firsttime=1, nsteps;
	Flt num[NSLABS], den[NSLABS];
	Flt length, dist, fx;
	Vec P;
	Flt tmin, tmax, dmin = -XHUGE, dmax = XHUGE;
	Color tcol,acol,save_col;
	Ray tray;
	int hit_within_border_flag;

	//Log( "test_for_media_between_points(t=%lf   xP: %lf %lf %lf)...\n",t,xP[0],xP[1],xP[2]);
	if (hitYN) {
		//Log( "this eye ray hit something\n");
		//P[0] = -t * I[0] + xP[0];
		//P[1] = -t * I[1] + xP[1];
		//P[2] = -t * I[2] + xP[2];
		VecAddS((-t),I,xP,P);
	}
	else {
		//Log( "this eye ray hit nothing\n");
		VecCopy(xP, P);
		//VecCopy(bob.Eye.view_from, P);
	}
	//Log( "P: %lf %lf %lf\n",P[0],P[1],P[2]);
	found=1;
	VecCopy(P, num);
	VecCopy(I, den);
	for(i=0; i<NSLABS; i++) {
		if(den[i] == 0.0) continue;
		tmin = (media.boundry_min[i] - num[i]) / den[i];
		tmax = (media.boundry_max[i] - num[i]) / den[i];
		if(tmin < tmax) {
			if(tmax < dmax) dmax = tmax;
			if(tmin > dmin) dmin = tmin;
		}
		else {
			if(tmin < dmax) dmax = tmin;
			if(tmax > dmin) dmin = tmax;
		}
		if(dmin>dmax || dmax<bob.rayeps) {
			found=0;
			break;
		}
	}
	//
	if (found) {
		//Log( "slab hit!\n");
		found=0;
		media.dist_min = XHUGE;
		media.dist_max = -XHUGE;
		VecCopy(P, tray.P);
		VecCopy(I, tray.D);
		for (i=0; i<6; i++) {
			if (media_bounding_box_intersect(&tray, &dist, i)) {
				found++;
				//rRayPoint(tray, dist, V);
				//Log( "hit bounding box #%i   at: %lf %lf %lf\n",i,V[0],V[1],V[2]);
				if (dist < media.dist_min) media.dist_min = dist;
				if (dist > media.dist_max) media.dist_max = dist;
			}
			if (found > 1) break;
		}
		//Log( "media.dist_min: %lf %lf\n",media.dist_min,media.dist_max);
		//are we inside the media bounding box?
		if (media.dist_max == media.dist_min) media.dist_min = 0.0;
		//
		//Log( "does the eye ray hit an object?\n");
		media.hit_within_border = 0;
		hit_within_border_flag = 0;
		if (media.dist_max > t) {
			media.dist_max = t;
			hit_within_border_flag = 1;
			//Log( "hit_within_border.\n");
		}
		if (media.dist_min > t) found = 0;
	}
	//
	if (found) {
		rRayPoint(tray, media.dist_min, P);
		//Log( "box hit at P: %lf %lf %lf\n",P[0],P[1],P[2]);
		length = media.dist_max - media.dist_min;
		nsteps = (int)(length / media.step) + 1;
		//
		media.media_inc[0] = (length * tray.D[0]) / (Flt)nsteps;
		media.media_inc[1] = (length * tray.D[1]) / (Flt)nsteps;
		media.media_inc[2] = (length * tray.D[2]) / (Flt)nsteps;
		//
		VecZero(acol);
		VecZero(tcol);
		for (i=0; i<nsteps; i++) {
			//are we near the end of the ray?
			if (hit_within_border_flag && (i >= (nsteps-3))) media.hit_within_border = 1;
			//
			//call function that looks toward light.
			VecCopy(tcol,save_col);
			VecZero(tcol);
			media_Shade(P,tcol);
			VecAdd(tcol, acol, acol);
			if (!firsttime) {
				if (ABS(tcol[0] - save_col[0]) > media.variance || ABS(tcol[1] - save_col[1]) > media.variance || ABS(tcol[2] - save_col[2]) > media.variance ) {
					//the color changed.
					media_adaptive(P, acol, save_col, tcol, 1);
				}
			}
			firsttime=0;
			VecAdd(P, media.media_inc, P);
		}
		fx = media.density * media.density;
		VecAddS(fx, acol, col, col);
	}
}
//
//*******************************************
//********************************************
//*********************************************
//**********************************************
//***********************************************
//************************************************
//*************************************************
//**************************************************
//---------------------------------------*---------------------------------------*        original sampling interval
//---------------------------------------*-------------------*-------------------*        sub sampling level 1
//-------------------------------------------------*---------*                            sub sampling level 2
//-------------------------------------------------*----*                                 sub sampling level 3
//----------------------------------------------------*-*                                 sub sampling level 4
//********************************************************
//*********************************************************
//**********************************************************
//***********************************************************
//************************************************************
//*************************************************************
//**************************************************************
//
//
//
//
//
void media_adaptive(Vec P, Color acol, Color colx1, Color colx2, unsigned int level)
{
	Color mid_col, col1, col2;
	Vec V, mdist;
	Flt div,div2;

	//Log("media_adaptive()...\n");
	//the last 2 points tested were different.
	//scan the point between them.
	if (level > media.maxlevel) return;
	//
	VecCopy(colx1, col1);
	VecCopy(colx2, col2);
	//
	div = (Flt)(0x00000001 << level);
	div2 = 1.0 / div;
	//since we are going to sample an extra point, reduce the power of this point.
	VecS(0.5, col2, col2);
	VecSub(acol, col2, acol);
	//Log("level: %i   div: %lf   div2: %lf\n",level,div,div2);
	//mdist[0] = media.media_inc[0] * div2;
	//mdist[1] = media.media_inc[1] * div2;
	//mdist[2] = media.media_inc[2] * div2;
	VecS(div2, media.media_inc, mdist);
	//Log("P: %lf %lf %lf\n",P[0],P[1],P[2]);
	//Log("mdist: %lf %lf %lf\n",mdist[0],mdist[1],mdist[2]);
	VecSub(P, mdist, V);
	//Log("mdist: %lf %lf %lf\n",mdist[0],mdist[1],mdist[2]);
	//P is now the middle point.
	//
	//scan the middle point.
	//
	VecZero(mid_col);
	media_Shade(V, mid_col);
	VecS(div2, mid_col, mid_col);
	VecAdd(mid_col, acol, acol);
	//
	//test between points 1 and 2.
	//
	//return;
	if (ABS(col1[0] - mid_col[0]) > media.variance || ABS(col1[1] - mid_col[1]) > media.variance || ABS(col1[2] - mid_col[2]) > media.variance ) {
		media_adaptive(V, acol, col1, mid_col, level+1);
	}
	//
	if (ABS(col2[0] - mid_col[0]) > media.variance || ABS(col2[1] - mid_col[1]) > media.variance || ABS(col2[2] - mid_col[2]) > media.variance ) {
		VecCopy(P,V);
		media_adaptive(V, acol, mid_col, col2, level+1);
	}
}


/*
void media_adaptive(Vec P, Color acol, Color colx1, Color colx2, int level)
{
	Color mid_col, col1, col2;
	Vec V, mdist;
	Flt div,div2;

	//the last 2 points tested were different.
	//scan the point between them.
	if (level > media.maxlevel) return;
	//
	VecCopy(colx1, col1);
	VecCopy(colx2, col2);
	//
	div = (Flt)(0x00000001 << level);
	div2 = 1.0 / div;
	//since we are going to sample an extra point,
	//reduce the power of this point.
	VecS(0.5, col2, col2);
	VecSub(acol, col2, acol);

	//Log("level: %i   div: %lf   div2: %lf\n",level,div,div2);
	//div2 = 1.0 / 2.0;
	mdist[0] = media.media_inc[0] * div2;
	mdist[1] = media.media_inc[1] * div2;
	mdist[2] = media.media_inc[2] * div2;
	VecSub(P, mdist, V);
	//P is now the middle point.
	//
	//scan the middle point.
	//
	VecZero(mid_col);
	media_Shade(V, mid_col);
	VecS(div2, mid_col, mid_col);
	VecAdd(mid_col, acol, acol);
	//
	//test between points 1 and 2.
	//
	if (ABS(col1[0] - mid_col[0]) > media.variance || ABS(col1[1] - mid_col[1]) > media.variance || ABS(col1[2] - mid_col[2]) > media.variance ) {
		media_adaptive(V, acol, col1, mid_col, level+1);
	}
	//
	if (ABS(col2[0] - mid_col[0]) > media.variance || ABS(col2[1] - mid_col[1]) > media.variance || ABS(col2[2] - mid_col[2]) > media.variance ) {
		VecCopy(P,V);
		media_adaptive(V, acol, mid_col, col2, level+1);
	}
}
*/



int media_bounding_box_intersect(Ray *ray, Flt *dist, int pnum)
{
	int i, j, l;
	int qi, qj, ri, rj, c1, c2;
	Flt n, d, t,m,b;
	bobPoint V;

	//Log("bounding_box_intersect()...\n");
	n = VecDot(ray->P, media.poly_normal[pnum]) + media.poly_d[pnum];
	d = VecDot(ray->D, media.poly_normal[pnum]);
	//check for ray in plane of polygon
	if(ABS(d) < bob.rayeps) { return 0; }
	t = -n/d;
	if(t < bob.rayeps) { return 0; }
	RayPoint(ray,t,V);
	c1 = (int)media.poly_p1[pnum];
	c2 = (int)media.poly_p2[pnum];
	l = 0;
	for (i = 0; i < 4; i++) {
		j = (i + 1) % 4;
		qi = qj = ri = rj = 0;
		if (media.poly_point[pnum][i][c2] == media.poly_point[pnum][j][c2]) continue;  //ignore horizontal lines
		if (media.poly_point[pnum][i][c2] < V[c2]) qi = 1;
		if (media.poly_point[pnum][j][c2] < V[c2]) qj = 1;
		if (qi == qj) continue;
		if (media.poly_point[pnum][i][c1] < V[c1]) ri = 1;
		if (media.poly_point[pnum][j][c1] < V[c1]) rj = 1;
		if (ri & rj) { l++; continue; }
      if ((ri|rj) == 0) continue;
		// more difficult acceptance...
		m = (media.poly_point[pnum][j][c2] - media.poly_point[pnum][i][c2]) / (media.poly_point[pnum][j][c1] - media.poly_point[pnum][i][c1]);
		b = (media.poly_point[pnum][j][c2] - V[c2]) - m * (media.poly_point[pnum][j][c1] - V[c1]);
		if ((-b/m) < bob.rayeps) l++;
	}
	if ((l % 2) == 0) return 0;
	//if ((l & 1) == 0) return 0;
	*dist = t;
	//Log("hit at %lf\n",*dist);
	return 1;
}


void media_Shade(Vec P, Color tcol)
{
	Vec L;
	Vec tmpV;         //direction to light vector
	Color c_diff;     //diffuse color
	Color c_dist;     //light color scaled by distance
	Color c_shadow;   //shadow color
	Flt light_spot;   //spot light brightness
	Light *cur_light; //ptr to current light we're looking at
	Isect nhit;
	Ray tray;
	Flt t;
	Vec N = {0.0,0.0,1.0};
	Vec tex_P;

	//Log( "media_Shade(%lf, %lf, %lf   color:%lf, %lf, %lf)...\n",P[0],P[1],P[2],tcol[0],tcol[1],tcol[2]);
	//
	//only if entering or cacheable
	cur_light = bob.light_head;
	for(cur_light = bob.light_head; cur_light; cur_light = cur_light->next) {
		if (cur_light->flag & L_MEDIA) {
			switch(cur_light->type) {
				case L_POINT:     VecSub(cur_light->position, P, L); break;
				case L_SPHERICAL: VecSub(cur_light->position, P, L); break;
				case L_SPOT:
					VecSub(cur_light->position, P, L);
					VecCopy(L, tmpV);
					VecNormalize(tmpV);
					light_spot = VecDot(cur_light->dir, tmpV);
					//if not in light
					if(light_spot < cur_light->max_angle) { continue; }
					//total illumination
					if(light_spot >= cur_light->min_angle) {
						light_spot = 1.0;
					}
					else {
						light_spot = (light_spot - cur_light->max_angle) / (cur_light->min_angle - cur_light->max_angle);
					}
					break;
			}
			//
			t = VecNormalizeL(L);
			VecCopy(P, tray.P);
			VecCopy(L, tray.D);
			MakeVector(1.0, 1.0, 1.0, c_shadow);
			//
			if(!bob.shadows || (cur_light->flag & L_NOSHADOWS) || (cur_light->type == L_SPHERICAL ?
				media_sShadow(&tray, &nhit, t, c_shadow, 0, cur_light, 1) :
				//media_Shadow(&tray, &nhit, t, c_shadow, 0, cur_light, 1))) {
				media_Shadow(&tray, &nhit, t, c_shadow, 0, cur_light))) {
				//there is some light!!!   scale light color by distance.
				//Log( "there is some light!\n");
				MakeVector(1.0, 1.0, 1.0, c_diff);
				if(media.surf.tex) {
					tex_fix(&media.surf, P, tex_P, N);
					VecCopy(media.surf.diff, c_diff);
				}
				switch(cur_light->illum) {
					case L_INFINITE:  VecCopy(cur_light->color, c_dist); break;
					case L_R:         VecS(1.0/t, cur_light->color, c_dist); break;
					case L_R_SQUARED: VecS(1.0/(t*t), cur_light->color, c_dist); break;
					//case 4:           VecS(1.0/(t*t*t*t), cur_light->color, c_dist); break;
				}
				//VecMul(c_dist, c_diff, c_diff);
				//if(cur_light->type == L_SPOT) { VecS(light_spot, c_diff, c_diff); }
				//VecMul(c_shadow, c_diff, c_diff);
				//VecAdd(c_diff, tcol, tcol);
				if(cur_light->type == L_SPOT) {
					tcol[0] += ( c_diff[0] * c_dist[0] * light_spot * c_shadow[0] );
					tcol[1] += ( c_diff[1] * c_dist[1] * light_spot * c_shadow[1] );
					tcol[2] += ( c_diff[2] * c_dist[2] * light_spot * c_shadow[2] );
				}
				else {
					tcol[0] += ( c_diff[0] * c_dist[0] * c_shadow[0] );
					tcol[1] += ( c_diff[1] * c_dist[1] * c_shadow[1] );
					tcol[2] += ( c_diff[2] * c_dist[2] * c_shadow[2] );
				}
			}
			//else
			//{
			//	Log( "blocked\n");
			//}
		}
	}
}




//	Returns 0 if totally shadowed
//	Returns 1 if partially or fully unshadowed
//
//Ray	*ray;
//Isect	*hit;
//Flt	tmax;			//dist to light we are trying to hit
//Color	mcolor;		//for colored shadows
//int	level;		//current tree level
//Light   *cur_light;     //light we are checking for shadow
int media_Shadow(Ray *ray, Isect *hit, Flt tmax, Color mcolor, unsigned int level, Light *cur_light)
{
	#ifdef SHADOW_CACHING
		Object *cached;
	#endif SHADOW_CACHING
	Surface         *prev_surf;     //used if light is inside object
	static Surface  tmp_surf;       //to allow textured shadows
	static int      i, in_surf;
	//Flt             t, caustic_scale;
	//Vec             P;              //point for texture call
	//Vec N,T;
	//Object  *prim;
	//Flt dot;


	//Log( "media_Shadow()...\n");
	//
	prev_surf = NULL;
	//check cache first
	#ifdef SHADOW_CACHING
		if(cached = cur_light->light_obj_cache[level]) {
			if((cached->o_procs->intersect)(cached, ray, hit) && hit->isect_t < tmax) {
				//Log( "cached hit!\n");
				return 0;
			}
		}
		cur_light->light_obj_cache[level] = NULL;
	#endif SHADOW_CACHING
	//Log( "tmax: %lf\n",tmax);
	//if (Intersect(ray, hit, tmax, hit->isect_self))
	//Log( "shooting ray from %lf %lf %lf   at %lf %lf %lf  with len: %lf\n",ray->P[0],ray->P[1],ray->P[2],ray->D[0],ray->D[1],ray->D[2],tmax);
	if (Intersect(ray, hit, tmax, NULL)) {
		//we hit an object.
		//Log( "hit\n");
		MakeVector(0.0, 0.0, 0.0, mcolor);
		return 0;
//		t = hit->isect_t;
//		memcpy(&tmp_surf, hit->isect_surf, sizeof(Surface));
//		if (tmp_surf.trans[0] < bob.rayeps && tmp_surf.trans[1] < bob.rayeps && tmp_surf.trans[2] < bob.rayeps)
//		{
//			MakeVector(0.0, 0.0, 0.0, mcolor);
//			if(tmp_surf.flags & S_CACHE)
//			{
//				cur_light->light_obj_cache[level] = hit->isect_prim;
//			}
//			return 0;
//		}
//		//
//		//I guess we hit some kind of transparent object.
//		//
//		if(bob.caustics)
//		{
//			//bobPoint   N;
//			//Object  *prim;
//			RayPoint(ray, t, P);
//			prim = hit->isect_prim;
//			(*prim->o_procs->normal) (prim, hit, P, N);
//			caustic_scale = VecDot(ray->D, N);
//			set_cscale(&caustic_scale);
//			//Log( "caustic_scale: %lf\n",caustic_scale);
//		}
//		else
//		{
//			caustic_scale = 1.0;
//		}
//		//
//		//Log( "mcolor before: %lf %lf %lf\n",mcolor[0],mcolor[1],mcolor[2]);
//		for(i=0; i<3; i++)
//		{
//			if(tmp_surf.trans[i] > bob.rayeps)
//			{
//				if(bob.exp_trans)
//				{
//					//Log( "t: %lf    tmp_surf.trans[%i]: %lf\n",t,i,tmp_surf.trans[i]);
//					mcolor[i] *= caustic_scale * pow(tmp_surf.trans[i], t);
//				}
//				else
//				{
//					//Log( "tmp_surf.trans[%i]: %lf\n",i,tmp_surf.trans[i]);
//					mcolor[i] *= caustic_scale * tmp_surf.trans[i];
//				}
//			}
//			else
//			{
//				mcolor[i] = 0.0;
//			}
//		}
//		//
//		//if (mcolor[0] < 0.0) mcolor[0] = 0.0;
//		//if (mcolor[1] < 0.0) mcolor[1] = 0.0;
//		//if (mcolor[2] < 0.0) mcolor[2] = 0.0;
//		//Log( "mcolor: %lf %lf %lf\n",mcolor[0],mcolor[1],mcolor[2]);
//		return 1;
//		//----------------------------------------
//		//
//		//
//		//Calculate refraction and find next hit.
//		//
//		//
//		//make intersection point new start point and continue
//		//RayPoint(ray, hit->isect_t, ray->P);
//		RayPoint(ray, t, P);
//		prim = hit->isect_prim;
//		(*prim->o_procs->normal) (prim, hit, P, N);
//		//
//		dot = VecDot(N, ray->D);
//		if(dot > 0.0)
//		{
//			VecNegate(N);
//			dot = -dot;
//		}
//		//
//		//tmax -= hit->isect_t;
//		//
//		// Flt eta;    //ratio of old/new iors
//		// Vec I,      //incident vector
//		// N,          //surface normal
//		// T;          //transmitted vector (calculated)
//		// Flt dot;    // -VecDot(I, N)
//		if ( refract(tmp_surf.ior, ray->D, N, T, dot) )
//		{
//			VecCopy(T, ray->D);
//			VecCopy(P, ray->P);
//			if (Intersect(ray, hit, tmax, hit->isect_self))
//			{
//				Log( "another hit.\n");
//				//
//				//
//				//we hit an object.
//				t = hit->isect_t;
//				memcpy(&tmp_surf, hit->isect_surf, sizeof(Surface));
//				if (tmp_surf.trans[0] < bob.rayeps && tmp_surf.trans[1] < bob.rayeps && tmp_surf.trans[2] < bob.rayeps)
//				{
//					//Log( "not trans...\n");
//					MakeVector(0.0, 0.0, 0.0, mcolor);
//					if(tmp_surf.flags & S_CACHE)
//					{
//						cur_light->light_obj_cache[level] = hit->isect_prim;
//					}
//					return 0;
//				}
//				//I guess we hit some kind of transparent object.
//				//
//				//Log( "trans...\n");
//				//
//				if(bob.caustics)
//				{
//					//bobPoint   N;
//					//Object  *prim;
//					RayPoint(ray, t, P);
//					prim = hit->isect_prim;
//					(*prim->o_procs->normal) (prim, hit, P, N);
//					caustic_scale = VecDot(ray->D, N);
//					set_cscale(&caustic_scale);
//					//Log( "caustic_scale: %lf\n",caustic_scale);
//				}
//				else
//				{
//					caustic_scale = 1.0;
//				}
//				//
//				for(i=0; i<3; i++)
//				{
//					if(tmp_surf.trans[i] > bob.rayeps)
//					{
//						if(bob.exp_trans)
//						{
//							//Log( "t: %lf    tmp_surf.trans[%i]: %lf\n",t,i,tmp_surf.trans[i]);
//							mcolor[i] *= caustic_scale * pow(tmp_surf.trans[i], t);
//						}
//						else
//						{
//							//Log( "tmp_surf.trans[%i]: %lf\n",i,tmp_surf.trans[i]);
//							mcolor[i] *= caustic_scale * tmp_surf.trans[i];
//						}
//					}
//					else
//					{
//						mcolor[i] = 0.0;
//					}
//				}
//				//
//				//
//			}
//		}
//		//if (mcolor[0] < 0.0) mcolor[0] = 0.0;
//		//if (mcolor[1] < 0.0) mcolor[1] = 0.0;
//		//if (mcolor[2] < 0.0) mcolor[2] = 0.0;
//		//Log( "mcolor: %lf %lf %lf\n",mcolor[0],mcolor[1],mcolor[2]);
//		return 1;
	}
	//Log( "no hit.\n");
	//MakeVector(1.0, 1.0, 1.0, mcolor);
	return 1;
}





//	sShadow(ray, hit, tmax, color, level, light, radius, inside)
//
//	sShadow() provides a 'shell' around Shadow for dealing with
//	spherical light sources.  sShadow calls Shadow l_samples
//	times.  Each time the ray is tweeked to hit somewhere on the
//	surface of the light being sampled.
//
int media_sShadow(Ray *ray, Isect *hit, Flt tmax, Color color, unsigned int level, Light *cur_light, unsigned int inside)
{
	int sample, visible, show;
	Color tmp_color;
	Ray tmp_ray;
	Vec tweek;
	Flt radius;
	Flt len;		//length to tweek vector
	Vec L;
	Flt light_spot, dd;

	//Log( "media_sShadow()...\n");
	if (cur_light->flag & L_SPOT_RADIUS) {
		//Log( "L_SPOT_RADIUS\n");
		VecSub(cur_light->position, ray->P, L);
		//VecCopy(L, tmpV);
		VecNormalize(L);
		light_spot = VecDot(cur_light->dir, L);
		//if not in light
		if(light_spot < cur_light->max_angle) { return 0; }
		//total illumination?
		if(light_spot >= cur_light->min_angle) {
			light_spot = 1.0;
		}
		else {
			light_spot = (light_spot - cur_light->max_angle) / (cur_light->min_angle - cur_light->max_angle);
		}
	}
	//
	radius = cur_light->radius;
	MakeVector(0.0, 0.0, 0.0, color);
	visible = 0;  //assume totally shadowed to start
	len = radius/tmax;
	//Log( "samples %i\n",cur_light->samples);
	for(sample=0; sample<cur_light->samples; sample++) {
		VecCopy(ray->D, tmp_ray.D);  //use tmp ray
		VecCopy(ray->P, tmp_ray.P);
		//tweek ray direction
		MakeVector(0.5-rnd(), 0.5-rnd(), 0.5-rnd(), tweek);
		VecNormalize(tweek);
		VecAddS(len, tweek, tmp_ray.D, tmp_ray.D);
		VecNormalize(tmp_ray.D);
		//
		show = 1;
		if (cur_light->density < 1.0) show = (rnd() < cur_light->density);
		if (show) {
			MakeVector(1.0, 1.0, 1.0, tmp_color);
			//Log( "shooting ray from %lf %lf %lf  with len: %lf\n",tmp_ray.P[0],tmp_ray.P[1],tmp_ray.P[2],tmax);
			//if (media_Shadow(&tmp_ray, hit, tmax, tmp_color, level, cur_light, inside)) {
			if (media_Shadow(&tmp_ray, hit, tmax, tmp_color, level, cur_light)) {
				//Log( "adding light %lf %lf %lf\n",tmp_color[0],tmp_color[1],tmp_color[2]);
				VecAdd(tmp_color, color, color);
				visible = 1;
			}
		}
	}
	//VecS((1.0/cur_light->samples), color, color);
	if (cur_light->flag & L_SPOT_RADIUS) {
		dd = 1.0 / (Flt)cur_light->samples * light_spot;
	}
	else {
		dd = 1.0 / (Flt)cur_light->samples;
	}
	VecS(dd, color, color);
	//
	return visible;
}





void setup_media(void)
{
	int i;
	Vec width,center;

	//Log( "setup_media()...\n");
	if (!media.media) {
		//Log( "media.media: %i\n",media.media);
		return;
	}
	//
	if (media.reach[0] > 0.0 && media.reach[1] > 0.0 && media.reach[2] > 0.0) {
		for (i=0; i<3; i++) {
			media.boundry_min[i] = media.center[i] - media.reach[i];
			media.boundry_max[i] = media.center[i] + media.reach[i];
		}
	}
	if (media.boundry_min[0] == 0.0 &&
			media.boundry_min[1] == 0.0 &&
			media.boundry_min[2] == 0.0 &&
			media.boundry_max[0] == 0.0 &&
			media.boundry_max[1] == 0.0 &&
			media.boundry_max[2] == 0.0) {
		VecCopy(bob.Root->o_dmin, media.boundry_min);
		VecCopy(bob.Root->o_dmax, media.boundry_max);

		for (i=0; i<3; i++) { width[i] = (media.boundry_max[i] - media.boundry_min[i]) / 2.0; }
		for (i=0; i<3; i++) { center[i] = media.boundry_min[i] + width[i]; }
		for (i=0; i<3; i++) { media.boundry_min[i] = (center[i] - (width[i] * 1.1)); }
		for (i=0; i<3; i++) { media.boundry_max[i] = (center[i] + (width[i] * 1.1)); }

		//VecS(1.1, media.boundry_min, media.boundry_min);
		//VecS(1.1, media.boundry_max, media.boundry_max);
	}
	//Log( "bob.Eye.view_from: %lf %lf %lf\n",bob.Eye.view_from[0],bob.Eye.view_from[1],bob.Eye.view_from[2]);
	//if (bob.Eye.view_from[0] >= media.boundry_min[0]) media.boundry_min[0] = bob.Eye.view_from[0] + 0.1;
	//if (bob.Eye.view_from[0] <= media.boundry_max[0]) media.boundry_max[0] = bob.Eye.view_from[0] - 0.1;
	//if (bob.Eye.view_from[1] >= media.boundry_min[1]) media.boundry_min[1] = bob.Eye.view_from[1] + 0.1;
	//if (bob.Eye.view_from[1] <= media.boundry_max[1]) media.boundry_max[1] = bob.Eye.view_from[1] - 0.1;
	//if (bob.Eye.view_from[2] >= media.boundry_min[2]) media.boundry_min[2] = bob.Eye.view_from[2] + 0.1;
	//if (bob.Eye.view_from[2] <= media.boundry_max[2]) media.boundry_max[2] = bob.Eye.view_from[2] - 0.1;
	//
	//
	//           1                      2
	//             ********************
	//            *^                  ^*
	//           * ^                  ^ *
	//          *  ^                  ^  *
	//         *   ^                  ^   *
	//        *    ^                  ^    *
	//       *     ^                  ^     *
	//    0 ********************************** 3
	//      *      ^                  ^      *
	//      *      ^                  ^      *
	//      *      ^                  ^      *
	//      *      ^                  ^      *
	//      *      ^                  ^      *
	//      *      ^                  ^      *
	//      *      ^                  ^      *
	//      *     5^                  ^6     *
	// +y   *      ^^^^^^^^^^^^^^^^^^^^      *
	//      *     ^                    ^     *
	//      *    ^                      ^    *
	//      *   ^                        ^   *
	//      *  ^                          ^  *
	//      * ^                            ^ *
	//      *^                              ^*
	//    4 ********************************** 7
	// -y
	// -x                                       +x
	//
	media.p[0][0] = media.boundry_min[0];
	media.p[0][1] = media.boundry_min[1];
	media.p[0][2] = media.boundry_max[2];

	media.p[1][0] = media.boundry_min[0];
	media.p[1][1] = media.boundry_max[1];
	media.p[1][2] = media.boundry_max[2];

	media.p[2][0] = media.boundry_max[0];
	media.p[2][1] = media.boundry_max[1];
	media.p[2][2] = media.boundry_max[2];

	media.p[3][0] = media.boundry_max[0];
	media.p[3][1] = media.boundry_min[1];
	media.p[3][2] = media.boundry_max[2];
	//
	media.p[4][0] = media.boundry_min[0];
	media.p[4][1] = media.boundry_min[1];
	media.p[4][2] = media.boundry_min[2];

	media.p[5][0] = media.boundry_min[0];
	media.p[5][1] = media.boundry_max[1];
	media.p[5][2] = media.boundry_min[2];

	media.p[6][0] = media.boundry_max[0];
	media.p[6][1] = media.boundry_max[1];
	media.p[6][2] = media.boundry_min[2];

	media.p[7][0] = media.boundry_max[0];
	media.p[7][1] = media.boundry_min[1];
	media.p[7][2] = media.boundry_min[2];
	//
	VecCopy(media.p[1], media.poly_point[0][0]);
	VecCopy(media.p[0], media.poly_point[0][1]);
	VecCopy(media.p[4], media.poly_point[0][2]);
	VecCopy(media.p[5], media.poly_point[0][3]);
	VecCopy(media.p[3], media.poly_point[1][0]);
	VecCopy(media.p[2], media.poly_point[1][1]);
	VecCopy(media.p[6], media.poly_point[1][2]);
	VecCopy(media.p[7], media.poly_point[1][3]);
	VecCopy(media.p[0], media.poly_point[2][0]);
	VecCopy(media.p[3], media.poly_point[2][1]);
	VecCopy(media.p[7], media.poly_point[2][2]);
	VecCopy(media.p[4], media.poly_point[2][3]);
	VecCopy(media.p[2], media.poly_point[3][0]);
	VecCopy(media.p[1], media.poly_point[3][1]);
	VecCopy(media.p[5], media.poly_point[3][2]);
	VecCopy(media.p[6], media.poly_point[3][3]);
	VecCopy(media.p[1], media.poly_point[4][0]);
	VecCopy(media.p[2], media.poly_point[4][1]);
	VecCopy(media.p[3], media.poly_point[4][2]);
	VecCopy(media.p[0], media.poly_point[4][3]);
	VecCopy(media.p[4], media.poly_point[5][0]);
	VecCopy(media.p[7], media.poly_point[5][1]);
	VecCopy(media.p[6], media.poly_point[5][2]);
	VecCopy(media.p[5], media.poly_point[5][3]);
	//
	////front
	//media.poly_normal[0][0] =  0.0;
	//media.poly_normal[0][1] = -1.0;
	//media.poly_normal[0][2] =  0.0;
	////back
	//media.poly_normal[1][0] =  0.0;
	//media.poly_normal[1][1] =  1.0;
	//media.poly_normal[1][2] =  0.0;
	////left
	//media.poly_normal[2][0] = -1.0;
	//media.poly_normal[2][1] =  0.0;
	//media.poly_normal[2][2] =  0.0;
	////right
	//media.poly_normal[3][0] =  1.0;
	//media.poly_normal[3][1] =  0.0;
	//media.poly_normal[3][2] =  0.0;
	////top
	//media.poly_normal[4][0] =  0.0;
	//media.poly_normal[4][1] =  0.0;
	//media.poly_normal[4][2] =  1.0;
	////bottom
	//media.poly_normal[5][0] =  0.0;
	//media.poly_normal[5][1] =  0.0;
	//media.poly_normal[5][2] = -1.0;
	//
	for (i=0; i<6; i++) {
		media_poly_setup(i);
		//Log( "media.poly_normal[%i]  %lf %lf %lf     media.poly_d[%i]  %lf\n",i,media.poly_normal[i][0],media.poly_normal[i][1],media.poly_normal[i][2],i,media.poly_d[i]);
	}
}


void media_poly_setup(int i)
{
	Vec P1,P2;

	//pd->poly_npoints = (short)npoints;
	//VecSub(pd->poly_point[0], pd->poly_point[1], P1);
	//VecSub(pd->poly_point[2], pd->poly_point[1], P2);
	//VecCross(P1, P2, pd->poly_normal);
	//VecNormalize(pd->poly_normal);
	//if (ABS(pd->poly_normal[0]) >= ABS(pd->poly_normal[1])
	//	&& ABS(pd->poly_normal[0]) >= ABS(pd->poly_normal[2])) {
	//	pd->poly_p1 = 1;
	//	pd->poly_p2 = 2;
	//} else if (ABS(pd->poly_normal[1]) >= ABS(pd->poly_normal[0])
	//	&& ABS(pd->poly_normal[1]) >= ABS(pd->poly_normal[2])) {
	//	pd->poly_p1 = 0;
	//	pd->poly_p2 = 2;
	//} else {
	//	pd->poly_p1 = 0;
	//	pd->poly_p2 = 1;
	//}
	//pd->poly_d = - VecDot(pd->poly_normal, pd->poly_point[0]);
	//
	VecSub(media.poly_point[i][0], media.poly_point[i][1], P1);
	VecSub(media.poly_point[i][2], media.poly_point[i][1], P2);
	VecCross(P1, P2, media.poly_normal[i]);
	VecNormalize(media.poly_normal[i]);
	//
	if (ABS(media.poly_normal[i][0]) >= ABS(media.poly_normal[i][1])
			&& ABS(media.poly_normal[i][0]) >= ABS(media.poly_normal[i][2])) {
		media.poly_p1[i] = 1;
		media.poly_p2[i] = 2;
	}
	else if (ABS(media.poly_normal[i][1]) >= ABS(media.poly_normal[i][0])
					&& ABS(media.poly_normal[i][1]) >= ABS(media.poly_normal[i][2])) {
		media.poly_p1[i] = 0;
		media.poly_p2[i] = 2;
	}
	else {
		media.poly_p1[i] = 0;
		media.poly_p2[i] = 1;
	}
	media.poly_d[i] = - VecDot(media.poly_normal[i], media.poly_point[i][0]);
}
