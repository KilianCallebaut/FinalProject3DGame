#include "Character.h"
#include "Model.h"
#include <string>




Character::Character()
{
}

Character initCharacter()
{
	Character character;
	std::string base = "Resources\\Models\\Shadowman";
	Model m = loadModel(base + "\\Shadowman.obj");
	m.ka = Vector3f(0.1, 0, 0);
	m.kd = Vector3f(0.5, 0, 0);
	m.ks = 8.0f;
	character.characterModel = m;

	for (int i = 0; i < 24; i++) {
		std::string number;
		if (i < 9) {
			number = "0" + (i + 1);
		}
		else {
			number = "" + (i + 1);
		}
		character.runFrames[i] = loadModel(base + "\\RunAnimation\\Shadowmanv_0000" + number + ".obj");
		character.runFrames[i].ka = Vector3f(0.1, 0, 0);
		character.runFrames[i].kd = Vector3f(0.5, 0, 0);
		character.runFrames[i].ks = 8.0f;
	}
	return character;

}

Model Character::nextFrame()
{
	switch (mode) {
		case 0:
			return characterModel;
		case 1:
			runcounter += 1;
			if (runcounter == 24)
				runcounter = 0;
			return runFrames[runcounter];
	}
}
