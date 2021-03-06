#include "SetLayer.h"
#include "SimpleAudioEngine.h"
#include "MainLayer.h"
#include <time.h>

using namespace cocos2d;
using namespace std;
using namespace ui;

bool SetLayer::init()
{
	//调用父类的初始化
	if ( !CCLayer::init() )
	{
		return false;
	}
	//获取可见区域尺寸
	Size visibleSize = Director::getInstance()->getVisibleSize();
	//获取可见区域原点坐标
	Point origin = Director::getInstance()->getVisibleOrigin();
	//创建背景
	Sprite* backGround = Sprite::create("pic/morning.png");
	//设置锚点
	backGround->setAnchorPoint(Point(0, 0));
	//设置精灵对象的位置
	backGround->setPosition(Point(origin.x,origin.y + visibleSize.height - backGround->getContentSize().height));
	//将精灵添加到布景中
	this->addChild(backGround, 0);

	//创建地面精灵
	Sprite* floor1 = Sprite::create("pic/floor.png");
	//设置锚点
	floor1->setAnchorPoint(Point(0, 0));
	//设置精灵对象位置
	floor1->setPosition(Point(origin.x, origin.y));
	this->addChild(floor1);
	floor1->runAction(RepeatForever::create(
		Sequence::create(
				MoveTo::create(0.5, Point(-120, 0)),
				MoveTo::create(0, Point(0, 0)),
				NULL
	)));

	Sprite* pauseBack = Sprite::create("pic/setBack.png");
	pauseBack->setPosition(Point(270, 500));
	this->addChild(pauseBack, 10);

	//音乐按钮
	Sprite* music = Sprite::create("pic/music.png");
	pauseBack->addChild(music, 1);
	music->setPosition(Point(100, 220));

	CheckBox* checkMusic = CheckBox::create(
			"button/sound_on.png",
			"button/sound_off.png",
			"button/sound_off.png",
			"button/sound_stop.png",
			"button/sound_stop.png"
	);
	pauseBack->addChild(checkMusic, 1);
	checkMusic->setPosition(Point(320, 220));
	checkMusic->setSelectedState(!MainLayer::musicFlag);
	checkMusic->addEventListener(CC_CALLBACK_2(SetLayer::selectedEvent0, this));
	//音效按钮
	Sprite* sound = Sprite::create("pic/sound.png");
	pauseBack->addChild(sound, 1);
	sound->setPosition(Point(100, 140));

	CheckBox* checkSound = CheckBox::create(
			"button/sound_on.png",
			"button/sound_off.png",
			"button/sound_off.png",
			"button/sound_stop.png",
			"button/sound_stop.png"
	);
	pauseBack->addChild(checkSound, 1);
	checkSound->setPosition(Point(320, 140));
	checkSound->setSelectedState(!MainLayer::soundFlag);
	checkSound->addEventListener(CC_CALLBACK_2(SetLayer::selectedEvent1, this));

	//返回主菜单
	MenuItemImage* menuItem = MenuItemImage::create(
		"button/menu.png",
		"button/menu_off.png",
		 CC_CALLBACK_1(SetLayer::menuCallBack, this) //点击时执行的回调方法
	);
	menuItem->setPosition(Point(200, 60));

	Menu* menu = Menu::create(menuItem, NULL);
	//设置菜单位置
	menu->setPosition(Point::ZERO);
	pauseBack->addChild(menu,1);


	//创建一个网络节点对象
	NodeGrid* effectNode = NodeGrid::create();
	//将网格节点添加到布景
	this->addChild(effectNode, 11);
	//创建标题精灵
	Sprite* title = Sprite::create("pic/title.png");
	//获取标题精灵尺寸大小
	Size size = title->getContentSize();
	//设置精灵位置
	title->setPosition(Point(270, 800));
	//将精灵添加到布景中
	effectNode->addChild(title, 1);
	//关闭深度检测
	Director::getInstance()->setDepthTest(false);
	//涟漪
	effectNode->runAction(RepeatForever::create(Ripple3D::create(2.0f, Size(64,48),Point(270, 800), 360, 2, 10)));

	return true;

}
void SetLayer::menuCallBack(Ref* pSender)
{
	sceneManager->goToMainScene();
}
void SetLayer::selectedEvent0(Ref* pSender,CheckBox::EventType type)
{
    switch (type)
    {
        case CheckBox::EventType::SELECTED:
        	CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
			MainLayer::musicFlag = false;
            break;
        case CheckBox::EventType::UNSELECTED:
        	CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
			MainLayer::musicFlag = true;
            break;
        default:
            break;
    }
}
void SetLayer::selectedEvent1(Ref* pSender,CheckBox::EventType type)
{
    switch (type)
    {
        case CheckBox::EventType::SELECTED:
        	MainLayer::soundFlag = false;
            break;
        case CheckBox::EventType::UNSELECTED:
        	MainLayer::soundFlag = true;
            break;
        default:
            break;
    }
}
