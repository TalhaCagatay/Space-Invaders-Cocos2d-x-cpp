
#include "GameOverScene.h"
#include "GameScene.h"
#include "GameLogger.h"

USING_NS_CC;

Scene* GameOverScene::createScene()
{
	return GameOverScene::create();
}

// on "init" you need to initialize your instance
bool GameOverScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto retryItem = MenuItemImage::create(
										"btn_retry_normal.png",
										"btn_retry_down.png",
										CC_CALLBACK_1(GameOverScene::menuRetryCallback, this));

	if (retryItem == nullptr ||
		retryItem->getContentSize().width <= 0 ||
		retryItem->getContentSize().height <= 0)
	{
		log_trace("'btn_retry_normal.png' or 'btn_retry_down.png' is missing.");
	}
	else
	{
		retryItem->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
		float x = origin.x + visibleSize.width / 2 - 30.f;
		float y = origin.y + retryItem->getContentSize().height * 2;
		retryItem->setPosition(Vec2(x, y));
	}

    auto exitItem = MenuItemImage::create(
                                           "btn_exit_normal.png",
                                           "btn_exit_down.png",
                                           CC_CALLBACK_1(GameOverScene::menuExitCallback, this));

    if (exitItem == nullptr ||
        exitItem->getContentSize().width <= 0 ||
        exitItem->getContentSize().height <= 0)
    {
		log_trace("'btn_exit_normal.png' or 'btn_exit_down.png' is missing.");
    }
    else
    {
		exitItem->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
        float x = origin.x + visibleSize.width / 2 + 30.f;
        float y = origin.y + exitItem->getContentSize().height * 2;
        exitItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(exitItem, retryItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 10);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows game title
    // create and initialize a label
    auto titleLabel = Label::createWithTTF("Space Invaders", "fonts/Marker Felt.ttf", 24);
    if (titleLabel == nullptr)
    {
		log_trace("'fonts/Marker Felt.ttf' is missing.");
    }
    else
    {
        // position the label on the center of the screen
        titleLabel->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - titleLabel->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(titleLabel, 10);
    }

	auto stateLabel = Label::createWithTTF("Crashed. Press Retry or Exit.", "fonts/Marker Felt.ttf", 32);
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

    return true;
}

void GameOverScene::menuExitCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();
}

void GameOverScene::menuRetryCallback(Ref* pSender)
{
	auto scene = GameScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.f, scene));
}

