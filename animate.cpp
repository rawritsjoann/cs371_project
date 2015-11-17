//cs371 Fall 2013
//program: animimate.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
//This file controls animation, especially cloth.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "log.h"
#include "vector.h"
#include "extern.h"
#include "ppm.h"
#include "files.h"
#include "animate.h"
struct Cloth cl;

void animControl(int num) {
	//This controls the creation of every animation frame.
	//This function will run in a loop until animation frames are complete.
	int i;
	char fnx[][16]={"cloth.b","cloth2.b","cloth3.b"};
	if (g.anim.running)
		return;
	cl.clothNgrabbed = 0;
	cl.clothSmoothing = 0;
	VecZero(cl.clothVelocity);
	cl.clothTexture[0]='\0';
	//
	g.anim.number = num;
	g.anim.frameno = 0;
	g.anim.running = 1;
	g.mode = MODE_NONE;
	files.setCurrentSceneName(fnx[g.anim.number]);
	//This will not draw, because nframes is defined in file.
	g.anim.skip=1;
	castRays(g.traceResolution);
	g.anim.frameno++;
	//
	cl.w = cl.clothWidth;
	cl.h = cl.clothHeight;
	cl.step = cl.clothStep;
	cl.mass = (Mass *)malloc((cl.h * cl.w + cl.w) * sizeof(Mass));
	cl.nmasses = 0;
	int max = (cl.clothWidth * cl.clothHeight) * 6;
	printf("max: %i\n",max);
	cl.spring = (Spring *)malloc(max * sizeof(Spring));
	cl.nsprings = 0;
	setupMasses(cl.clothCenter);
	setupSprings();

	char fname[256];
	for (i=0; i<g.anim.nframes; i++) {
		//When to skip a frame???
		checkForSkip();
		g.anim.frameRendered=0;
		castRays(g.traceResolution);
		g.anim.frameno++;
		//save image
		if (g.anim.saveFiles && g.anim.frameRendered) {
			//sprintf(fname,"./anim/f%03i.ppm",i);
			sprintf(fname,"./%s/f%03i.ppm", g.anim.directory, i);
			Ppmimage *image = ppm6CreateImage(g.screen.xres, g.screen.yres);
			floatdataToChar(g.screendata, (unsigned char *)image->data,
			                  g.screen.xres, g.screen.yres);
			ppm6SaveImage(image, fname);
			ppm6CleanupImage(image);
		}
		if (g.anim.frameStop && g.anim.frameno > g.anim.frameStop)
			break;
	}
	g.anim.running = 0;
	free(cl.mass);
	free(cl.spring);
	printf("\n"); fflush(stdout);
	return;
}

void checkForSkip() {
	//modFrames holds a value that mods the frame-number.
	//If (frameno mod modFrames) == 0, draw frame.
	//Allows skipping frames that would render slowly.
	g.anim.skip=0;
	if (g.anim.frameStart == g.anim.frameStop &&
		g.anim.frameStart == g.anim.frameno)
		return;
	if (
		(!g.anim.frameno) ||
		(g.anim.frameStart && g.anim.frameno < g.anim.frameStart) ||
		(g.anim.frameStop && g.anim.frameno > g.anim.frameStop) ||
		(g.anim.frameMod>1 && g.anim.frameno % g.anim.frameMod != 0)
	   ) {
		g.anim.skip=1;
	}
}

void setupMasses(Vec center) {
	int i,j,k;
	Flt y = center[1];
	Flt x = center[0] - ((cl.step * (Flt)(cl.w-1)) / 2.0);
	Flt z = center[2] - ((cl.step * (Flt)(cl.h-1)) / 2.0);
	cl.nmasses = 0;
	//printf("setupMasses(%lf %lf %lf)...\n",center[0],center[1],center[2]);
	for (i=0; i<cl.h; i++) {
		for (j=0; j<cl.w; j++) {
			cl.mass[cl.nmasses].mass = 1.0;
			cl.mass[cl.nmasses].oomass = 1.0 / cl.mass[cl.nmasses].mass;
			cl.mass[cl.nmasses].pos[0] = (Flt)j * cl.step + x;
			cl.mass[cl.nmasses].pos[1] = y;
			cl.mass[cl.nmasses].pos[2] = (Flt)i * cl.step + z;
			VecCopy(cl.clothVelocity, cl.mass[cl.nmasses].vel);
			VecZero(cl.mass[cl.nmasses].force);
			//Determine if this mass is grabbed.
			//This means it is being controlled by the user.
			cl.mass[cl.nmasses].grabbed = 0;
			for (k=0; k<cl.clothNgrabbed; k++) {
				if (cl.clothGrabbed[k] == cl.nmasses) {
					cl.mass[cl.nmasses].grabbed = 1;
					break;
				}
			}
			cl.nmasses++;
		}
	}
	printf("nmasses: %i\n",cl.nmasses);
}

Flt springLen(Spring *s) {
	Flt d0 = cl.mass[s->mass[1]].pos[0] - cl.mass[s->mass[0]].pos[0];
	Flt d1 = cl.mass[s->mass[1]].pos[1] - cl.mass[s->mass[0]].pos[1];
	Flt d2 = cl.mass[s->mass[1]].pos[2] - cl.mass[s->mass[0]].pos[2];
	return sqrt(d0*d0+d1*d1+d2*d2);
}

void setupSprings()
{
	int i,j;
	//setup springs
	cl.nsprings = 0;
	//structural
	for (i=0; i<cl.h; i++) {
		for (j=0; j<cl.w; j++) {
			if (j < cl.w-1) {
				cl.spring[cl.nsprings].mass[0] = i*cl.w+j;
				cl.spring[cl.nsprings].mass[1] = i*cl.w+j+1;
				cl.spring[cl.nsprings].length =
										springLen(&cl.spring[cl.nsprings]);
				cl.nsprings++;
			}
			if (i < cl.h-1) {
				cl.spring[cl.nsprings].mass[0] = i*cl.w+j;
				cl.spring[cl.nsprings].mass[1] = (i+1)*cl.w+j;
				cl.spring[cl.nsprings].length =
										springLen(&cl.spring[cl.nsprings]);
				cl.nsprings++;
			}
		}
	}
	//bend
	for (i=0; i<cl.h; i++) {
		for (j=0; j<cl.w; j++) {
			if (j < cl.w-2) {
				cl.spring[cl.nsprings].mass[0] = i*cl.w+j;
				cl.spring[cl.nsprings].mass[1] = i*cl.w+j+2;
				cl.spring[cl.nsprings].length =
										springLen(&cl.spring[cl.nsprings]);
				cl.nsprings++;
			}
			if (i < cl.h-2) {
				cl.spring[cl.nsprings].mass[0] = i*cl.w+j;
				cl.spring[cl.nsprings].mass[1] = (i+2)*cl.w+j;
				cl.spring[cl.nsprings].length =
										springLen(&cl.spring[cl.nsprings]);
				cl.nsprings++;
			}
		}
	}
	//shear
	for (i=0; i<cl.h-1; i++) {
		for (j=0; j<cl.w-1; j++) {
		 	cl.spring[cl.nsprings].mass[0] = i*cl.w+j;
			cl.spring[cl.nsprings].mass[1] = (i+1)*cl.w+j+1;
			cl.spring[cl.nsprings].length =
										springLen(&cl.spring[cl.nsprings]);
			cl.nsprings++;
			if (j > 0) {
				cl.spring[cl.nsprings].mass[0] = i*cl.w+j+1;
				cl.spring[cl.nsprings].mass[1] = (i+1)*cl.w+j;
				cl.spring[cl.nsprings].length =
										springLen(&cl.spring[cl.nsprings]);
				cl.nsprings++;
			}
		}
	}
	printf("nsprings: %i\n",cl.nsprings);
}

void maintainSprings()
{
	int i,k,m0,m1;
	Flt dist,factor,cent;
	Vec springVec;
	Vec springforce;
	//Move the masses...
	for (i=0; i<cl.nmasses; i++) {
		for (k=0; k<3; k++) {
			cl.mass[i].vel[k] += cl.mass[i].force[k] * cl.mass[i].oomass;
			cl.mass[i].pos[k] += cl.mass[i].vel[k];
			cl.mass[i].force[k] = 0.0;
			//Air resistance, or some type of damping
			cl.mass[i].vel[k] *= cl.clothDamping;
		}
	}
	//Resolve all springs...
	for (i=0; i<cl.nsprings; i++) {
		m0 = cl.spring[i].mass[0];
		m1 = cl.spring[i].mass[1];
		//forces are applied here
		//get vector between the two masses
		for (k=0; k<3; k++)
			springVec[k] = cl.mass[m0].pos[k] - cl.mass[m1].pos[k];
		dist = VecNormalizeL(springVec);
		//the spring force is added to the mass force
		factor = -(dist - cl.spring[i].length) * cl.stiffness;
		//if (factor > 0.001 || factor < -0.001) {
		for (k=0; k<3; k++) {
			springforce[k] = springVec[k] * factor;
			//apply positive force to one end of the spring,
			//and negative force to the other end.
			cl.mass[m0].force[k] += springforce[k];
			cl.mass[m1].force[k] -= springforce[k];
		}
		//}
		//distance between the two masses
		Flt maxlen = cl.spring[i].length * cl.stretchiness;
		//Flt maxlen = cl.spring[i].length * 1.1;
		if (dist > maxlen) {
			//max distance reached.
			if (cl.mass[m0].grabbed && !cl.mass[m1].grabbed) {
				//move m1 toward m0.
				for (k=0; k<3; k++) {
					cent = cl.mass[m0].pos[k];
					cl.mass[m1].pos[k] = cent + (maxlen * -0.5 * springVec[k]);
				}
			} else if (cl.mass[m1].grabbed && !cl.mass[m0].grabbed) {
				//move m0 toward m1.
				for (k=0; k<3; k++) {
					cent = cl.mass[m1].pos[k];
					cl.mass[m0].pos[k] = cent - (maxlen * -0.5 * springVec[k]);
				}
			} else {
				for (k=0; k<3; k++) {
					cent = (cl.mass[m0].pos[k] + cl.mass[m1].pos[k]) * 0.5;
					cl.mass[m0].pos[k] = cent - (maxlen * -0.5 * springVec[k]);
					cl.mass[m1].pos[k] = cent + (maxlen * -0.5 * springVec[k]);
				}
			}
		}
		//damping (turned on)
		for (k=0; k<3; k++) {
			springforce[k] = (cl.mass[m1].vel[k] - cl.mass[m0].vel[k]) * 0.02;
			cl.mass[m0].force[k] += springforce[k];
			cl.mass[m1].force[k] -= springforce[k];
		}
	}
}

#ifdef ALLOW_PATCHES
void findMassNormals()
{
	int i,j,l,m;
	int i0,i1,i2,i3;
	Vec norm;
	Vec a[3];
	//calculate a normal for each mass particle
	for (i=0; i<cl.nmasses; i++) {
		VecZero(cl.mass[i].norm);
	}
	for (i=0; i<cl.h-1; i++) {
		m = i+1;
		for (j=0; j<cl.w-1; j++) {
			l = j+1;
			i0 = i*cl.w+j;
			i1 = i*cl.w+l;
			i2 = m*cl.w+l;
			i3 = m*cl.w+j;
			//0,1,2
			VecCopy(cl.mass[i0].pos, a[0]);
			VecCopy(cl.mass[i1].pos, a[1]);
			VecCopy(cl.mass[i2].pos, a[2]);
			makeUnitNormalFromVerts(a, norm);
			VecAdd(norm, cl.mass[i0].norm, cl.mass[i0].norm);
			VecAdd(norm, cl.mass[i1].norm, cl.mass[i1].norm);
			VecAdd(norm, cl.mass[i2].norm, cl.mass[i2].norm);
			//0,2,3
			VecCopy(cl.mass[i0].pos, a[0]);
			VecCopy(cl.mass[i2].pos, a[1]);
			VecCopy(cl.mass[i3].pos, a[2]);
			makeUnitNormalFromVerts(a, norm);
			VecAdd(norm, cl.mass[i0].norm, cl.mass[i0].norm);
			VecAdd(norm, cl.mass[i2].norm, cl.mass[i2].norm);
			VecAdd(norm, cl.mass[i3].norm, cl.mass[i3].norm);
		}
	}
	for (i=0; i<cl.nmasses; i++) {
		VecNormalize(cl.mass[i].norm);
	}
}
#endif //ALLOW_PATCHES



void anim_loop_a(void) {
	//cloth falling on sphere
	static Sphere ss;
	int i,j,k,l,m;
	Vec v[4];
	int i0,i1,i2,i3;

	if (g.anim.frameno < 1) {
		MakeVector(0, 110, 0, ss.center);
		ss.radius = 50.0;
	} else {
		for (i=0; i<cl.nmasses; i++) {
			cl.mass[i].force[1] += cl.clothGravity;
			//collision with floor.
			if (cl.mass[i].pos[1] < 0.2) {
				cl.mass[i].pos[1] = 0.2;
				VecS(0.99, cl.mass[i].force, cl.mass[i].force);
				VecS(0.99, cl.mass[i].vel, cl.mass[i].vel);
				cl.mass[i].force[1] = 0.0;
				cl.mass[i].vel[1]   = 0.0;
			}
		}
		//---------------------------------------------------------------------
		if (g.anim.frameno > 0) {
			int m0;
			for (j=0; j<cl.clothNgrabbed; j++) {
				m0 = cl.clothGrabbed[j];
				VecZero(cl.mass[m0].force);
				VecZero(cl.mass[m0].vel);
			//	cl.mass[m0].pos[0] -= 0.25;
			//	cl.mass[m0].pos[1] += 0.1;
			}
			//---------------------------------------------------------------------
			//take control over the grabbed masses.
			Vec sv[16];
			for (j=0; j<cl.clothNgrabbed; j++) {
				VecCopy(cl.mass[cl.clothGrabbed[j]].pos, sv[j]);
			}
			for (j=0; j<cl.clothSpringSteps; j++)
				maintainSprings();
			for (j=0; j<cl.clothNgrabbed; j++) {
				VecCopy(sv[j], cl.mass[cl.clothGrabbed[j]].pos);
			}
		} else {
			for (j=0; j<cl.clothSpringSteps; j++)
				maintainSprings();
		}
		//
		//check for cloth collision
		Flt r = ss.radius+8.0;
		for (i=0; i<cl.nmasses; i++) {
			Vec d;
			VecSub(cl.mass[i].pos, ss.center, d);
			Flt dist = sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);
			if (dist < r) {
				//move mass to the sphere edge.
				VecNormalize(d);
				for (k=0; k<3; k++) {
					cl.mass[i].pos[k] = ss.center[k] + d[k] * r * 1.01;
					//some penalty away from surface
					cl.mass[i].force[k] += d[k] * 0.1;
				}
				VecS(0.995, cl.mass[i].vel, cl.mass[i].vel);
			}
		}

	}
	//put movable objects into a tmp scene file
	if (!g.anim.skip) {
		const char tmp[]="g.anim.txt";
		FILE *fpo = fopen(tmp,"w");
		if (!fpo) exit(1);
		fprintf(fpo,"surf: .3 .6 .3\n");
		fprintf(fpo,"surf-specular: .5 .5 .5\n");
		fprintf(fpo,"sphere: %lf %lf %lf %lf\n",
						ss.center[0], ss.center[1], ss.center[2], ss.radius);
		fprintf(fpo,"surf: .9 .2 .2\n");
		Flt step[2]={1,1};
		if (cl.clothTexture[0]) {
			fprintf(fpo,"surf-texmap: %s\n",cl.clothTexture);
			step[0] = cl.clothTexTile[0]/(Flt)cl.w;
			step[1] = cl.clothTexTile[1]/(Flt)cl.h;
		}

		#ifdef ALLOW_PATCHES
		if (cl.clothSmoothing) {
			findMassNormals();
		}
		#endif //ALLOW_PATCHES

		for (i=0; i<cl.h-1; i++) {
			m = i+1;
			for (j=0; j<cl.w-1; j++) {
				l = j+1;
				i0 = i*cl.w+j;
				i1 = i*cl.w+l;
				i2 = m*cl.w+l;
				i3 = m*cl.w+j;
				VecCopy(cl.mass[i0].pos, v[0]);
				VecCopy(cl.mass[i1].pos, v[1]);
				VecCopy(cl.mass[i2].pos, v[2]);
				VecCopy(cl.mass[i3].pos, v[3]);
				//0,1,2
				fprintf(fpo,"tri:\n");
				fprintf(fpo,"%lf %lf %lf\n",v[0][0], v[0][1], v[0][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[1][0], v[1][1], v[1][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[2][0], v[2][1], v[2][2]);
				if (cl.clothTexture[0]) {
					fprintf(fpo,"tri-texcoord: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf\n",
						step[0]*(Flt)j,     step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)(i+1));
				}

				#ifdef ALLOW_PATCHES
				if (cl.clothSmoothing) {
					fprintf(fpo,"tri-patch: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
					cl.mass[i0].norm[0],cl.mass[i0].norm[1],cl.mass[i0].norm[2],
					cl.mass[i1].norm[0],cl.mass[i1].norm[1],cl.mass[i1].norm[2],
					cl.mass[i2].norm[0],cl.mass[i2].norm[1],cl.mass[i2].norm[2]);
				}
				#endif //ALLOW_PATCHES

				//0,2,3
				fprintf(fpo,"tri:\n");
				fprintf(fpo,"%lf %lf %lf\n",v[0][0], v[0][1], v[0][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[2][0], v[2][1], v[2][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[3][0], v[3][1], v[3][2]);
				if (cl.clothTexture[0]) {
					fprintf(fpo,"tri-texcoord: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf\n",
						step[0]*(Flt)j,     step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)(i+1),
						step[0]*(Flt)j,     step[1]*(Flt)(i+1));
				}

				#ifdef ALLOW_PATCHES
				if (cl.clothSmoothing) {
					fprintf(fpo,"tri-patch: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
					cl.mass[i0].norm[0],cl.mass[i0].norm[1],cl.mass[i0].norm[2],
					cl.mass[i2].norm[0],cl.mass[i2].norm[1],cl.mass[i2].norm[2],
					cl.mass[i3].norm[0],cl.mass[i3].norm[1],cl.mass[i3].norm[2]);
				}
				#endif //ALLOW_PATCHES

			}
		}
		fclose(fpo);
		FILE *fpi = fopen(tmp,"r");
		if (!fpi) exit(1);
		fileloop(fpi);
		fclose(fpi);
	}
	return;
}

void anim_loop_b(void) {
	//drag a cloth across scene.
	static Sphere ss[3];
	static int nspheres=1;
	int i,j,k,l,m;
	Vec v[4];
	int i0,i1,i2,i3;
	Flt r, dist;
	if (g.anim.frameno < 1) {
		//spheres
		MakeVector(0.0, 40.0, 0.0, ss[0].center);
		ss[0].radius = 60.0;
		nspheres = 1;
	} else {
		for (i=0; i<cl.nmasses; i++) {
			//gravity
			cl.mass[i].force[1] += cl.clothGravity;
			//collision with floor?
			if (cl.mass[i].pos[1] < 0.2) {
				cl.mass[i].pos[1] = 0.22;
				VecS(0.994, cl.mass[i].force, cl.mass[i].force);
				VecS(0.994, cl.mass[i].vel, cl.mass[i].vel);
				cl.mass[i].force[1] = 0.0;
				cl.mass[i].vel[1]   = 0.0;
			}
		}

		//---------------------------------------------------------------------
		//pull cloth across the scene
		if (g.anim.frameno > 0) {
			for (j=0; j<cl.clothNgrabbed; j++) {
				k = cl.clothGrabbed[j];
				VecZero(cl.mass[k].force);
				VecZero(cl.mass[k].vel);
				if (g.anim.frameno > 1) {
					cl.mass[k].pos[0] += -0.2;
				}
			}
			Vec sv[16];
			for (j=0; j<cl.clothNgrabbed; j++) {
				VecCopy(cl.mass[cl.clothGrabbed[j]].pos, sv[j]);
			}
			for (j=0; j<cl.clothSpringSteps; j++)
				maintainSprings();
			for (j=0; j<cl.clothNgrabbed; j++) {
				VecCopy(sv[j], cl.mass[cl.clothGrabbed[j]].pos);
			}
		} else {
			for (j=0; j<cl.clothSpringSteps; j++)
				maintainSprings();
		}
		//
		//check for cloth collision after spring resolution
		for (i=0; i<cl.nmasses; i++) {
			Vec d;
			for (j=0; j<nspheres; j++) {
				r = ss[j].radius+4.0;
				VecSub(cl.mass[i].pos, ss[j].center, d);
				dist = sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
				if (dist < r) {
					//move mass to the sphere edge.
					VecNormalize(d);
					for (k=0; k<3; k++) {
						cl.mass[i].pos[k] = ss[j].center[k] + d[k] * r * 1.01;
						cl.mass[i].force[k] += d[k] * 0.1;
					}
					VecS(0.94, cl.mass[i].vel, cl.mass[i].vel);
				}
			}
		}
	}
	//put movable objects into a scene file
	if (!g.anim.skip) {
		const char tmp[]="g.anim.txt";
		FILE *fpo = fopen(tmp,"w");
		if (!fpo) exit(1);
		for (j=0; j<nspheres; j++) {
			fprintf(fpo,"surf: .3 .6 0\n");
			fprintf(fpo,"surf-specular: .2 .2 .2\n");
			fprintf(fpo,"sphere: %lf %lf %lf %lf\n",
							ss[j].center[0], ss[j].center[1], ss[j].center[2],
							ss[j].radius);
		}
		fprintf(fpo,"surf: .9 .2 .2\n");
		Flt step[2]={1,1};
		if (cl.clothTexture[0]) {
			fprintf(fpo,"surf-texmap: %s\n",cl.clothTexture);
			step[0] = cl.clothTexTile[0]/(Flt)cl.w;
			step[1] = cl.clothTexTile[1]/(Flt)cl.h;
		}

		#ifdef ALLOW_PATCHES
		if (cl.clothSmoothing) {
			findMassNormals();
		}
		#endif //ALLOW_PATCHES

		for (i=0; i<cl.h-1; i++) {
			m = i+1;
			for (j=0; j<cl.w-1; j++) {
				l = j+1;
				i0 = i*cl.w+j;
				i1 = i*cl.w+l;
				i2 = m*cl.w+l;
				i3 = m*cl.w+j;
				VecCopy(cl.mass[i0].pos, v[0]);
				VecCopy(cl.mass[i1].pos, v[1]);
				VecCopy(cl.mass[i2].pos, v[2]);
				VecCopy(cl.mass[i3].pos, v[3]);
				//0,1,2
				fprintf(fpo,"tri:\n");
				fprintf(fpo,"%lf %lf %lf\n",v[0][0], v[0][1], v[0][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[1][0], v[1][1], v[1][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[2][0], v[2][1], v[2][2]);
				if (cl.clothTexture[0]) {
					fprintf(fpo,"tri-texcoord: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf\n",
						step[0]*(Flt)j,     step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)(i+1));
				}

				#ifdef ALLOW_PATCHES
				if (cl.clothSmoothing) {
					fprintf(fpo,"tri-patch: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
					cl.mass[i0].norm[0],cl.mass[i0].norm[1],cl.mass[i0].norm[2],
					cl.mass[i1].norm[0],cl.mass[i1].norm[1],cl.mass[i1].norm[2],
					cl.mass[i2].norm[0],cl.mass[i2].norm[1],cl.mass[i2].norm[2]);
				}
				#endif //ALLOW_PATCHES

				//0,2,3
				fprintf(fpo,"tri:\n");
				fprintf(fpo,"%lf %lf %lf\n",v[0][0], v[0][1], v[0][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[2][0], v[2][1], v[2][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[3][0], v[3][1], v[3][2]);
				if (cl.clothTexture[0]) {
					fprintf(fpo,"tri-texcoord: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf\n",
						step[0]*(Flt)j,     step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)(i+1),
						step[0]*(Flt)j,     step[1]*(Flt)(i+1));
				}

				#ifdef ALLOW_PATCHES
				if (cl.clothSmoothing) {
					fprintf(fpo,"tri-patch: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
					cl.mass[i0].norm[0],cl.mass[i0].norm[1],cl.mass[i0].norm[2],
					cl.mass[i2].norm[0],cl.mass[i2].norm[1],cl.mass[i2].norm[2],
					cl.mass[i3].norm[0],cl.mass[i3].norm[1],cl.mass[i3].norm[2]);
				}
				#endif //ALLOW_PATCHES

			}
		}

		fclose(fpo);
		FILE *fpi = fopen(tmp,"r");
		if (!fpi) exit(1);
		fileloop(fpi);
		fclose(fpi);
	}
	return;
}


void anim_loop_c(void) {
	//table cloth on table.
	static Sphere ss[120];
	static int nspheres = 0;
	int i,j,k,l,m;
	int i0,i1,i2,i3;
	Vec v[4], d;
	Flt r, dist;

	if (g.anim.frameno < 1) {
		//a ring of collision spheres...
		int n=90;
		Flt ang = 0.0, inc = (3.14159*2.0) / (Flt)n;
		nspheres = 0;
		for (i=0; i<n; i++) {
			MakeVector(cos(ang)*60.0, 80.0, sin(ang)*60.0, ss[i].center);
			ss[i].radius = 8.0;
			ang += inc;
			nspheres++;
		}
	} else {
		for (i=0; i<cl.nmasses; i++) {
			//gravity
			//cl.mass[i].force[1] += -0.009;
			cl.mass[i].force[1] += cl.clothGravity;
			//collision with floor.
			if (cl.mass[i].pos[1] < 0.2) {
				cl.mass[i].pos[1] = 0.22;
				VecS(0.9, cl.mass[i].force, cl.mass[i].force);
				VecS(0.9, cl.mass[i].vel, cl.mass[i].vel);
				cl.mass[i].force[1] = 0.0;
				cl.mass[i].vel[1]   = 0.0;
			}
		}

		//---------------------------------------------------------------------
		//pull cloth across the scene
		if (g.anim.frameno > 1000) {
			//if (g.anim.frameno > 2500)
			//	ngrabbed=0;
			for (j=0; j<cl.clothNgrabbed; j++) {
				k = cl.clothGrabbed[j];
				//mass[k].pos[1] = 90.0;
				VecZero(cl.mass[k].force);
				VecZero(cl.mass[k].vel);
				if (g.anim.frameno > 100) {
					cl.mass[k].pos[0] += -0.2;
				}
			}
			Vec sv[16];
			for (j=0; j<cl.clothNgrabbed; j++) {
				VecCopy(cl.mass[cl.clothGrabbed[j]].pos, sv[j]);
			}
			for (j=0; j<cl.clothSpringSteps; j++)
				maintainSprings();
			for (j=0; j<cl.clothNgrabbed; j++) {
				VecCopy(sv[j], cl.mass[cl.clothGrabbed[j]].pos);
			}
		} else {
			for (j=0; j<cl.clothSpringSteps; j++)
				maintainSprings();
		}
		//
		//check for cloth collision
		for (i=0; i<cl.nmasses; i++) {
			//table top
			// 95h 70r
			Vec cent = {0.0, 88.0, 0.0};
			VecSub(cl.mass[i].pos, cent, d);
			dist = sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
			if (dist < 59.0 &&
				cl.mass[i].pos[1] < 88.0 &&
				cl.mass[i].pos[1] > 85.0) {
					//move mass to the top of table.
					cl.mass[i].pos[1] = 88.1;
					cl.mass[i].vel[1] = 0.0;
					cl.mass[i].force[1] = 0.0;
					VecS(0.5, cl.mass[i].vel, cl.mass[i].vel);
					VecS(0.5, cl.mass[i].force, cl.mass[i].force);
			}

			for (j=0; j<nspheres; j++) {
				r = ss[j].radius;
				VecSub(cl.mass[i].pos, ss[j].center, d);
				dist = sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
				if (dist < r) {
					//move mass to the sphere edge.
					VecNormalize(d);
					for (k=0; k<3; k++) {
						cl.mass[i].pos[k] = ss[j].center[k] + d[k] * r * 1.01;
						cl.mass[i].force[k] += d[k] * 0.1;
					}
					VecS(0.94, cl.mass[i].vel, cl.mass[i].vel);
					VecZero(cl.mass[i].vel);
					VecZero(cl.mass[i].force);
				}
			}
		}
	}
	//
	//put movable objects into a tmp scene file
	if (!g.anim.skip) {
		const char tmp[]="g.anim.txt";
		FILE *fpo = fopen(tmp,"w");
		if (!fpo) exit(1);
		//printf("build scene items...\n"); fflush(stdout);
		fprintf(fpo,"surf: .9 .2 .2\n");
		Flt step[2]={1,1};
		if (cl.clothTexture[0]) {
			fprintf(fpo,"surf-texmap: %s\n",cl.clothTexture);
			step[0] = cl.clothTexTile[0]/(Flt)cl.w;
			step[1] = cl.clothTexTile[1]/(Flt)cl.h;
		}
		#ifdef ALLOW_PATCHES
		if (cl.clothSmoothing) {
			findMassNormals();
		}
		#endif //ALLOW_PATCHES
		for (i=0; i<cl.h-1; i++) {
			m = i+1;
			for (j=0; j<cl.w-1; j++) {
				l = j+1;
				i0 = i*cl.w+j;
				i1 = i*cl.w+l;
				i2 = m*cl.w+l;
				i3 = m*cl.w+j;
				VecCopy(cl.mass[i0].pos, v[0]);
				VecCopy(cl.mass[i1].pos, v[1]);
				VecCopy(cl.mass[i2].pos, v[2]);
				VecCopy(cl.mass[i3].pos, v[3]);
				//0,1,2
				fprintf(fpo,"tri:\n");
				fprintf(fpo,"%lf %lf %lf\n",v[0][0], v[0][1], v[0][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[1][0], v[1][1], v[1][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[2][0], v[2][1], v[2][2]);
				if (cl.clothTexture[0]) {
					fprintf(fpo,"tri-texcoord: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf\n",
						step[0]*(Flt)j,     step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)(i+1));
				}
				#ifdef ALLOW_PATCHES
				if (cl.clothSmoothing) {
					fprintf(fpo,"tri-patch: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
					cl.mass[i0].norm[0],cl.mass[i0].norm[1],cl.mass[i0].norm[2],
					cl.mass[i1].norm[0],cl.mass[i1].norm[1],cl.mass[i1].norm[2],
					cl.mass[i2].norm[0],cl.mass[i2].norm[1],cl.mass[i2].norm[2]);
				}
				#endif //ALLOW_PATCHES
				//0,2,3
				fprintf(fpo,"tri:\n");
				fprintf(fpo,"%lf %lf %lf\n",v[0][0], v[0][1], v[0][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[2][0], v[2][1], v[2][2]);
				fprintf(fpo,"%lf %lf %lf\n",v[3][0], v[3][1], v[3][2]);
				if (cl.clothTexture[0]) {
					fprintf(fpo,"tri-texcoord: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf\n",
						step[0]*(Flt)j,     step[1]*(Flt)i,
						step[0]*(Flt)(j+1), step[1]*(Flt)(i+1),
						step[0]*(Flt)j,     step[1]*(Flt)(i+1));
				}
				#ifdef ALLOW_PATCHES
				if (cl.clothSmoothing) {
					fprintf(fpo,"tri-patch: ");
					fprintf(fpo,"%lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
					cl.mass[i0].norm[0],cl.mass[i0].norm[1],cl.mass[i0].norm[2],
					cl.mass[i2].norm[0],cl.mass[i2].norm[1],cl.mass[i2].norm[2],
					cl.mass[i3].norm[0],cl.mass[i3].norm[1],cl.mass[i3].norm[2]);
				}
				#endif //ALLOW_PATCHES
			}
		}
		fclose(fpo);
		FILE *fpi = fopen(tmp,"r");
		if (!fpi) exit(1);
		fileloop(fpi);
		fclose(fpi);
	}
	return;
}

void anim_loop_d(void)
{
	static double dx = 0.0;
	if (!g.anim.skip) {
		const char tmp[]="g.anim.txt";
		FILE *fpo = fopen(tmp,"w");
		if (!fpo) exit(1);
		//printf("build scene items...\n"); fflush(stdout);

		fprintf(fpo,"trans-translate: %lf 0 0\n", dx);
		fprintf(fpo,"checker:\n");
		fprintf(fpo,"2 0 2\n");
		fprintf(fpo,".8 .8 .8  .4 .3 .1\n");
		fprintf(fpo,"surf: .01 .01 .01\n");
		fprintf(fpo,"surf-specular: .2 .2 .2\n");
		fprintf(fpo,"surf-pattern: 1\n");
		fprintf(fpo,"ring: 0 0 0  0 1 0  0 1000\n");
		fprintf(fpo,"trans-pop: 1\n");

		dx -= 0.1;

		fclose(fpo);
		FILE *fpi = fopen(tmp,"r");
		if (!fpi) exit(1);
		fileloop(fpi);
		fclose(fpi);
	}
	return;
}

void anim_loop_e(void)
{
	static double dy = 36.0;
	if (!g.anim.skip) {
		const char tmp[]="g.anim.txt";
		FILE *fpo = fopen(tmp,"w");
		if (!fpo) exit(1);
		//printf("build scene items...\n"); fflush(stdout);
		fprintf(fpo,"trans-rotate: 0 %lf 0\n", dy);
		fprintf(fpo,"surf: 1 1 0\n");
		fprintf(fpo,"box: LTRBFB 0 0 0  1 1 1\n");
		fprintf(fpo,"trans-pop: 1\n");
		dy -= 4.5;
		fclose(fpo);
		FILE *fpi = fopen(tmp,"r");
		if (!fpi) exit(1);
		fileloop(fpi);
		fclose(fpi);
	}
	return;
}


