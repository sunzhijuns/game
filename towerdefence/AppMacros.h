#ifndef __AppMacros_H__
#define __AppMacros_H__

#include "cocos2d.h"
#include <string>

using namespace cocos2d;
using namespace std;

//自定义的宏
#define begin_PATH string("begin/")
#define rankinglist_PATH string("rankinglist/")
#define choose_PATH string("choose/")
#define help_PATH string("help/")
#define pause_PATH string("pause/")
//定义不同层次的Z Level
#define BACKGROUND_LEVEL_CGQ 0	//背景层次
#define GAME_LEVEL_CGQ 1		//游戏活动层次
#define DASHBOARD_LEVEL_CGQ 3	//仪表板层次

#endif
