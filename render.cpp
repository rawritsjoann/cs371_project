//cs371 Fall 2013
//program: render.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<GL/glx.h>
#include "defs.h"
#include "log.h"
#include "extern.h"
#include "fonts.h"
#include "files.h"

#define MAXWINSIZES 8
static const int winsize[MAXWINSIZES][2] = {
{160,120},
{320,240},
{400,300},
{640,480},
{800,600},
{480,540},
{1024,768},
//{960,1080},
{1280,960}};
static int currentWinsize=0;

void getCurrentWinsize(int *x, int *y) {
	*x = winsize[currentWinsize][0];
	*y = winsize[currentWinsize][1];
}

void currentWinsizeIncrement(const int dir) {
	if (dir > 0) {
		if (++currentWinsize >= MAXWINSIZES)
			currentWinsize = 0;
		return;
	}
	if (dir < 0) {
		if (--currentWinsize < 0)
			currentWinsize = MAXWINSIZES-1;
	}
}

void renderViewport(const int y, const int w, const int h) {
	//x: column, is always 0
	//y: row
	//w: width of area to be drawn
	//h: height of area to be drawn
	//float *p = g.screendata + (y*g.screen.xres3+x*3);
	float *p = g.screendata + (y*g.screen.xres3);
	//Log("render_viewport(%i %i %i %i)\n", 0, y, w, h);
	glRasterPos2i(0, y);
	glDrawPixels(w, h, GL_RGB, GL_FLOAT, (const GLvoid *)p);
}

void render(void)
{
	//printf("render() count: %i\n", ++ccount);
	//printf("g.mode: %i\n",g.mode);
	//g.ready_to_print=0;
	if (g.mode == MODE_HELP) {
		//Log("help\n");
		//Help
		int step=22;
		unsigned int cref=0x00550000;
		Rect r;
		r.left=10;
		r.bot=g.screen.yres-30;
		r.center=0;
		ggprint16(&r, 36, 0, "Help");
		ggprint16(&r, step, cref, "F2 - List of Scene files");
		ggprint16(&r, step, cref, "F3 - Edit scene file");
		ggprint16(&r, step, cref, "F4 - Antialias");
		ggprint16(&r, step, cref, "F5 - Refresh trace");
		ggprint16(&r, step, cref, "F6 - Save image");
		ggprint16(&r, step, cref, "F7 - Ctrl-F7 to build a cubemap");
		ggprint16(&r, step, cref, "F9 - Set window size");
		ggprint16(&r, step, cref, "Keys 1 - 8 set trace pixelation");
		ggprint16(&r, step, cref, "I - Set rendering interval");
		ggprint16(&r, step, cref, "R - Refresh trace");
		ggprint16(&r, step, cref, "Esc - Stop trace");
		ggprint16(&r, step, cref, "Ctrl-Z - Quit");
	}
	if (g.mode == MODE_FILES) {
		//Show files
		static void (*pfunc)(Rect *, int, int, const char *, ...);
		Rect r;
		int i, cref;
		//int nfiles = get_nscenefiles();
		//int currfile = get_current_scene_num();
		int nfiles = files.getNSceneFiles();
		int currfile = files.getCurrentSceneNum();
		int indent = 10, gap = 4;
		//
		//spacing based on largest font.
		int hstep = 160;
		int vstep = 21;
		int colWidth = hstep+gap;
		//How many rows and columns are on screen?
		int nrows = (g.screen.yres-vstep*3) / vstep + 1;
		int ncols = (g.screen.xres-indent) / (hstep+gap);
		//printf("ncols: %i  nrows: %i\n",ncols,nrows);
		//
		pfunc = ggprint16;
		//if (cols * rows < nfiles) {
		//	hstep = 120;
		//	vstep = 14;
		//	pfunc = ggprint08;
		//}
		//which row/col is the current file in?
		//int currCol = currfile / nrows;
		//int currRow = currfile % nrows;
		//printf("currfile: %i   currCol: %i  currRow: %i\n",
		//				currfile, currCol, currRow);
		//
		//Which file should the list start at?
		int start = 0;
		//Is the current file's row/col on screen???
		if (currfile >= nrows*ncols) {
			//This code can be refactored into a single line.
			//Any optimization will be minimal.
			int rowOffset = nrows;
			int diff = currfile - nrows*ncols;
			int additionalOffset = nrows * (diff / nrows);
			//printf("additionalOffset: %i\n", additionalOffset);
			rowOffset += additionalOffset;
			start = rowOffset;
			//printf("OFF SCREEN   start at: %i\n", start);
		}
		glClear(GL_COLOR_BUFFER_BIT);
		//
		//Number of rows and columns are recorded while drawing and used later.
		g.filesRowCol[0]=0;
		g.filesRowCol[1]=1;
		int rowcount=0;
		int maxrow=0;
		//
		r.left=indent;
		r.bot=g.screen.yres-vstep;
		r.center=0;
		pfunc(&r, vstep, 0x00aa0000, "Arrows, Enter, F5, F3");
		glLineWidth(2);
		for (i=0; i<nfiles; i++) {
			if (i<start) continue;
			rowcount++;
			cref = 0x00333366;
			if (i==currfile) {
				glColor3ub(60,50,0);
				glBegin(GL_LINE_LOOP);
					glVertex2i(r.left-2,   r.bot);
					glVertex2i(r.left-2,   r.bot+vstep-1);
					glVertex2i(r.left+hstep, r.bot+vstep-1);
					glVertex2i(r.left+hstep, r.bot);
				glEnd();
				glColor3ub(90,90,0);
				glBegin(GL_QUADS);
					glVertex2i(r.left-2,   r.bot);
					glVertex2i(r.left-2,   r.bot+vstep-1);
					glVertex2i(r.left+hstep, r.bot+vstep-1);
					glVertex2i(r.left+hstep, r.bot);
				glEnd();
				cref = 0x00eeee00;
			}
			pfunc(&r, vstep, cref, "%s", files.getSceneName(i));
			if (r.bot < vstep) {
				//Move to next column
				g.filesRowCol[1]++;
				r.left += (hstep + gap);
				if (r.left > (g.screen.xres - colWidth))
					break;
				r.bot = g.screen.yres - vstep * 2;
				if (rowcount > maxrow)
					maxrow = rowcount;
				rowcount=0;
			}
		}
		glLineWidth(1);
		g.filesRowCol[0] = maxrow;
		//g.mode = 0;
	}
	if (g.mode == MODE_WINDOW_SIZE) {
		//window size
		int i;
		unsigned int cref;
		Rect r;
		r.left=10;
		r.bot=g.screen.yres-30;
		r.center=0;
		glClear(GL_COLOR_BUFFER_BIT);
		ggprint16(&r, 26, 0x00aa0000, "Files...");
		glLineWidth(2);
		for (i=0; i<MAXWINSIZES; i++) {
			cref = (i==currentWinsize) ? 0x000000ff : 0x00000000;
			if (i==currentWinsize) {
				glColor3ub(90,90,0);
				glBegin(GL_LINE_LOOP);
					glVertex2i(r.left-2,   r.bot);
					glVertex2i(r.left-2,   r.bot+17);
					glVertex2i(r.left+400, r.bot+17);
					glVertex2i(r.left+400, r.bot);
				glEnd();
			}
			ggprint10(&r, 18, cref, "%i x %i",winsize[i][0],winsize[i][1]);
		}
		glLineWidth(1);
		r.bot -= 20;
		ggprint16(&r, 36, 0x00aa0000, "Up, Down, Enter");
	}
}


