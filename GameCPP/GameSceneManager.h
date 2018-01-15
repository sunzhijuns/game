#ifndef __GameSceneManager_H__
#define __GameSceneManager_H__

#include "cocos2d.h"

using namespace cocos2d;
//用于创建场景的类
class GameSceneManager
{
public:
	Scene* mainScene;
	Scene* gameScene;
	Scene* birdScene;
	Scene* fruitScene;
	Scene* musicScene;
	Scene* rankScene;
	Scene* aboutScene;
	Scene* helpScene;
public:
    //创建主界面场景对象的方法
    void createMainScene();
    //转到撞死鸟游戏界面
    void goToFlappyScene(int);
    //转到水果大暴走游戏界面
    void goToSquishyScene(int);
    //转到游戏选择界面
    void goToGameScene();
	//转到主界面
    void goToMainScene();
    //转到设置界面
    void goToSetScene();
    //转到排行榜界面
    void goToRankBirdScene();
    void goToRankFruitScene();
    //转到关于界面
    void goToAboutScene();
    //转到帮助界面
    void goToHelpScene();
};

#endif
