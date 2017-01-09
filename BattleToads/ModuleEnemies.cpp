#include "ModuleEnemies.h"
#include "ModulePlayer.h"
Enemy::Enemy() 
{
	
}

Enemy::Enemy(int x, int y)
{
	position.x = x;
	position.y = y;
	
	state = WALK_ENEMY;

	animationWalk.frames.push_back({ 13, 55, 36, 31 });
	animationWalk.frames.push_back({ 54, 56, 36, 31 });
	animationWalk.frames.push_back({ 94, 60, 36, 31 });
	animationWalk.speed = 0.1;

	animationAttack.frames.push_back({ 50, 15, 36, 31 });
	animationAttack.frames.push_back({ 90, 17, 41, 28 });
	animationAttack.frames.push_back({ 50, 15, 41, 28 });
	animationAttack.speed = 0.1;

	animationDead.frames.push_back({145,64,33,35});
	animationDead.frames.push_back({ 14,100,33,25 });
	animationDead.speed = 0.1;
	animationDead.loop = false;

	SDL_Rect bodyRect = { x,y,30,30 };
	body = App->collision->AddCollider(bodyRect);
	body->colliderType = ENEMY;
	body->addObserver(this);

	bodyRect = {x-5,y,5,5};
	sensor = App->collision->AddCollider(bodyRect);
	sensor->colliderType = SENSOR;
	sensor->addObserver(this);
}

Enemy::~Enemy() {}

void Enemy::Update() {

	if (life <= 0) {
		state = DIE_ENEMY;
	}

	switch (state) {
		
	case WALK_ENEMY:
		Walk();
		break;
	case ATTACK_ENEMY:
		Attack();
		break;
	case MOVE_OTHER_SIDE:
		MoveOtherSide();
		break;
	case ENEMY_IDLE:
		Idle();
		break;
	case DIE_ENEMY:
		Die();
		break;

	default:
		break;
	}
	UpdateCollidersPosition();
}

void Enemy :: MoveOtherSide() 
{
	
	currentAnimation = &animationWalk;
	if (flipHorizontal == true && position.x > targetPositionAttack)
	{
		position.x -= speed;

		if (position.x <= targetPositionAttack) {
			state = WALK_ENEMY;
			return;
		}
	
	}
	else if(flipHorizontal == false && position.x < targetPositionAttack)
	{
		position.x += speed;
		if (position.x >= targetPositionAttack) {
			state = WALK_ENEMY;
			return;
		}
	}
	
}

void Enemy::Walk() 
{
	iPoint playerPosition = App->player->position;
	currentAnimation = &animationWalk;
	if (position.x == playerPosition.x) {
		return;
	}

	if (position.x > playerPosition.x) 
	{
		position.x -= speed;
		flipHorizontal = true;
	}
	else if(position.x < playerPosition.x)
	{
		position.x += speed;
		flipHorizontal = false;
	}


	if (go_down == false) {
		if (position.y > playerPosition.y)
		{
			position.y -= speed;
		}
		else if (position.y < playerPosition.y)
		{
			position.y += speed;
		}
	}
	else {
		if (position.y <= wallPositionTarget) {
			position.y += speed;
			if (position.y >= wallPositionTarget) {
				position.y = wallPositionTarget;
			}
		}
		
		
	}
}

void Enemy::Attack() {

	currentAnimation = &animationAttack;

	if (currentAnimation->Finished()) {
		state = WALK_ENEMY;
		App->player->life-=10;
	}
}

void Enemy::Idle() {
	if (tiempoIdle < 90) { // for testing
		tiempoIdle++;
	}

	if (tiempoIdle >= 90) {
		state = WALK_ENEMY;
		tiempoIdle = 0;
	}
}

void Enemy::UnderAttack() {

}

void Enemy::Die() {
	currentAnimation = &animationDead;
}

void Enemy::CheckLife() {
}

void Enemy::UpdateCollidersPosition() {

	body->rect.x = position.x;
	body->rect.y = position.y;
	if(flipHorizontal == true)
		sensor->rect.x = position.x-5;
	else
		sensor->rect.x = position.x+30;
	sensor->rect.y = position.y;
	
}

void Enemy::onNotify(GameEvent event) {
	if ((state == MOVE_OTHER_SIDE || state == ATTACK_ENEMY) &&  event != ENEMY_DAMAGE)  { //||  state == ATTACK_ENEMY
		return; 
	}
	switch (event) {
	
		case NO_COLLISION:
			go_down = false;
			state = WALK_ENEMY;
			break;

		case PLAYER_COLLISION:
			state = ATTACK_ENEMY;
			break;

		case ENEMY_DAMAGE:
			life -= 50;
			break;

		default:
			break;
	}
}

void Enemy::onNotify(GameEvent event,int position) {

	if (state == MOVE_OTHER_SIDE || state == ATTACK_ENEMY ) { // || state == ATTACK_ENEMY
		return;
	}

	switch (event) {

	case WALL_COLLISION:
		go_down = true;
		wallPositionTarget= position-1;
		break;
	
	case ENEMY_COLLISION:
		if (state != ATTACK_ENEMY && PlayerInYourDirection()) {
			state = MOVE_OTHER_SIDE;
			if(flipHorizontal==true)
				targetPositionAttack = position - 60;
			else
				targetPositionAttack = position + 60;
		}	
		break;
	default:
		break;
	}
}

bool Enemy::PlayerInYourDirection() {
	if (flipHorizontal == false && App->player->position.x > position.x)
	{
		return true;
	}

	if (flipHorizontal == true && App->player->position.x < position.x) {
		return true;
	}

	return false;
}

ModuleEnemies::ModuleEnemies(bool active):Module(active) {}


ModuleEnemies::~ModuleEnemies(){}


update_status ModuleEnemies::PreUpdate() 
{
	for (list<Enemy*>::iterator it = enemies.begin(); it != enemies.end();)
	{
		if ((*it)->to_delete == true)
		{
			RELEASE(*it);
			it = enemies.erase(it);
		}
		else
			++it;
	}
	
	return UPDATE_CONTINUE;
}

update_status ModuleEnemies::Update() 
{
	bool twoMovingSides = false;

	for (list<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); ++it) 
	{
		(*it)->Update();
	
		App->renderer->Blit(graphics,(*it)->position.x, (*it)->position.y, &((*it)->currentAnimation->GetCurrentFrame()), 1.0f, (*it)->flipHorizontal);

	}


	
	if (debug == true) 
	{
		DebugDraw();
	}

	return UPDATE_CONTINUE;
}

bool ModuleEnemies::CleanUp()
{
	for (list<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); ++it)
		RELEASE(*it);

	enemies.clear();

	return true;
}

void ModuleEnemies::DebugDraw()
{
	for (std::list<Enemy*>::iterator it = enemies.begin(); it != enemies.end(); ++it)
	{
		//App->renderer->DrawQuad((*it)->rect, 255, 0, 0, 80);
	}
}

bool ModuleEnemies::Start() 
{	
	graphics = App->textures->Load("rtype/BattletoadSprites/BT_PsykoPig.gif");
	return true;
}

Enemy* ModuleEnemies::AddEnemy(int x , int y) 
{
	Enemy *enemie = new Enemy(x,y);
	enemies.push_back(enemie);
	return enemie;
}