#include "GameSceneManager.h"
#include "MainLayer.h"
#include "SetLayer.h"
#include "BirdLayer.h"
#include "FruitLayer.h"
#include "GameLayer.h"
#include "RankBirdLayer.h"
#include "RankFruitLayer.h"
#include "AboutLayer.h"
#include "HelpLayer.h"

using namespace cocos2d;

//创建主界面场景
void GameSceneManager::createMainScene()
{
	mainScene = Scene::create();
	MainLayer* layer = MainLayer::create();
	layer->sceneManager = this;
	mainScene->addChild(layer);
}
//转到主界面
void GameSceneManager::goToMainScene()
{
	Director::getInstance()->setDepthTest(true);
	mainScene = Scene::create();
	MainLayer* layer = MainLayer::create();
	mainScene->addChild(layer);
	layer->sceneManager = this;
	auto ss = TransitionFade::create(1, mainScene);
	//切换场景
	Director::getInstance()->replaceScene(ss);
}
//转到撞死鸟游戏界面
void GameSceneManager::goToFlappyScene(int i)
{
	Director::getInstance()->setDepthTest(true);
	birdScene = Scene::create();
	BirdLayer* layer = BirdLayer::create();
	birdScene->addChild(layer);
	layer->sceneManager = this;
	if(i==0)
	{
		auto ss = TransitionFadeTR::create(1, birdScene);
		//切换场景
		Director::getInstance()->replaceScene(ss);
	}
	if(i==1)
	{
		auto ss = TransitionFade::create(1, birdScene);
		//切换场景
		Director::getInstance()->replaceScene(ss);
	}
}
//转到夹水果游戏界面
void GameSceneManager::goToSquishyScene(int i)
{
	Director::getInstance()->setDepthTest(true);
	fruitScene = Scene::create();
	FruitLayer* layer = FruitLayer::create();
	fruitScene->addChild(layer);
	layer->sceneManager = this;
	if(i==0)
	{
		auto ss = TransitionFadeTR::create(1, fruitScene);
		//切换场景
		Director::getInstance()->replaceScene(ss);
	}
	if(i==1)
	{
		auto ss = TransitionFade::create(1, fruitScene);
		//切换场景
		Director::getInstance()->replaceScene(ss);
	}
}
//撞到游戏选择界面
void GameSceneManager::goToGameScene()
{
	Director::getInstance()->setDepthTest(true);
	gameScene = Scene::create();
	GameLayer* layer = GameLayer::create();
	gameScene->addChild(layer);
	layer->sceneManager = this;
	auto ss = TransitionPageTurn::create(1, gameScene, false);
	//切换场景
	Director::getInstance()->replaceScene(ss);
}
//传到设置界面
void GameSceneManager::goToSetScene()
{
	Director::getInstance()->setDepthTest(true);
	musicScene = Scene::create();
	SetLayer* layer = SetLayer::create();
	musicScene->addChild(layer);
	layer->sceneManager = this;
	auto ss = TransitionPageTurn::create(1, musicScene, false);
	//切换场景
	Director::getInstance()->replaceScene(ss);
}
//
void GameSceneManager::goToRankBirdScene()
{
	Director::getInstance()->setDepthTest(true);
	rankScene = Scene::create();
	RankBirdLayer* layer = RankBirdLayer::create();
	rankScene->addChild(layer);
	layer->sceneManager = this;
	auto ss = TransitionPageTurn::create(1, rankScene, false);
	//切换场景
	Director::getInstance()->replaceScene(ss);
}
void GameSceneManager::goToRankFruitScene()
{
	Director::getInstance()->setDepthTest(true);
	rankScene = Scene::create();
	RankFruitLayer* layer = RankFruitLayer::create();
	rankScene->addChild(layer);
	layer->sceneManager = this;
	auto ss = TransitionPageTurn::create(1, rankScene, false);
	//切换场景
	Director::getInstance()->replaceScene(ss);
}
void GameSceneManager::goToAboutScene()
{
	Director::getInstance()->setDepthTest(true);
	aboutScene = Scene::create();
	AboutLayer* layer = AboutLayer::create();
	aboutScene->addChild(layer);
	layer->sceneManager = this;
	auto ss = TransitionPageTurn::create(1, aboutScene, false);
	//切换场景
	Director::getInstance()->replaceScene(ss);
}
void GameSceneManager::goToHelpScene()
{
	Director::getInstance()->setDepthTest(true);
	helpScene = Scene::create();
	HelpLayer* layer = HelpLayer::create();
	helpScene->addChild(layer);
	layer->sceneManager = this;
	auto ss = TransitionPageTurn::create(1, helpScene, false);
	//切换场景
	Director::getInstance()->replaceScene(ss);
}
