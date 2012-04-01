#include "Gameplay.h"
#include "Input.h"

#define PTM_RATIO 32

Gameplay::Gameplay() {
}

Gameplay::~Gameplay() {
	unscheduleUpdate();
}

CCScene* Gameplay::scene()
{
	CCScene * scene = NULL;
	do 
	{
		// 'scene' is an autorelease object
		scene = CCScene::node();
		CC_BREAK_IF(! scene);

		// 'layer' is an autorelease object
		Gameplay *layer = Gameplay::node();
		CC_BREAK_IF(! layer);

		// add layer as a child to scene
		scene->addChild(layer);
	} while (0);

	// return the scene
	return scene;
}

bool Gameplay::init() 
{
	setIsTouchEnabled(true);
	CCSize size = CCDirector::sharedDirector()->getWinSize();

	// Background
	mBackground = CCSprite::spriteWithFile("space.png");
	mBackground->setAnchorPoint(ccp(0,0));
	mBackground->setPosition(ccp(0,0));
	//addChild(mBackground);

	world = CCNode::node();
	world->setContentSize(size);
	world->setAnchorPoint(ccp(0.5, 0.5));
	world->setPosition(ccp(size.width / 2, size.height / 2));
	addChild(world);

	player = new Player();
	player->setPosition(ccp(size.width / 2, size.height / 2));
	world->addChild(player, 1);

	// setup sun
	sun = new Sun();
	sun->setPosition(ccp(-sun->getContentSize().width / 2 + 400, world->getContentSize().height / 2));
	world->addChild(sun);

	// setup world rotation around sun
	float sunAnchorPositionX = (sun->getPositionX())  / world->getContentSize().width;
	world->setAnchorPoint(ccp(sunAnchorPositionX, 0.5));
	world->setPosition(ccp(world->getPositionX() - (0.5 + fabs(sunAnchorPositionX)) * world->getContentSize().width, world->getPositionY()));
	


	initPhysicalWorld();
	scheduleUpdate();
	CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
	createPlayer(screenSize.width/2, screenSize.height/2);
	addPlanet("planet_01.png", ccp(500, 100));
	addPlanet("planet_02.png", ccp(500, 200));
	addPlanet("planet_03.png", ccp(500, 300));

	return true;
}

void Gameplay::initPhysicalWorld()
{
	// Create Box2d world
	b2Vec2 gravity = b2Vec2(0.0f, 0.0f);
	bool doSleep = false;
	//mWorld = new b2World(gravity, doSleep);
	mWorld = new b2World(gravity);
	//mWorld->SetContinuousPhysics(true);

	// Debug Draw functions
	GLESDebugDraw* m_debugDraw = new GLESDebugDraw( PTM_RATIO );
	mWorld->SetDebugDraw(m_debugDraw);

	uint32 flags = 0;
	flags += b2Draw::e_shapeBit;
	flags += b2Draw::e_pairBit;
	m_debugDraw->SetFlags(flags);	
}

void Gameplay::updatePhysic( ccTime dt )
{
	int32 velocityIterations = 8;
	int32 positionIterations = 1;

	mWorld->Step(dt, velocityIterations, positionIterations);
	/*
	for (b2Body* b = mWorld->GetBodyList(); b; b = b->GetNext())
	{
		b2Body* ground = planet->GetBody();
		b2CircleShape* circle = (b2CircleShape*)planet->GetShape();
		// Get position of our "Planet"
		b2Vec2 center = ground->GetWorldPoint(circle->m_p);
		// Get position of our current body in the iteration
		b2Vec2 position = b->GetPosition();
		// Get the distance between the two objects.	
		b2Vec2 d = center - position;
		// The further away the objects are, the weaker the gravitational force is
		float force = 250.0f / d.LengthSquared(); // 150 can be changed to adjust the amount of force
		d.Normalize();
		b2Vec2 F = force * d;
		// Finally apply a force on the body in the direction of the "Planet"
		b->ApplyForce(F, position);

		if (b->GetUserData() != NULL) {
			CCSprite *myActor = (CCSprite*)b->GetUserData();
			myActor.position = CGPointMake( b->GetPosition().x * PTM_RATIO, b->GetPosition().y * PTM_RATIO);
			myActor.rotation = -1 * CC_RADIANS_TO_DEGREES(b->GetAngle());
		}	
	}*/
}


void Gameplay::update(ccTime dt) {
Input::instance()->update();
	if(Input::instance()->keyDown(VK_UP)) {
		player->setPositionY(player->getPositionY() + 3);
	}

	if(Input::instance()->keyDown(VK_DOWN)) {
		player->setPositionY(player->getPositionY() - 3);
	}

	CCPoint sub = ccpSub(player->getPosition(), sun->getPosition());
	float angle =  CC_RADIANS_TO_DEGREES(ccpToAngle(sub));
	world->setRotation(angle);

	updatePhysic(dt);
}

void Gameplay::createPlayer(float posx, float posy)
{
	// CREATE SPRITE
	CCPoint position = ccp(posx, posy);
	mPlayer = CCSprite::spriteWithFile("CloseNormal.png");
	mPlayer->setPosition(ccp(position.x, position.y));
	addChild(mPlayer);

	// PHYSICAL REPRESENTATION
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;

	bodyDef.position.Set(position.x/PTM_RATIO, position.y/PTM_RATIO);
	bodyDef.userData = mPlayer;
	b2Body *body = mWorld->CreateBody(&bodyDef);

	// Define another box shape for our dynamic body.
	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(.5f, .5f);//These are mid points for our 1m box

	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;	
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	body->CreateFixture(&fixtureDef);
}

void Gameplay::addPlanet( std::string planetSpriteName, CCPoint position )
{
	Planet* planet = new Planet(planetSpriteName);
	planet->setPos(position);
	world->addChild(planet->getSprite());

	mPlanets.push_back(planet);

	// Physical representation

	b2BodyDef planetBodyDef;
	planetBodyDef.position.Set(position.x/PTM_RATIO, position.y/PTM_RATIO);
	planetBodyDef.userData = mPlayer;
	b2Body* planetBody = mWorld->CreateBody(&planetBodyDef);

	b2CircleShape shape;
	shape.m_radius = 1.0f;
	shape.m_p.Set(8.0f, 8.0f);
	b2FixtureDef fd;
	fd.shape = &shape;
	b2Fixture* planetFixture = planetBody->CreateFixture(&fd);

	planet->mPlanetBody = planetBody;
	planet->mPlanetFixture = planetFixture;
}

void Gameplay::clearLevel()
{
	for(int i = 0; i < mPlanets.size(); ++i) {
		if(mPlanets[i]) {
			removeChild(mPlanets[i]->getSprite(), true);
			delete mPlanets[i];
			mPlanets[i] = NULL;
		}
	}
	mPlanets.clear();
}

void Gameplay::draw()
{
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	mWorld->DrawDebugData();

	// restore default GL states
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}


