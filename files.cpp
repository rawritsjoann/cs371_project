//cs371 Fall 2013
//program: files.cpp
//author:  Gordon Griesel
//date:    Summer 2013
//purpose: framework for ray tracing
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include "defs.h"
#include "log.h"
#include "files.h"

//This is where scene files are kept
struct Files files;

void replaceFileExtension(char *file, const char *ext)
{
	//printf("get_good_image_name()\n");
	//strcpy(file, (char *)scenefiles[current_scene]);
	//printf("**%s**\n", file);
	//start looking from right
	int slen = strlen(file);
	for (int i=slen-1; i>=0; i--) {
		if (file[i] == '.') {
			file[i] = '\0';
			break;
		}
	}
	strcat(file, ext);
	//printf("**%s**\n", file);
}

int Files::currentIsDir()
{
	return (scene[currentScene].type == DT_DIR);
}

int Files::setCurrentSceneName(char *sname)
{
	//this was developed to animate cloth.
	int i;
	for (i=0; i<n; i++) {
		if (strcmp(sname, (char *)scene[i].name) == 0) {
			currentScene = i;
			return 0;
		}
	}
	return 1;
}

void Files::currentSceneIncrement(const int dir)
{
	//This function helps us move through the list of files
	//while it is shown on the screen.
	int absdir = ABS(dir);
	int save_cs = currentScene;
	currentScene += dir;
	if (currentScene >= n) {
		if (absdir == 1) {
			currentScene = 0;
			return;
		}
		currentScene = save_cs;
	}
	if (currentScene < 0) {
		if (absdir == 1) {
			currentScene = n-1;
			return;
		}
		currentScene = save_cs;
	}
	return;
}

void Files::goToParentDir()
{
	//User selected the .. entry on the file list.
	//
	//printf("goToParentDir()...\n");
	//printf("old scenePath **%s**\n", path);
	//Current path must have 2 slashes.
	int slashCount=0;
	char *p = path;
	while (*p) {
		if (*p == '/')
			slashCount++;
		p++;
	}
	if (slashCount < 2)
		return;
	//
	//find the 2nd / from the right.
	int n=0;
	int k = strlen(path) - 1;
	for (int i=k; i>=0; i--) {
		if (path[i] == '/') {
			n++;
			if (n == 2) {
				//printf("found slash\n");
				path[i+1] = '\0';
				break;
			}
		}
	}
	//printf("new files.path **%s**\n", path);
}

int Files::dirWasSelected()
{
	//printf("dirWasSelected()...\n");
	//move ahead to a subdirectory.
	//printf("scenefiles[current_scene].name **%s**\n",
	//						scene[currentScene].name);
	if (scene[currentScene].type == DT_DIR) {
		//printf("type is DT_DIR\n");
		//Space character is on front of filename, for sorting.
		if (strcmp(scene[currentScene].name, " ..") == 0) {
			goToParentDir();
			return 1;
		}
		strcat(path, scene[currentScene].name+1);
		strcat(path, "/");
		//printf("new path **%s**\n", path);
		return 1;
	}
	return 0;
}

void Files::getFiles()
{
	//printf("get_files()...\n");
	//This gets the filenames in a directory
	//and populates scenefiles array.
	int i,j,slen;
	DIR *d;
	struct dirent *dir;
	//printf("path **%s**\n", path);
	d = opendir(path);
	int firsttime = (n==0);
	n = 0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			slen = strlen(dir->d_name);
			if (slen < 2) continue;
			if (slen >= MAXFILELEN) continue;
			if (n >= MAXFILES) continue;
			if ((dir->d_type == DT_DIR) ||
			    (dir->d_name[slen-2]=='.' && dir->d_name[slen-1]=='b')) {
				//show all subdirectories, except "."
				strcpy(scene[n].name, dir->d_name);
				scene[n].type = dir->d_type;
				if (dir->d_type == DT_DIR) {
					//	printf("found sub **%s**\n",
					//				scenefiles[nscenefiles].name);
					//precatenate a space before sorting.
					//adjust when using.
					for (int i=slen; i>0; i--)
						scene[n].name[i] = scene[n].name[i-1];
					scene[n].name[0] = ' ';
					scene[n].name[slen+1] = '\0';
				}
				n++;
			}
		}
		closedir(d);
		if (n > 0) {
			//sort the file names.
			struct Scene tmp;
			for (i=0; i<n-1; i++) {
				for (j=0; j<n-1-i; j++) {
					if (strcmp(scene[j+1].name, scene[j].name) < 0) {
						memcpy(&tmp,        &scene[j+1], sizeof(Scene));
						memcpy(&scene[j+1], &scene[j],   sizeof(Scene));
						memcpy(&scene[j],   &tmp,        sizeof(Scene));
					}
				}
			}
		}
	}
	if (n == 0) {
		//No scene files found, so create one.
		createDefaultFile();
	}
	if (firsttime) {
		//Move current-file index to a script file, not a dir.
		//The first file without a leading space.
		currentScene = 0;
		//int i=0;
		while (currentScene < n) {
			if (scene[currentScene].name[0] != ' ')
				break;
			currentScene++;
		}
		if (currentScene >= n)
			currentScene = 0;
	}
}

void Files::createDefaultFile(void)
{
	char ts[256];
	sprintf(ts, "%saatemp.b", path);
	FILE *fpo = fopen(ts, "w");
	if (!fpo) {
		Log("ERROR: opening **%s** file\n", ts);
		exit(EXIT_FAILURE);
	}
	fprintf(fpo, "from: 0 0 2\n");
	fprintf(fpo, "at: 0 0 0\n");
	fprintf(fpo, "angle: 45\n");
	fprintf(fpo, "up: 0 1 0\n");
	fprintf(fpo, "ambient: .2 .2 .2\n");
	fprintf(fpo, "background: 0 0 .5\n");
	fprintf(fpo, "light: 0 1 2000\n");
	fprintf(fpo, "light-color: 1 1 1\n");
	fprintf(fpo, "surf: 1 0 0\n");
	fprintf(fpo, "sphere: 0 0 0 1\n");
	fclose(fpo);
	strcpy(scene[0].name, ts);
	n=1;
}


