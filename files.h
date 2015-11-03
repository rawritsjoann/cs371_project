#ifndef _FILES_H_
#define _FILES_H_

//#include <dirent.h> 

#define MAXFILES 1000
#define MAXFILELEN 256

//This is where scene files are kept when we are browsing them.
struct Scene {
	//this holds description of a .b file
	int type;
	char name[MAXFILELEN];
};

struct Files {
	Scene *scene;
	int n;
	int currentScene;
	char path[256];
	Files() {
		//constructor
		scene = new Scene [MAXFILES];
		n = 0;
		currentScene = 0;
		//do a string copy
		char xpath[] = "./scenes/";
		char *x = xpath, *p = path;
		while (*x)
			*(p++) = *(x++);
		*p = '\0';
	}
	~Files() {
		delete [] scene;
	}
	char *getFilePath() {
		return path;
	}
	char *getCurrentSceneName() {
		return (char *)scene[currentScene].name;
	}
	int getNSceneFiles() {
		return n;
	}
	void setNSceneFiles(int num) {
		n = num;
	}
	int getCurrentSceneNum() {
		return currentScene;
	}
	//void setCurrentSceneNum(int num) {
	//	currentScene = num;
	//}
	char *getSceneName(const int i) {
		return (char *)scene[i].name;
	}
	int currentIsDir();
	int setCurrentSceneName(char *sname);
	void currentSceneIncrement(const int dir);
	void goToParentDir();
	int dirWasSelected();
	void getFiles();
	void createDefaultFile();
};
extern struct Files files;

#endif //_FILES_H_

