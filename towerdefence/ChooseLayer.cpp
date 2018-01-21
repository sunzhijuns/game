#include "ChooseLayer.h"
#include "AppDelegate.h"
#include "DialogLayer.h"
#include <string>
#include <audio/include/SimpleAudioEngine.h>
#include "AppMacros.h"

using namespace cocos2d;
using namespace std;

int ChooseLayer::modeLevel = 0;

//实现ChooseLayer类中的init方法，初始化布景
bool ChooseLayer::init()
{
    //调用父类的初始化
    if ( !Layer::init() )
    {
        return false;
    }

    //获取可见区域尺寸
    Size visibleSize = Director::sharedDirector()->getVisibleSize();
    //获取可见区域原点坐标
    Point origin = Director::sharedDirector()->getVisibleOrigin();
	//加载音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_button_press.mp3"
	);
    //创建一个精灵对象，包含背景图片
	Sprite* bSprite = Sprite::create(choose_PATH+string("background.png"));
	//设置精灵对象的位置
	bSprite->setPosition(Point(400,240));
	//将精灵添加到布景中
	this->addChild(bSprite,BACKGROUND_LEVEL_CGQ);

	//创建关卡1的精灵
	MenuItemImage *oneGuan = MenuItemImage::create
	(
		choose_PATH+string("guanka1.png"),		//平时的图片
		choose_PATH+string("guanka11.png"),		//选中时的图片
		CC_CALLBACK_1(ChooseLayer::oneMode, this)
	);
	//设置位置
	oneGuan->setPosition(Point(270,240));

	//创建关卡2的精灵
	MenuItemImage *twoGuan = MenuItemImage::create
	(
		choose_PATH+string("guanka2.png"),		//平时的图片
		choose_PATH+string("guanka22.png"),		//选中时的图片
		CC_CALLBACK_1(ChooseLayer::twoMode, this)
	);
	twoGuan->setPosition(Point(370,240));

	//创建关卡3的精灵
	MenuItemImage *threeGuan = MenuItemImage::create
	(
		choose_PATH+string("guanka3.png"),		//平时的图片
		choose_PATH+string("guanka33.png"),		//选中时的图片
		CC_CALLBACK_1(ChooseLayer::threeMode, this)
	);
	threeGuan->setPosition(Point(470,240));

	//创建关卡4的精灵
	MenuItemImage *fourGuan = MenuItemImage::create
	(
		choose_PATH+string("guanka4.png"),     //平时的图片
		choose_PATH+string("guanka44.png"),   //选中时的图片
		CC_CALLBACK_1(ChooseLayer::fourMode, this)
	);
	fourGuan->setPosition(Point(570,240));

	//创建关卡5的精灵
	MenuItemImage *fiveGuan = MenuItemImage::create
	(
		choose_PATH+string("guanka5.png"),		//平时的图片
		choose_PATH+string("guanka55.png"),		//选中时的图片
		CC_CALLBACK_1(ChooseLayer::fiveMode, this)
	);
	//设置位置
	fiveGuan->setPosition(Point(670,240));

	//创建返回按钮精灵
	MenuItemImage *fanHui = MenuItemImage::create
	(
		choose_PATH+string("fanhui.png"),		//平时的图片
		choose_PATH+string("fanhui.png"),		//选中时的图片
		CC_CALLBACK_1(ChooseLayer::sixMode, this)
	);
	//设置位置
	fanHui->setPosition(Point(750,420));

	//创建菜单对象
	pMenu = Menu::create(oneGuan,twoGuan,threeGuan,fourGuan,fiveGuan,fanHui,NULL);
	//设置菜单位置
	pMenu->setPosition(Point(0,0));
	//将菜单添加到布景中
	this->addChild(pMenu,GAME_LEVEL_CGQ);

	return true;
}

void ChooseLayer::oneMode(Object* pSender)
{
	ChooseLayer::modeLevel = 0;
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
	//关卡1地图
	man->toGameLayer();
	if(DialogLayer::isMusic)
	{
		//播放背景音乐
		CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
		(
			"sound/bg_music.mp3",
			true
		);
	}
}
void ChooseLayer::twoMode(Object* pSender)
{
	ChooseLayer::modeLevel = 1;
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
	//关卡2地图
	man->toGameLayer();
	if(DialogLayer::isMusic)
	{
		//播放背景音乐
		CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
		(
			"sound/bg_music.mp3",
			true
		);
	}
}
void ChooseLayer::threeMode(Object* pSender)
{
	ChooseLayer::modeLevel = 2;
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
	//关卡3地图
	man->toGameLayer();
	if(DialogLayer::isMusic)
	{
		//播放背景音乐
		CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
		(
			"sound/bg_music.mp3",
			true
		);
	}
}
void ChooseLayer::fourMode(Object* pSender)
{
	ChooseLayer::modeLevel = 3;
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
	//关卡4地图
	man->toGameLayer();
	if(DialogLayer::isMusic)
	{
		//播放背景音乐
		CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
		(
			"sound/bg_music.mp3",
			true
		);
	}
}
void ChooseLayer::fiveMode(Object* pSender)
{
	ChooseLayer::modeLevel = 4;
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
	//关卡5地图
	man->toGameLayer();
	if(DialogLayer::isMusic)
	{
		//播放背景音乐
		CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
		(
			"sound/bg_music.mp3",
			true
		);
	}
}

void ChooseLayer::sixMode(Object* pSender)
{
	//返回主界面
	man->toBeginLayer();
	//添加按键时的音效
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
}

