#ifndef __AppMacros_H__
#define __AppMacros_H__
#include "cocos2d.h"
//用来进行算法搜索的六个方向的二维数组
int SEQUENCEZARRAY[8][2] = //col, row
{
	{0,1},{-1, 0},{1, 0},
	{0,-1},{-1, 1},
	{-1, -1},{1, -1},{1, 1}
};
const int kParticleZOrder = 10;
const int kItemZOrder = 10;
const int kOceanBg0ZOrder = -1;
const int kOceanBg1ZOrder = 0;
const int kGamePauseBBZOrder = 10;
const int kWaveZOrder = 100; //顶部的灰色骷髅头
const int kShipWaveZOrder = 7;//第几波敌船即将来袭


#define HKMJITEMSPTAG 20
#define PLANEITEMSPTAG 21
#define MAPWIDTH 768
#define CELLSIZE 16
#define STARTROW 47
#define STARTCOL 70
#define WQTAG 10
#define RCOFFSET 1
#define PI 3.1415926
#define DWSPRITETAG 13
#define DWPTAG 12
#define COMPASSSPRITE 1
#define BQSPRITETAG 2
#define R 123
#define ZMAX 200
#define ATTACKRANGE 80
#define HEIGHT  768
#define WIDTH  1136
#define JUNXIANSPTAG 101
#endif
