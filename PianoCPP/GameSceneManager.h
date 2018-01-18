#ifndef __GameSceneManager_H__
#define __GameSceneManager_H__

#include "cocos2d.h"

using namespace cocos2d;

//用于创建场景的类
class GameSceneManager
{
public:
	Scene* logoScene;
	Scene* loadScene;
	Scene* gameScene;
public:
	//创建场景对象的方法
	void createLogoScene();
	//切换场景方法
	void goToGameScene();
	void goToLoadScene();
};

#endif
