#include "Globals.h"
#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleInput.h"
#include "ModuleParticles.h"
#include "ModuleRender.h"
#include "ModuleCollision.h"
#include "ModuleFadeToBlack.h"
#include "ModulePlayer.h"
#include "ModuleAudio.h"
#include <stdlib.h>  
#include "SDL/include/SDL.h"


// Reference at https://www.youtube.com/watch?v=OEhmUuehGOA

ModulePlayer::ModulePlayer(bool active) : Module(active)
{
	speed = 2;

	//Animation states:
	state = IDLE;
	attackState = BASIC_PUNCH;

	// idle animation (just the ship)
	idle.frames.push_back({ 36, 23, 26, 34 });
	idle.frames.push_back({ 69, 23, 26, 34 });
	idle.speed = 0.025f;
	// move upwards
	up.frames.push_back({ 598, 26, 26, 35 });
	up.frames.push_back({ 628, 26, 26, 35 });
	up.speed = 0.1f;
	// Move down
	down.frames.push_back({ 598, 26, 26, 35 });
	down.frames.push_back({ 628, 26, 26, 35 });
	down.speed = 0.1f;
	//Move forward
	forward.frames.push_back({ 437, 26, 29, 34 });
	forward.frames.push_back({ 467, 26, 26, 33 });
	forward.frames.push_back({ 500, 30, 34, 31 });
	forward.frames.push_back({ 536, 26, 22, 37 });
	forward.frames.push_back({ 561, 30, 34, 29 });
	forward.speed = 0.1f;

	// Jump 
	startJumpPosition = 0;
	jumping = false;
	goingUp = false;
	jump.frames.push_back({ 415,167,47,26 });
	jump.speed = 0.1f;
	jumpHeight = 40.0f;
	// Attack
	punching = false;
	rightPunch.frames.push_back({ 99,25,27,32 });
	rightPunch.frames.push_back({ 128,25,39,32 });
	rightPunch.speed = 0.1f;
	leftPunch.frames.push_back({ 99,25,27,32 });
	leftPunch.frames.push_back({ 168,26,35,32 });
	leftPunch.speed = 0.1f;
	// KickAttack
	renderWithOffset = AnimationRender();
	offsetLeftKick = { { 12, 0 },{ 18,14 } };
	offsetRighKick = { { 0,0 },{ 0,14 } };
	kickAttack.frames.push_back({ 167,165,41,35 });
	kickAttack.frames.push_back({ 212,154,43,48 });
	kickAttack.speed = 0.1f;
	// Final punch
	finalPunch.frames.push_back({208,17,35,42});
	finalPunch.frames.push_back({250,19,34,39});
	finalPunch.frames.push_back({285,24,52,32}); 
	finalPunch.frames.push_back({ 345, 3,45,56 });
	//finalPunch.frames.push_back({});
	finalPunch.speed = 0.1f;
}

ModulePlayer::~ModulePlayer()
{
	
}

// Load assets
bool ModulePlayer::Start()
{
	LOG("Loading player");

	graphics = App->textures->Load("rtype/BattletoadSprites/rash.gif");

	destroyed = false;
	position.x = 150;
	position.y = 120;

	SDL_Rect collRec;
	collRec.x = 150;
	collRec.y = 120;
	collRec.h = 16;
	collRec.w = 16;
	collider = App->collision->AddCollider(collRec);
	collider->colliderType = PLAYER;
	collider->addObserver(this);
	return true;
}

// Unload assets
bool ModulePlayer::CleanUp()
{
	LOG("Unloading player");

	App->textures->Unload(graphics);

	return true;
}

// Update: draw background
update_status ModulePlayer::Update()
{
	
	switch (state)
	{
	case IDLE:
		Idle();
		break;
	case WALK:
		Walk();
		break;
	case JUMP:
		Jump();
		break;
	case RUN:
		Run();
		break;
	case ATTACK:
		Attack();
		break;
	case SUPER_ATTACK:
		SuperAttack();
		break;
	case KICK_ATTACK:
		KickAttack();
		break;
	default:
		Idle();
		break;
	}

	return UPDATE_CONTINUE;
}

void ModulePlayer::onNotify(GameEvent event) 
{
	
}

void ModulePlayer::Walk() 
{	
	speed = 2;

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_IDLE && App->input->GetKey(SDL_SCANCODE_W) == KEY_IDLE &&
		App->input->GetKey(SDL_SCANCODE_A) == KEY_IDLE && App->input->GetKey(SDL_SCANCODE_D) == KEY_IDLE &&
		App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_IDLE && App->input->GetKey(SDL_SCANCODE_M) == KEY_IDLE) 
	{
		state = IDLE;
		return;
	}
		
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) 
	{	
		state = WALK;
		position.y -= speed;

		if (current_animation != &up)
		{
			up.Reset();
			current_animation = &up;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		state = WALK;
		position.y += speed;
		
		if (current_animation != &down)
		{
			down.Reset();
			current_animation = &down;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{	
		
		state = WALK;
		position.x -= speed;
		flipHorinzontal = true;
		current_animation = &forward;
	}

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{	
		state = WALK;
		position.x += speed;
		flipHorinzontal = false;
		current_animation = &forward;
	}

	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		state = RUN;
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) {
		goingUp = true;
		startJumpPosition = position.y;
		state = JUMP;
	}

	if (App->input->GetKey(SDL_SCANCODE_M) == KEY_REPEAT) {
		punching = true;
		state = ATTACK;
		return;
	}

	App->renderer->Blit(graphics, position.x, position.y, &(current_animation->GetCurrentFrame()), 0.1f, flipHorinzontal);
}

void ModulePlayer::Idle() 
{
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_IDLE && App->input->GetKey(SDL_SCANCODE_W) == KEY_IDLE &&
		App->input->GetKey(SDL_SCANCODE_A) == KEY_IDLE && App->input->GetKey(SDL_SCANCODE_D) == KEY_IDLE &&
		App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_IDLE && App->input->GetKey(SDL_SCANCODE_M) == KEY_IDLE&&
		App->input->GetKey(SDL_SCANCODE_N)==KEY_IDLE && App->input->GetKey(SDL_SCANCODE_B) == KEY_IDLE)
	{
		state = IDLE;
		current_animation = &idle;
		
	}
	
	else if (App->input->GetKey(SDL_SCANCODE_B) == KEY_REPEAT) {
		state = SUPER_ATTACK;
	}
	
	else if (App->input->GetKey(SDL_SCANCODE_N) == KEY_REPEAT) {
		state = KICK_ATTACK;
	}
	else {
		state = WALK;
	}
	App->renderer->Blit(graphics, position.x, position.y, &(current_animation->GetCurrentFrame()), 0.1f, flipHorinzontal);
}

void ModulePlayer::Run() 
{
	speed = 4;
	current_animation = &forward;

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		current_animation = &down;
		position.y -= speed;
		
	}

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		current_animation = &down;
		position.y += speed;
		
	}

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		current_animation = &forward;
		position.x -= speed;
		flipHorinzontal = true;
		
	}

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		current_animation = &forward;
		position.x += speed;
		flipHorinzontal = false;
		
	}

	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_UP) 
	{
		state = WALK;
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) 
	{
		goingUp = true;
		startJumpPosition = position.y;
		state = JUMP;
	}

	App->renderer->Blit(graphics, position.x, position.y, &(current_animation->GetCurrentFrame()), 0.1f, flipHorinzontal);
}

void ModulePlayer :: Jump() 
{	
	current_animation = &jump;
	if (goingUp == true) 
	{
		position.y -= 2;
		if (position.y <= startJumpPosition - jumpHeight)
			goingUp = false;

	}
	else {
		position.y += 2;
		if (startJumpPosition <= position.y)
		{
			state = IDLE;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		position.x -= speed;
		flipHorinzontal = true;
	}

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		position.x += speed;
		flipHorinzontal = false;
	}

	App->renderer->Blit(graphics, position.x, position.y, &(current_animation->GetCurrentFrame()), 0.1f, flipHorinzontal);
}

void ModulePlayer::Attack() 
{
	
	if (punching == true && current_animation->Finished()) {
		
		animationCounter = 0;
		if (punchCounter%2== 0) {
			current_animation = &rightPunch;
			flipCompensation = 16;
		}
		else {
			current_animation = &leftPunch;
			flipCompensation = 11;
		}
	}
	
	if (current_animation->Finished()) {
			state = IDLE;
			punching = false;
			rightPunch.Reset();
			leftPunch.Reset();
			++animationCounter;
			++punchCounter;
			return;
		
	}

	if (!current_animation->Finished() && flipHorinzontal) { 
		
		if (animationCounter >= 9 && animationCounter <= 20) {// frames where we need to adjust
			if(animationCounter <=18)
				App->renderer->Blit(graphics, position.x - flipCompensation, position.y, &(current_animation->GetCurrentFrame()), 0.1f, flipHorinzontal); // punche sprite
		}
		else {
			App->renderer->Blit(graphics, position.x , position.y, &(current_animation->GetCurrentFrame()), 0.1f, flipHorinzontal);// pre punch sprite
		}
		++animationCounter; // count the actual frame
	}
	else {
		App->renderer->Blit(graphics, position.x, position.y, &(current_animation->GetCurrentFrame()), 0.1f, flipHorinzontal);
	}
}


void ModulePlayer::SuperAttack() {
	current_animation = &finalPunch;
	App->renderer->Blit(graphics, position.x, position.y, &(current_animation->GetCurrentFrame()), 0.1f, flipHorinzontal);
	if (current_animation->Finished()) {
		finalPunch.Reset();
		state = IDLE;
	}
}


void ModulePlayer::KickAttack() {

	current_animation = &kickAttack;
	
	renderWithOffset.Update(App,graphics,current_animation,flipHorinzontal,position, offsetLeftKick, offsetRighKick);

	if (current_animation->Finished()) {
		
		kickAttack.Reset();
		state = IDLE;
	}
}

