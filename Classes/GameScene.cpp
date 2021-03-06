
#include "GameScene.h"
#include "GameOverScene.h"
#include "Agent.h"
#include "AgentPool.h"
#include "Config.h"
#include "GameLogger.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

Scene* GameScene::createScene()
{
	auto scene = Scene::createWithPhysics();
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	scene->getPhysicsWorld()->setGravity(Vec2(0, 0));

	auto layer = GameScene::create();
	layer->SetPhysicsWorld(scene->getPhysicsWorld());

	scene->addChild(layer);
	return scene;
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
    //////////////////////////////
    // super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "btn_close_normal.png",
                                           "btn_close_down.png",
                                           CC_CALLBACK_1(GameScene::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
		log_trace("'btn_close_normal.png' or 'btn_close_selected.png' is missing.");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

	_specialMenuItem = MenuItemImage::create(
											"btn_special_normal.png",
											"btn_special_down.png",
											"btn_special_disable.png",
											CC_CALLBACK_1(GameScene::menuSpecialCallback, this));

	if (_specialMenuItem == nullptr ||
		_specialMenuItem->getContentSize().width <= 0 ||
		_specialMenuItem->getContentSize().height <= 0)
	{
		log_trace("'btn_special_normal.png' or 'btn_special_selected.png' is missing.");
	}
	else
	{
		float x = origin.x + visibleSize.width - _specialMenuItem->getContentSize().width / 2;
		float y = origin.y + visibleSize.height - _specialMenuItem->getContentSize().height;
		_specialMenuItem->setPosition(Vec2(x, y));
	}

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, _specialMenuItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 10);

    // add a label shows game title
    // create and initialize a label
    auto titleLabel = Label::createWithTTF("Space Invaders", "fonts/Marker Felt.ttf", 24);
    if (titleLabel == nullptr)
    {
		log_trace("'fonts/Marker Felt.ttf' is missing.");
    }
    else
    {
        // position the label on the top center of the screen
        titleLabel->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - titleLabel->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(titleLabel, 10);
    }

	auto dispLabel = Label::createWithTTF("Score : ", "fonts/Marker Felt.ttf", 24);
	if (dispLabel == nullptr)
	{
		log_trace("'fonts/Marker Felt.ttf' is missing.");
	}
	else
	{
		// position the label on the top left of the screen
		dispLabel->setPosition(Vec2(origin.x + 50, origin.y + visibleSize.height - 30));

		// add the label as a child to this layer
		this->addChild(dispLabel, 10);
	}

	_scoreLabel = Label::createWithTTF("0", "fonts/Marker Felt.ttf", 24);
	if (_scoreLabel == nullptr)
	{
		log_trace("'fonts/Marker Felt.ttf' is missing.");
	}
	else
	{
		// position the label on the center of the screen
		_scoreLabel->setPosition(Vec2(
							dispLabel->getPositionX() + dispLabel->getContentSize().width + 5.f, 
							dispLabel->getPositionY()));

		// add the label as a child to this layer
		this->addChild(_scoreLabel, 10);
	}

	// show progress bar
	auto frame = Sprite::create("prog_frame.png");
	frame->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	frame->setScaleX(0.5f);
	frame->setPosition(Point(dispLabel->getPositionX() - 35.f, dispLabel->getPositionY() - 20.f));
	this->addChild(frame, 10);

	_progBarSprite = Sprite::create("prog_bar.png");
	_progBarSprite->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	_progBarSprite->setContentSize(Size(0.f, 24.f));
	_progBarSprite->setPosition(Point(frame->getPositionX() + 4.f, frame->getPositionY() - 3.f));
	this->addChild(_progBarSprite, 10);

    // add background
    auto sprite = Sprite::create("background.png");
    if (sprite == nullptr) {
		log_trace("'background.png' is missing.");
    }
    else
    {
        // position the sprite on the center of the screen
		sprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }

	// add space ship
	auto selfSprite = AgentPool::getInstance()->getAgent(AGENT_SELF_TAG);
	if (selfSprite == nullptr) {
		log_trace("self ship creation failed.");
	}
	else {
		selfSprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, SHIP_HEIGHT * 3 + origin.y));
		this->addChild(selfSprite, 1);
	}
	
	_isTouchBegan = false;
	_isSpecialMode = false;
	_killCntInNormal = 0;
	_level = 0;
	_elapsed = 0.f;
	_rocketElapsed = 0.f;
	_specialMenuItem->setEnabled(false);

	// add event handlers
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);

	listener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onContactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

	// define schedulers
	//generateEnemies();
	/*schedule(schedule_selector(GameScene::generateEnemies), 30.f);*/
	//schedule(schedule_selector(GameScene::generateRockets), NORMAL_SPEED);
	scheduleOnce(schedule_selector(GameScene::enableSpecialMenuItem), 20.f);
	scheduleUpdate();
    return true;
}

bool GameScene::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
	_isTouchBegan = true;
	_touchXPos = touch->getLocation().x;
	return true;
}

bool GameScene::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
	_touchXPos = touch->getLocation().x;
	return true;
}

bool GameScene::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event)
{
	_isTouchBegan = false;
	return true;
}

// Handle collision event
bool GameScene::onContactBegin(cocos2d::PhysicsContact &contact)
{
	Node* shapeA = contact.getShapeA()->getBody()->getNode();
	Node* shapeB = contact.getShapeB()->getBody()->getNode();

	if ((shapeA->getTag() == AGENT_ROCKET_TAG 
			&& shapeB->getTag() == AGENT_ENEMY_TAG)
		|| (shapeB->getTag() == AGENT_ROCKET_TAG
			&& shapeA->getTag() == AGENT_ENEMY_TAG))
	{	// rocket collide with enemy
		this->removeChild(shapeA);
		AgentPool::getInstance()->returnAgent((AgentSprite*)shapeA);
		this->removeChild(shapeB);
		AgentPool::getInstance()->returnAgent((AgentSprite*)shapeB);

		increaseScore();

		if (!_isSpecialMode)
		{
			_killCntInNormal++;
			_progBarSprite->setContentSize(Size(168.07f * _killCntInNormal / 10, 24.f));
			if (_killCntInNormal == 10)
			{	//	enter to special mode
				convertState(true);
			}
		}
	}
	else if (shapeA->getTag() == AGENT_SELF_TAG
		|| shapeB->getTag() == AGENT_SELF_TAG)
	{	// crashed
		this->stopAllActions();
		auto scene = GameOverScene::createScene();
		Director::getInstance()->replaceScene(TransitionFlipX::create(0.1f, scene));
		return false;
	}

	return true;
}

void GameScene::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();
}

void GameScene::menuSpecialCallback(Ref* pSender)
{
	cocos2d::Vector<Node*> children = this->getChildren();
	for each (Node* child in children)
	{
		if (child->getTag() != AGENT_ENEMY_TAG 
						&& child->getTag() != AGENT_ROCKET_TAG)
		{
			continue;
		}

		this->removeChild(child);
		AgentPool::getInstance()->returnAgent((AgentSprite*)child);
		increaseScore();
	}

	((MenuItemImage*)pSender)->setEnabled(false);
	scheduleOnce(schedule_selector(GameScene::enableSpecialMenuItem), 20.f);
}

/**
* generate rockets frequently.
*/
void GameScene::generateRockets(float dt)
{
	auto rocketSprite = AgentPool::getInstance()->getAgent(AGENT_ROCKET_TAG);
	if (rocketSprite == nullptr) {
		log_trace("rocket creation failed.");
	}
	else {
		auto selfNode = getChildByTag(AGENT_SELF_TAG);
		if (selfNode != nullptr)
		{
			rocketSprite->setPositionX(selfNode->getPositionX()); 
			rocketSprite->setPositionY(selfNode->getPositionY() + selfNode->getContentSize().height);
			this->addChild(rocketSprite, 1);
		}
	}
}

void GameScene::increaseScore()
{
	int curscore = std::atoi(_scoreLabel->getString().c_str());
	curscore += 20;
	_scoreLabel->setString(StringUtils::format("%d", curscore));
}

void GameScene::showCrashState()
{
	unscheduleUpdate();
	_specialMenuItem->setEnabled(false);
	this->stopAllActions();

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto stateLabel = Label::createWithTTF("Crashed. Game Exit!", "fonts/Marker Felt.ttf", 32);
	if (stateLabel == nullptr)
	{
		log_trace("'fonts/Marker Felt.ttf' is missing.");
	}
	else
	{
		// position the label on the center of the screen
		stateLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
			origin.y + (visibleSize.height - stateLabel->getContentSize().height) / 2));

		// add the label as a child to this layer
		this->addChild(stateLabel, 10);
	}


}

void GameScene::enableSpecialMenuItem(float dt)
{
	_specialMenuItem->setEnabled(true);
}

void GameScene::convertState(bool bSpecialMode)
{
	_killCntInNormal = 0;
	_isSpecialMode = bSpecialMode;

	//unschedule(schedule_selector(GameScene::generateRockets));
	//schedule(schedule_selector(GameScene::generateRockets), _isSpecialMode ? SPECIAL_SPEED : NORMAL_SPEED);
	
	if (!_isSpecialMode)
	{
		_progBarSprite->setContentSize(Size(0.f, 24.f));
	}
	else
	{	// convert to normal mode after delay time.
		auto delayAction = DelayTime::create(SPECIAL_DELAY);
		auto convertAction = CallFunc::create(CC_CALLBACK_0(GameScene::convertState, this, false));
		this->runAction(Sequence::createWithTwoActions(delayAction, convertAction));
	}
}

/**
* generate enemies frequently.
*/
void GameScene::generateEnemies(float dt)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

#define	SHIP_OCCU_WIDTH		SHIP_WIDTH * 3.f / 2.f
#define	SHIP_OCCU_HEIGHT	SHIP_HEIGHT * 3.f / 2.f
#define MARGIN_X			SHIP_WIDTH / 2.f

	float occu_width = SHIP_OCCU_WIDTH;
	float delta_val = visibleSize.width - MARGIN_X * 2.f;
	int col_cnt = delta_val / occu_width + 1;
	int row_cnt = std::min(++_level + 1, 5);
	_level = row_cnt - 1;

	// arrange items in rows (2, 3, 4 and 5 rows of enemies, 5 is max)
	for (int j = 0; j < row_cnt; j++)
	{
		for (int i = 0; i < col_cnt; i++)
		{
			auto enemySprite = AgentPool::getInstance()->getAgent(AGENT_ENEMY_TAG);
			if (enemySprite == nullptr) {
				log_trace("enemy ship creation failed.");
			}
			else {
				enemySprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
				enemySprite->setPosition(Vec2(origin.x + MARGIN_X + SHIP_OCCU_WIDTH * i, 
												visibleSize.height + origin.y + SHIP_OCCU_HEIGHT * j));
				this->addChild(enemySprite, 1);
			}
		}
	}
}

/**
* Update function which is called every dt
*/
void GameScene::update(float dt)
{
	_rocketElapsed += dt;
	if ((_isSpecialMode && _rocketElapsed >= SPECIAL_SPEED)
		|| (!_isSpecialMode && _rocketElapsed >= NORMAL_SPEED))
	{
		_rocketElapsed = 0.f;
		generateRockets();
	}

	/*_elapsed += dt;
	if (_elapsed < 0.1f)
	{
		return;
	}

	_elapsed = 0.f;*/

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	bool bEnemyExist = false;

	cocos2d::Vector<Node*> children = this->getChildren();
	for each (Node* child in children)
	{
		// no update for other agents.
		if (child->getTag() != AGENT_SELF_TAG 
					&& child->getTag() != AGENT_ENEMY_TAG 
					&& child->getTag() != AGENT_ROCKET_TAG)
		{
			continue;
		}

		AgentSprite* agent = (AgentSprite*)child;
		if (agent->getTag() == AGENT_ROCKET_TAG)
		{	// rocket move upwards
			agent->setPositionY(agent->getPositionY() + ROCKET_DELTA_Y);
			// If rocket has gone out of screen, remove from parent
			if (agent->getPositionY() > origin.y + visibleSize.height + 40.f) 
			{
				this->removeChild(agent);
				AgentPool::getInstance()->returnAgent(agent);
			}
		}
		else if (agent->getTag() == AGENT_SELF_TAG)
		{	// for self ship, if current is no touch state, continue
			if (!_isTouchBegan)
			{
				continue;
			}
			float curx = agent->getPositionX();
			if (curx + SHIP_DELTA_X <= _touchXPos)
			{
				agent->setPositionX(curx + SHIP_DELTA_X);
			}
			else if (curx - SHIP_DELTA_X >= _touchXPos)
			{
				agent->setPositionX(curx - SHIP_DELTA_X);
			}
		}
		else if (agent->getTag() == AGENT_ENEMY_TAG)
		{	// enemies are coming down.
			agent->setPositionY(agent->getPositionY() - SHIP_DELTA_Y);
			// If enemy ship has gone out of screen, remove from parent
			if (agent->getPositionY() < origin.y - 40.f)
			{
				this->removeChild(agent);
				AgentPool::getInstance()->returnAgent(agent);
			}
			else 
			{
				bEnemyExist = true;
			}
		}
	}

	// if no enemies, then to appear enemies
	if (!bEnemyExist)
	{
		generateEnemies();
	}
}

