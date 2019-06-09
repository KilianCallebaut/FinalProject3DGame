#pragma once
#include "Model.h"

class Character
{
public:
	Character();
	Model characterModel;
	Model *runFrames;
	Vector3f position;
	Vector3f rotation;
	float scale = 1;

	//mode: 0=still, 1=running
	int mode = 0;

	int runcounter;

	Model nextFrame();
};

Character initCharacter();

