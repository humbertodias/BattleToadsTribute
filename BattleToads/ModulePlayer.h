#ifndef __ModulePlayer_H__
#define __ModulePlayer_H__

#include "Module.h"
#include "Animation.h"
#include "Point.h"
#include "Observer.h"
#include "ModuleCollision.h"
#include "Temporizer.h"
#include "AnimationRender.h"

#define CAMERA_SPEED_WALK 3
#define CAMERA_SPEED_RUN 5
#define STOP_CAMERA_BEGGINING 178

struct SDL_Texture;

enum AnimationState { IDLE,WALK,JUMP,RUN,ATTACK,SUPER_ATTACK,KICK_ATTACK,RECEIVE_HEAVY_ATTACK,GOING_DOWN }; // Desirable have dragon , stick
enum AnimationAttacksState { BASIC_PUNCH,SUPER_PUNCH,HEAD_PUNCH, KICK_PUNCH, TONGUE_PUNCH };

class ModulePlayer : public Module ,Observer
{
public:
	ModulePlayer(bool active = true);
	~ModulePlayer();
	bool Start();
	update_status Update();
	bool CleanUp();
	void onNotify(GameEvent event);

private:
	void Jump();
	void Walk();
	void Idle();
	void Run();
	void Attack();
	void SuperAttack();
	void KickAttack();
	void ReceiveHeavyAttack();
	void SetCollidersPosition();
	void GoingDown();
	
	
public:
	int speed;
	int life = 2000;
	int totalLife = 10;
	SDL_Texture* graphics = nullptr;
	Animation* current_animation = nullptr;
	iPoint position;
	bool destroyed = false;
	Collider *collider;
	Collider *colliderAttack;
	
	// Walk
	Animation forward;
	Animation backward;
	Animation up;
	Animation down;
	//idle
	Animation idle;
	// jump
	Animation jump;
	bool jumping;
	bool goingUp;
	int startJumpPosition;
	int jumpHeight;
	//Attack
	Animation rightPunch;
	Animation leftPunch;
	vector<iPoint> offsetLeftAttackLeft;
	vector<iPoint> offsetRightAttackLeft;
	vector<iPoint> offsetLeftAttackRight;
	vector<iPoint> offsetRightAttackRight;
	bool punching;
	//Kick Attack
	vector<iPoint> offsetLeftKick;
	vector<iPoint> offsetRighKick;
	AnimationRender renderWithOffset;
	Animation kickAttack;
	//Super Attack
	vector<iPoint> offsetLeftFinalPunch;
	vector<iPoint> offsetRighFinalPunch;
	Animation finalPunch;
	//Receive heavy attack
	vector<iPoint> offsetLeftReceiveHeavyAttack;
	vector<iPoint> offsetRighReceiveHeavyAttack;
	Animation receiveHeavyAttack;
	Temporizer timeDown;
	AnimationState state;
	bool flagLockAttackZone = false;
	
private:

	int  blockRight = 0;
	int  blockLeft = 0;
	bool cameraLocking = false;
	int punchCounter = 0;
	bool flipHorinzontal = false;
	AnimationAttacksState attackState;
	void LockAttackZone();
	int audioIDpunch;
	int audioIDdamage;
	bool flagAudio = true;

};

#endif