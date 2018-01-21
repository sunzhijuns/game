#include <audio/include/SimpleAudioEngine.h>
#include "AppDelegate.h"
#include "HelpLayer.h"
#include "AppMacros.h"
#include "cocos2d.h"

using namespace cocos2d;
using namespace std;

//实现HelpLayer类中的init方法，初始化布景
bool HelpLayer::init()
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
    Sprite* backSprite = Sprite::create(help_PATH+string("backgroundh.png"));
    //设置精灵对象的位置
    backSprite->setPosition(Point(400,240));
	//将精灵添加到布景中
	this->addChild(backSprite,BACKGROUND_LEVEL_CGQ);

	//创建“上一幅帮助图片”精灵
	MenuItemImage *leftItem = MenuItemImage::create
	(
		help_PATH+string("left_up.png"),
		help_PATH+string("left_down.png"),
		CC_CALLBACK_1(HelpLayer::menuCallBack0, this)
	);
	leftItem->setScale(1.5);
	//设置精灵菜单按钮的位置
	leftItem->setPosition(Point(330,450));

	//创建“下一幅帮助图片”精灵
	MenuItemImage *rightItem = MenuItemImage::create
	(
		help_PATH+string("right_up.png"),
		help_PATH+string("right_down.png"),
		CC_CALLBACK_1(HelpLayer::menuCallBack1, this)
	);
	rightItem->setScale(1.5);
	//设置精灵菜单按钮的位置.
	rightItem->setPosition(Point(330,30));

	//创建“返回”精灵
	MenuItemImage *fanhuiItem = MenuItemImage::create
	(
		help_PATH+string("fanhui.png"),
		help_PATH+string("fanhui.png"),
		CC_CALLBACK_1(HelpLayer::menuCallBack2, this)
	);
	//设置精灵菜单按钮的位置.
	fanhuiItem->setPosition(Point(40,420));

	pMenu = Menu::create(leftItem,rightItem,fanhuiItem,NULL);
	//设置菜单的位置
	pMenu->setPosition(Point(0,0));
	//将菜单添加到布景中
	this->addChild(pMenu,GAME_LEVEL_CGQ);
	initIntroPic();
    return true;
}
void HelpLayer::initIntroPic()
{
	for(int i = 0;i<9;i++)
	{
		//通过createHelp获取对应图片的路径
		help[i] = Sprite::create(StringUtils::format("help/help%d.png",i));
		help[i]->setPosition(Point(440,245+i*480));
		this->addChild(help[i],GAME_LEVEL_CGQ);
	}
}

//向右箭头
void HelpLayer::menuCallBack0(Object* pSende)
{
	//播放背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
	(
		"sound/sf_button_press.mp3"
	);
	for(int i= 0 ;i<9;i++)
	{
		help[i] ->setPosition(Point(400,-480));
	}
	currPicIndex ++;
	if(currPicIndex == 9)
	{
		currPicIndex = 0;
	}
	help[currPicIndex%9] ->setPosition(Point(440,245));
}

//向左箭头
void HelpLayer::menuCallBack1(Object* pSende)
{
	//播放背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
	(
		"sound/sf_button_press.mp3"
	);
	for(int i= 0 ;i<9;i++)
	{
		help[i] ->setPosition(Point(400,-480));
	}
	currPicIndex --;
	if(currPicIndex == -1)
	{
		currPicIndex = 8;
	}
	help[currPicIndex%9] ->setPosition(Point(440,245));
}

//向左箭头
void HelpLayer::menuCallBack2(Object* pSende)
{
	//播放音效
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
	//返回主界面
	man->toBeginLayer();
}

