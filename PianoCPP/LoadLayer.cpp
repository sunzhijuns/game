#include "LoadLayer.h"
#include "AppMacros.h"

using namespace cocos2d;
using namespace ui;

vector<Sprite3D*> LoadLayer::instrument3D;

bool LoadLayer::init()
{
	//调用父类的初始化
	if ( !Layer::init() )
	{
		return false;
	}

	instrumentNum = 1;
    currentNum = 0;

	//获取可见区域尺寸
	Size visibleSize = Director::getInstance()->getVisibleSize();
	//获取可见区域原点坐标
	Point origin = Director::getInstance()->getVisibleOrigin();

	label = Label::createWithTTF("0 %",fonts_RESOURE_PATH + "Marker Felt.ttf", 40);
	label->setPosition(Point(visibleSize.width/2, visibleSize.height/2));
	this->addChild(label, 10);

	Sprite* spp = Sprite::create(pic_RESOURE_PATH + "loadBack.png");
	spp->setPosition(Point(visibleSize.width/2, visibleSize.height/2));
	this->addChild(spp, 1);

    
    string route1 = pic_RESOURE_PATH + "loadingBack.png";		//底座图片
	string route2 = pic_RESOURE_PATH + "load.png";				//拖拉进度图片
	string route3 = pic_RESOURE_PATH + "transparent.png";			//旋钮图片
    
    potentiometer = ControlPotentiometer::create(				//创建圆盘拖拉条对象
                                                 route1.c_str(),route2.c_str(),route3.c_str());
	potentiometer->setPosition(Point(visibleSize.width/2, visibleSize.height/2));
	potentiometer->setMinimumValue(0);
	potentiometer->setMaximumValue(100);
	potentiometer->setValue(0);
	potentiometer->setEnabled(false);
	this->addChild(potentiometer, 2);
    
    Label* labels = Label::createWithTTF("\u52a0\u8f7d\u4e2d......",fonts_RESOURE_PATH + "FZKATJW.ttf", 40);
	labels->setPosition(Point(visibleSize.width/2, visibleSize.height/2-120));
	this->addChild(labels, 10);

	loadingPic();

	return true;

}

void LoadLayer::loadingPic()
{
	auto TexureCache=Director::getInstance()->getTextureCache();
	TexureCache->addImageAsync(pic_RESOURE_PATH + "back.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
    TexureCache->addImageAsync(pic_RESOURE_PATH + "back_down.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
    TexureCache->addImageAsync(pic_RESOURE_PATH + "black.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
    TexureCache->addImageAsync(pic_RESOURE_PATH + "black_down.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
    TexureCache->addImageAsync(pic_RESOURE_PATH + "black_small.jpg", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
    TexureCache->addImageAsync(pic_RESOURE_PATH + "black_small_half.jpg", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
    TexureCache->addImageAsync(pic_RESOURE_PATH + "blackBackGround.png",CC_CALLBACK_1(LoadLayer::loadingCallBack,this));
    TexureCache->addImageAsync(pic_RESOURE_PATH + "crystal_cn.png",CC_CALLBACK_1(LoadLayer::loadingCallBack,this));
    TexureCache->addImageAsync(pic_RESOURE_PATH + "exit.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "exit_down.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "flute_cn.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "guitar_cn.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "instrument.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "koto_cn.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "kuang.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "left.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "musicbox_cn.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "piano_cn.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "right.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "sax_cn.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "selectBack.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "sound.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "soundBackGround.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "transparent.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "violin_cn.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "volume_cn.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "white.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "white_down.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "white_small.jpg", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
	TexureCache->addImageAsync(pic_RESOURE_PATH + "yellow.png", CC_CALLBACK_1(LoadLayer::loadingCallBack, this));
}

void LoadLayer::loadingCallBack(Ref* r)
{
	++currentNum;
	int percent=(int)(((float)currentNum / PIC_TOTAL) * 100);
	string ss = StringUtils::format("%d %%", percent);
	label->setString(ss.c_str());
	potentiometer->setValue(percent);
    
    if(currentNum == 5)
    {
        auto director = Director::getInstance();
        schedRound = director->getScheduler();
        schedRound->schedule(schedule_selector(LoadLayer::update), this, 0.02, false);
    }

	if(currentNum == PIC_TOTAL)
	{
		sceneManager->goToGameScene();
	}
}

void LoadLayer::loading3D(int number)
{
    
    
    string name;
	switch(number)
	{
		case 1:
			name = "piano";
			break;
		case 2:
			name = "sax";
			break;
		case 3:
			name = "mb";
			break;
		case 4:
			name = "guitar";
			break;
		case 5:
			name = "violin";
			break;
		case 6:
			name = "flute";
			break;
		case 7:
			name = "fxc";
			break;
		case 8:
			name = "koto";
			break;
	}
    string route1 = obj_RESOURE_PATH + name + ".obj";
    string route2 = pic_RESOURE_PATH + name + ".png";
	Sprite3D* sp = Sprite3D::create(route1.c_str(), route2.c_str());
	sp->setScale(2);
	sp->retain();
	LoadLayer::instrument3D.push_back(sp);
    
}

void LoadLayer::update(float dt)
{

	if(instrumentNum <= OBJ_TOTAL)
	{
		loading3D(instrumentNum);
		instrumentNum++;
	}else
	{
		schedRound->pauseTarget(this);
	}
}
