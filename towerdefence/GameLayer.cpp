#include <audio/include/SimpleAudioEngine.h>
#include "GameLayer.h"
#include "BulletSprite.h"
#include "ChooseLayer.h"
#include "DialogLayer.h"
#include "AchieveLayer.h"
#include "AppDelegate.h"
#include "AppMacros.h"
#include "Weapon.h"

using namespace cocos2d;
using namespace std;
static Dictionary s_dic;

#define CELL_BORDER        (28)	//六边形边长
#define CELL_HEIGHT        (50)	//六边形格子贴图高度 由六边形边长乘以sin(60)近似得到

enum
{
	kTagTileMap
};

//用来进行算法搜索的六个方向的二维数组
int sequenceZ[2][6][2] = 	//col, row
{
		{//偶数行
				{-1, -1},
				{0, -1},
				{1, 0},
				{-1, 0},
				{-1, 1},
				{0, 1}
		},
		{//奇数行
				{1, -1},
				{0, -1},
				{-1, 0},
				{1, 0},
				{0, 1},
				{1, 1}
		}
};
//出发点row, col
static int source[]={5, 15};
//目的点的row, col
static int targetAll[1][2] ={{3, 2}};

bool GameLayer::isPause=false;
//结束点col, row
int* target;
//0代表未去过，1代表去过
int** visited;
//A*用优先级队列
typedef int(*INTPARR)[2];
//A*用优先级队列容器中的比较器，内部重载了（）操作符，此为C++中的函数对象
struct cmp
{
	bool operator()(INTPARR o1, INTPARR o2)
	{
		int* t1 = o1[1];
		int* t2 = o2[1];

		//门特卡罗距离
		int a = visited[o2[0][1]][o2[0][0]]+abs(t1[0]-target[0])+abs(t1[1]-target[1]);
		int b = visited[o2[0][1]][o2[0][0]]+abs(t2[0]-target[0])+abs(t2[1]-target[1]);

		return a>b;
	}
};
//A*用优先级队列
priority_queue<INTPARR,vector<INTPARR>,cmp>* astarQueue;
GameLayer::GameLayer(){}
//析构函数
GameLayer::~GameLayer()
{
	//释放内存
	freeMemory();
	//释放地图
	for(int i=0;i<row;i++)
	{
		delete []MAP_DATA[i];
	}
	delete []MAP_DATA;
}

//初始方法
bool GameLayer::init()
{
	//调用父类的初始化
	if ( !Layer::init() )
	{
		return false;
	}

	//创建一个精灵对象，包含background.png图片
	auto gbsprite = Sprite::create("pic/back.png");
	//设置精灵对象的位置
	gbsprite->setPosition(Point(400,240));
	//将精灵添加到布景中
	this->addChild(gbsprite,BACKGROUND_LEVEL_CGQ);

	//加载音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_button_press.mp3"
	);

    //广度优先A*算法
	astarQueue = NULL;
	hm = NULL;
	//获取当前屏幕的大小
	auto winSize = Director::getInstance()->getWinSize();
	//加载TMX地图
	auto map = TMXTiledMap::create("map/MyTilesMap"+
			StringUtils::format("%d",ChooseLayer::modeLevel)+".tmx");
	//设置地图的锚点
    map->setAnchorPoint(Point(0,1.0));
	//设置地图位置
    map->setPosition(Point(0, winSize.height-3));
	//将地图添加到布景中
	addChild(map,BACKGROUND_LEVEL_CGQ, kTagTileMap);
	//获得地图的宽度和高度
	int mapWidth = map->getMapSize().width;
	int mapHeight = map->getMapSize().height;
	//将地图的宽度和高度设置为二维数组的行和列
	row = mapWidth;
	col = mapHeight;
	//创建动态二维数组
	MAP_DATA = new int*[row];
	for(int i = 0; i<row; i++)
	{
		MAP_DATA[i] = new int[col];
	}

	//得到地图中的layer
	tmxLayer = map->layerNamed("Layer 0");
	//获得一个图素中的属性值
	for(int i=0; i<row; i++)
	{
		for(int j=0; j<col; j++)
		{
			//得到layer中每一个图块的gid
			unsigned int gid = tmxLayer->tileGIDAt(Point(i, j));
			//通过gid得到该图块中的属性集,属性集中是以键值对的形式存在的
			auto tiledic = map->propertiesForGID(gid);
			//通过键得到value
			const String mvalue = tiledic.asValueMap()["value"].asString();
			//将mvalue转换成int变量
			int mv = mvalue.intValue();
			//初始化地图中的数据
			MAP_DATA[i][j] = mv;

		}
	}
	//设置抗锯齿，如果需要对地图进行放大或缩小时，就可以使用
	auto children = tmxLayer->getChildren();
	SpriteBatchNode* child = NULL;
	for(Object* object:children)
	{
		child = static_cast<SpriteBatchNode*>(object);
		child->getTexture()->setAntiAliasTexParameters();
	}
	//获得单个图块的大小，为了在绘制时得到偏移量，否则绘制出来的线条有半个图块的偏移差
	auto m_tamara = tmxLayer->tileAt(Point(0,0));
	auto texture = m_tamara->getTexture();
	auto blockSize = texture->getContentSize();
	trans = Point(blockSize.width/4,blockSize.height/2);

	//创建“暂停”按钮精灵
	MenuItemImage *zanTingItem = MenuItemImage::create
	(
		"pic/zanting.png",		//平时的图片
		"pic/zanting.png",		//选中时的图
		CC_CALLBACK_1(GameLayer::zanTing, this)
	);
	//设置暂停菜单按钮的位置
	zanTingItem->setPosition(Point(40,140));

	//创建暂停菜单对象
	pMenu = Menu::create(zanTingItem,NULL);
	//设置菜单的位置
	pMenu->setPosition(Point(0,0));
	//将菜单添加到布景中
	this->addChild(pMenu,DASHBOARD_LEVEL_CGQ);

	//创建终点精灵
	targetSprite = Sprite::create("pic/target.png");
	//设置精灵的位置
	auto end = tmxLayer->positionAt(Point(targetAll[0][1],targetAll[0][0]));
	endWorld = tmxLayer->convertToWorldSpaceAR(Point(end.x+trans.x,end.y+trans.y));
	targetSprite->setPosition(endWorld);
	//将终点精灵对象添加到布景中
	this->addChild(targetSprite,GAME_LEVEL_CGQ+1);
	//加载音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_swish.mp3"
	);
	//加载音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_creep_die_0.mp3"
	);
	//加载游戏结束的音效音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_game_over.mp3"
	);
	//加载第三个炮台发射子弹的音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_rocket_launch.mp3"
	);
	//加载第一个炮台发射子弹的音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_minigun_hit.mp3"
	);
	//加载第二个炮台发射子弹的音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_laser_beam.mp3"
	);

    //创建存放怪的数组
    arrMon = Array::create();
    arrMon ->retain();
	//创建存放怪action的数组
    arrAction = Array::create();
    arrAction ->retain();
    //创建存放武器的数组
    arrWeap = Array::create();
    arrWeap ->retain();
    //创建存放武器菜单数组
    arrMenu = Array::create();
    arrMenu ->retain();
    //创建存放怪Bullet的数组
    arrBullet = Array::create();
    arrBullet ->retain();
    //创建存放金钱的数组
    arrSellUpdate = Array::create();
    arrSellUpdate ->retain();

	//添加所有label
	addLabel();
	//添加防御塔菜单精灵
	addMenuSprite();
	//初始的总金币数
	money = 280;
	//初始化生命值
	ten = 18;
	//初始化时间常量
    TIME_MAIN=0.7 ;
	//初始化升级的武器
	updateWeapon = NULL;
	//升级防御塔的标志位
	WeaponUpdate = false;
	//初始化游戏结束的标志位为false
	GameOver=false;
	//初始化野怪移动的标志位为false
	isMonsterRun = false;
	//初始化创建怪的标志位
	isfoundMonster = false;
    //移除防御塔精灵对象的标志位
    removeWeap = false;
    //初始化游戏中怪的批次数
    pass = 0;
    //初始化总分数为0
    score = 0;
	//设置定时回调指定方法干活
	auto director = Director::getInstance();
	auto sched = director->getScheduler();
	//定时调用run方法，以秒为单位
	sched->scheduleSelector(SEL_SCHEDULE(&GameLayer::run), this, 1.0, false);
	//定时调用runBullet方法，以秒为单位
	sched->scheduleSelector(SEL_SCHEDULE(&GameLayer::runBullet), this, 0.002f, false);
	//定时调用attack方法，以秒为单位
	sched->scheduleSelector(SEL_SCHEDULE(&GameLayer::attack), this, 0.3f, false);

	//创建单点触摸监听
	EventListenerTouchOneByOne* listener = EventListenerTouchOneByOne::create();
	//设置下传触摸
	listener->setSwallowTouches(true);
	//开始触摸时回调onTouchBegan方法
	listener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
	//开始触摸时回调onTouchMoved方法
	listener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);
	//开始触摸时回调onTouchEnded方法
	listener->onTouchEnded = CC_CALLBACK_2(GameLayer::onTouchEnded, this);
	//添加到监听器
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	//初始化计算路径的标志位为false
	isCaulateOver = false;
	//拿到目标点
	target = targetAll[0];
	//绘制路径
	if(calculatePath())
	{
		printPath();
	}
	//初始化创建多个怪的方法，然后调用ready方法
	foundMonsters();

	return true;
}

void GameLayer::zanTing(Object* pSender)
{
	//播放音效
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
	//判断粒子效果的标志位
	if(!isPause)
	{
		//暂停背景音乐
		CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
		//获取导演
		Director *director = Director::getInstance();
		//导演执行暂停音乐的工作
		director->pause();
		//创建暂停界面
		DialogLayer* dialogLayer = DialogLayer::create();
		//设置位置
		dialogLayer->setPosition(Point(0,0));
		//添加到布景中
		this->addChild(dialogLayer,6);
		//暂停键的标志位
		isPause=true;
	}
}

//计算路径的方法
bool GameLayer::calculatePath()
{
	//释放内存
	freeMemory();
	//初始化搜索列表
	initForCalculate();
	//初始化地图
	initVisitedArr();
	//用A*算法搜索路径
	bool b=BFSAStar();
	return b;
}

//释放内存
void GameLayer::freeMemory()
{
	//清空hm中的键值对
	if(hm != NULL)
	{
		hm->clear();
		delete hm;
		hm = NULL;
	}
	//清空广度优先A*队列中的指针
	if(astarQueue != NULL)
	{
		while(!astarQueue->empty())
		{
			astarQueue->pop();
		}
		delete astarQueue;
		astarQueue = NULL;
	}
	//释放访问数组
	if(visited != NULL)
	{
		for(int i=0;i<row;i++)
		{
			delete []visited[i];
		}
		delete []visited;
		visited = NULL;
	}
	isCaulateOver = false;
}

//计算之前初始化计算中用到的容器
void GameLayer::initForCalculate()
{
	//创建动态二维数组
	visited = new int*[row];
	for(int i = 0; i<row; i++)
	{
		visited[i] = new int[col];
	}
	//A*优先级队列比较器
	astarQueue = new priority_queue<INTPARR,vector<INTPARR>,cmp>();//A*用优先级队列
	//结果路径记录
	hm = new map<string,int(*)[2]>();
}

//初始化去过未去过的数组
void GameLayer::initVisitedArr()
{
	for(int i=0;i<row;i++)
	{
		for(int j=0;j<col;j++)
		{
			visited[i][j] = 0;
		}
	}
}

//广度优先A*算法BFSAStar
bool GameLayer::BFSAStar()
{
	//定义一个标志位
	bool flag = true;
	string str1;
	string str2;
	//开始状态
	int(*start)[2] = new int[2][2];
	start[0][0] = source[0];
	start[0][1] = source[1];
	start[1][0] = source[0];
	start[1][1] = source[1];
	//将开始点放进A*用优先级队列中
	astarQueue->push(start);
	while(flag)
	{
		//如果栈不空
		if(astarQueue->empty())
		{
			return false;
		}
		//从队首取出边
		int(*currentEdge)[2] = astarQueue->top();
		astarQueue->pop();
		//取出此边的目的点
		int* tempTarget = currentEdge[1];
		//判断目的点是否去过，若去过则直接进入下次循环
		if(visited[tempTarget[1]][tempTarget[0]] != 0)
		{
			continue;
		}
		visited[tempTarget[1]][tempTarget[0]] = visited[currentEdge[0][1]][currentEdge[0][0]]+1;
		str1 = StringUtils::format("%d", tempTarget[0]);
		str2 = StringUtils::format("%d", tempTarget[1]);
		//记录目的点的父节点
		hm->insert(map<string,int(*)[2]>::value_type(str1+":"+str2,currentEdge));
		//判断是否找到目的点
		if(tempTarget[0] == target[0] && tempTarget[1] == target[1])
		{
			isCaulateOver= true;
			return isCaulateOver;
		}
		//将所有可能的边入优先级队列
		int currCol = tempTarget[0];
		int currRow = tempTarget[1];
		int(*sequence)[2] = NULL;
		//根据当前图块的奇数偶数行来确定搜索的方向
		if(currRow%2 == 0)
		{
			sequence = sequenceZ[0];
		}else
		{
			sequence = sequenceZ[1];
		}
		for(int m = 0; m<6; m++)
		{
			int* rc = sequence[m];
			int i = rc[1];
			int j = rc[0];

			if(i==0 && j==0)
			{
				continue;
			}
			if(currRow+i>=0 && currRow+i<row && currCol+j>=0 && currCol+j<col &&
					MAP_DATA[currRow+i][currCol+j]!=1)
			{
				//创建二维数组
				int(*tempEdge)[2] = new int[2][2];
				//设置为下一个目标点
				tempEdge[0][0] = tempTarget[0];
				tempEdge[0][1] = tempTarget[1];
				tempEdge[1][0] = currCol+j;
				tempEdge[1][1] = currRow+i;
				//将二维数组添加进A*用优先级队列中
				astarQueue->push(tempEdge);
			}
		}
	}
}

//打印结果路径
void GameLayer::printPath()
{
	//清除以前的路径
	way.clear();

	string str1;
	string str2;
	//绘制最终的搜索结果路径
	map<string, int(*)[2]>::iterator iter;
	int* temp = target;
	while(true)
	{
		str1 = StringUtils::format("%d", temp[0]);
		str2 = StringUtils::format("%d", temp[1]);
		string key = str1+":"+str2;
		//寻找对应的值
		iter = hm->find(key);
		int(*tempA)[2] = iter->second;
		//查到元素
		if(iter != hm->end())
		{
			//拿到起始点的坐标
			Point start = tmxLayer->positionAt(Point(tempA[0][1],tempA[0][0]));
			//拿到目标点的坐标
			Point end = tmxLayer->positionAt(Point(tempA[1][1],tempA[1][0]));
			//将起始点转换到世界坐标系中
			Point startWorld = tmxLayer->convertToWorldSpaceAR(Point(start.x+trans.x,start.y+trans.y));
			//将目标点转换到实际坐标系中
			Point endWorld = tmxLayer->convertToWorldSpaceAR(Point(end.x+trans.x,end.y+trans.y));
			//将目标点添加到路径中
			way.push_back(endWorld);
			CCLOG("endWorld.x=%f,endWorld.y=%f",endWorld.x,endWorld.y);
			//排队密度
			glLineWidth( 3.0f );
			//绘制路径
			cocos2d::ccDrawColor4F(0.0f, 0.0f, 0.0f, 1.0f);
			cocos2d::ccDrawLine(startWorld, endWorld);
		}
		//判断有否到出发点
		if(tempA[0][0]==source[0]&&tempA[0][1]==source[1])
		{
			break;
		}
		//线段的起点数组
		temp = tempA[0];
	}
}

//出野怪前的准备方法
void GameLayer::ready()
{
	//创建起点精灵
	startSprite = Sprite::create("pic/start.png");
	//设置精灵的位置
	auto start = tmxLayer->positionAt(Point(source[1],source[0]));
	startWorld = tmxLayer->convertToWorldSpaceAR(Point(start.x+trans.x,start.y+trans.y));
	startSprite->setPosition(startWorld);
	//将精灵对象添加到布景中
	this->addChild(startSprite,5);

	//在起点精灵上添加百分比动作特效
	ProgressTo *to1 = ProgressTo::create(6, 100);
	left = ProgressTimer::create(Sprite::create("pic/ring2.png"));
	left->setType( ProgressTimer::Type::RADIAL);
	left->setRotation(-90);
	this->addChild(left,5);
	left->setPosition(startWorld);
	left->runAction(
			Sequence::create(to1,
			CallFuncN::create(CC_CALLBACK_0(GameLayer::playGameCallback, this)),
			NULL
			)
	);

}

//出野怪前准备方法的回调方法
void GameLayer::playGameCallback()
{
	//移除精灵
	this->removeChild(left);
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_swish.mp3");
	//怪移动的标志位设置为true
	isMonsterRun=true;
}

//出野怪时起始点要播放的两个特效
void GameLayer::addParticle(Point point,float time)
{
	if(DialogLayer::isParticle)
	{
		int countVar=rand()%5;
		for(int i=0;i<35+countVar;i++)
		{
			float angleVar=rand()%360;
			int lengthVar=rand()%10;
			int countVar=rand()%10;
			int spriteLength=rand()%5;

			particle=Sprite::create("pic/white1.png");
			particle->setAnchorPoint(Point(0,0.5));
			particle->setRotation(-angleVar);
			this->addChild(particle,5);

			Point vocter=ccpForAngle(angleVar*3.1415926/180);
			Point monPointOne = ccpAdd(ccpMult(vocter,(float)(lengthVar+30)),point);
			particle->setPosition(monPointOne);
			particle->setScale(0.7);
			particle->runAction(
						Sequence::create(
								Spawn::createWithTwoActions
								(
									MoveTo::create(time*3/7.0,point),
									FadeIn::create(time/5.0)
								),
								CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
								NULL
								)
						);
		}

		cc = Sprite::create("pic/fire1.png");
		ActionInterval *act=FadeIn::create(time*4/5);
		ActionInterval *activeFade=FadeOut::create(time*4/5);
		cc->setPosition(point);
		this->addChild(cc,6);
		cc->setScale(2.0);
		cc->runAction(Sequence::create(
				Spawn::createWithTwoActions
							(
								act,
								activeFade
							),
							CallFuncN::create(CC_CALLBACK_0(GameLayer::removeSpriteAdd,this)),
							NULL
				)
		);
	}else{
		removeSpriteAdd();
	}
}

//将地图格子行列号转换为对应格子的贴图坐标
Point GameLayer::fromColRowToXY(int col, int row)//入口参数//横着看的x，y
{
	row++;
	Point start = tmxLayer->positionAt(Point(col,row));//横着看的x，y
	Point startWorld = tmxLayer->convertToWorldSpaceAR(Point(start.x+trans.x,start.y+trans.y));
	return startWorld;
}
//将触控点位置转换为地图格子行列号
Point GameLayer::fromXYToColRow(int xPos, int yPos)
{
    #define GRID_WIDTH        (CELL_BORDER*1.5f)
    #define GRID_HEIGHT       (CELL_HEIGHT/2)
	#define TEMP_1            ((GRID_WIDTH*GRID_WIDTH - GRID_HEIGHT*GRID_HEIGHT)/2.f)
    #define TEMP_2            ((GRID_WIDTH*GRID_WIDTH + GRID_HEIGHT*GRID_HEIGHT)/2.f)

	int i = (int)floor(yPos/GRID_HEIGHT);
	int y = (int)(yPos-i*GRID_HEIGHT);

	int j = (int)floor(xPos/GRID_WIDTH);
	int x = (int)(xPos-j*GRID_WIDTH);

	if(((i+j)&1)!=0)
	{
		if(x*GRID_WIDTH-y*GRID_HEIGHT > TEMP_1) j++;
	}
	else
	{
		if(x*GRID_WIDTH+y*GRID_HEIGHT > TEMP_2) j++;
	}

	i = (int)floor((i+(1-(j&1)))/2);
	//8 是地图总行数，从0开始数，实际开发中需要根据具体情况修改
	return ccp(j-1,9-i);
}

//选中已经放在地图中的防御塔并对其进行操作
bool GameLayer::onTouchBegan(Touch *pTouch, Event *pEvent)
{
	//move的时候began不走
	if(touchMove)
	{
		return false;
	}
	//拿到当前触控点的坐标
	Point point = pTouch->getLocation();
	//如果防御塔不升级
	if(!WeaponUpdate)
	{
		//遍历存放防御塔菜单精灵的数组
		for(int k = 0; k<arrMenu->count(); k++)
		{
			//拿到防御塔菜单精灵
			Weapon* pWeapon =(Weapon*)arrMenu->objectAtIndex(k);
			//获取防御塔的坐标
			Point pp = pWeapon->getPosition();
			//如果是点击菜单防御塔
			if(abs(point.x- pp.x)<32&&abs(point.y- pp.y)<32)
			{
				//得到点击到的菜单防御塔的id
				int id = pWeapon->id;
				//得到安装该防御塔所需的金币
				int tempValue=pWeapon->value;
				//拿到触控点的地图的行列号
				Point ccpxy = fromXYToColRow((int)point.x,(int)point.y);
				//将行列号转换为对应的地图贴片的坐标
				Point ccp = fromColRowToXY(ccpxy.x,ccpxy.y);
				//如果钱不够
				if(money<tempValue)
				{
					return false;
				}
				//根据得到的菜单防御塔的id创建一个防御塔
				Weapon *oneTa =Weapon::create(id);
				//设置防御塔的位置
				oneTa->setPosition(ccp);
				//将防御塔添加到布景中
				this->addChild(oneTa,6);
				//设置防御塔
				s_dic.setObject(oneTa, pTouch->getID());
				//将防御塔移动的标志位设置为true
				touchMove=true;

				return true;
			}
		}
	}else{
		//循环数组看选择的是升级还是出售
		for(int k=0;k<arrSellUpdate->count();k++)
		{
			std::string overStr = "$";
			//定义一个临时变量，用来记录防御塔更新时所需的金币数
			int tempValue=updateWeapon->upValue;
			//如果是升级
			if(abs(point.x-730)<32&&abs(point.y-408)<32)
			{
				//如果钱不够升级
				if(money<tempValue)
				{
					return false;
				}
				//总的金币数减去升级防御塔所需的金币数
				money-=tempValue;
				//把int 型的数据转换成string型的 然后set
				char a[6];
				snprintf(a, 6, "%d",money);
				//更新当前总金币数
				moneyL->setString((overStr+a).c_str());
				//调用update方法
				updateWeapon->update();

				return true;
			}
			//如果是出售防御塔
			if(abs(point.x-730)<32&&abs(point.y-68)<32)
			{
				//总的金币数加上要出售防御塔所得的金币数
				money+=updateWeapon->sellValue;
				char a[6];//把int 型的分数转换成string型的 然后set
				snprintf(a, 6, "%d",money);
				//更新总的金币数
				moneyL->setString((overStr+a).c_str());
				//调用sellWeapon方法，出售防御塔
				sellWeapon(updateWeapon);

				return true;
			}
		}
	}
	//遍历存放子防御塔的数组
	int k = 0;
	for(; k<arrWeap->count(); k++)
	{
		//拿到指向子防御塔的指针
		Weapon* pWeapon =(Weapon*)arrWeap->objectAtIndex(k);
		//获取其位置
		Point ccWeapon = pWeapon->getPosition();
		//如果是点击了子防御塔
		if(abs(point.x-ccWeapon.x)<32&&abs(point.y-ccWeapon.y)<32)
		{
			Point ccpxy = fromXYToColRow((int)ccWeapon.x,(int)ccWeapon.y);
			//如果已经选中了一个防御塔
			if(WeaponUpdate)
			{
				//设置为不可见
				(updateWeapon->getChildByTag(1))->setVisible(false);
			}
			//把选中的防御塔给要升级的
			updateWeapon=pWeapon;
			//使得防御塔升级按钮可见
			setUpdateTrue();
			//移动标志位设为false
			touchMove=false;

			return true;
		}
	}
	if(k==arrWeap->count())
	{
		//设置防御塔菜单精灵可见
		setWeaponTrue();
		//触控移动的标志
		touchMove=false;
	}
	return false;
}

//触控移动防御塔的方法
void GameLayer::onTouchMoved(Touch *pTouch, Event *pEvent)
{
	//防御塔的物理位置，看是否在路径当中，若在则重新计算路径
	if(!touchMove)
	{
		return ;
	}
	//得到当前触控位置的坐标
	Point point = pTouch->getLocation();
	//根据菜单防御塔的id创建一个表示防御塔攻击范围的精灵对象
	Weapon* trSprite = (Weapon*)s_dic.objectForKey(pTouch->getID());
	//设置表示防御塔攻击范围的精灵可见
	(trSprite->getChildByTag(1))->setVisible(true);
	//拿到触控点对应的地图的行列号
	Point ccpxy = fromXYToColRow((int)point.x,(int)point.y);
	//将该行列号转换为对应贴图的坐标
	Point ccp = fromColRowToXY(ccpxy.x,ccpxy.y);
	//设置精灵对象的位置
	trSprite->setPosition(ccp);
}

//触控抬起
void GameLayer::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	//如果点击到菜单防御塔的标志位为false则返回
	if(!touchMove)
	{
		return;
	}
	//获取触控点位置的坐标
	Point point = pTouch->getLocation();
	//创建一个表示攻击范围的精灵对象
	Weapon* trSprite = (Weapon*)s_dic.objectForKey(pTouch->getID());
	//设置表示攻击范围的精灵不可见
	(trSprite->getChildByTag(1))->setVisible(false);
	//得到触控点对应的地图的行列号
	Point ccpxy = fromXYToColRow((int)point.x,(int)point.y);
	//将该地图的行列号转换成对应贴图的坐标
	Point ccp = fromColRowToXY(ccpxy.x,ccpxy.y);
	//如果防御塔放置的图块是地图的墙
	if(MAP_DATA[(int)ccpxy.x][(int)(ccpxy.y+1)]!=0)
	{
		//移除防御塔标志位设定为true
		removeWeap = true;
	}else//如果放置的位置为可以放置的位置
	{
		int i=0;
		//循环路径数组，看看是否为路径中的
		for(;i<way.size();i++)
		{
			//拿到路径中的每个点
			Point ccpWay = (Point)way.at(i);
			//如果当前放防御塔的格子为路径中的
			if(ccp.x==ccpWay.x&&ccp.y==ccpWay.y)
			{
				//重新计算路径，把当前格子设为可以放防御塔怪不可以走
				MAP_DATA[(int)ccpxy.x][(int)(ccpxy.y+1)]=1;
				//进行路径搜索
				bool isCaulate=calculatePath();
				//如果重新找到路径
				if(isCaulate)
				{
					//把还没到此格子的怪里的way换了,创建新的way
					printPath();
					//遍历存放怪的数组
					for(int j=0;j<arrMon->count();j++)
					{
						//拿到当前的怪
						Monsters* mon = (Monsters*)arrMon->objectAtIndex(j);
						//第几个路径
						int wayat = way.size()-i;
						//如果怪没有经过该放防御塔的格子(-2为视觉和时间上的关系不是具体数据)
						if(mon->way<wayat-2)
						{	//把新的路径给怪
							mon->selfWay=way;
						}
					}
					//将移除防御塔的标志位设置为false
					removeWeap=false;
					//把防御塔当前处的格子行列给特效精灵对象
					trSprite->pointColRow=ccpxy;
					break;
				}else
				{
					//如果没找到路径,把原来格子设置为怪可以走
					MAP_DATA[(int)ccpxy.x][(int)(ccpxy.y+1)]=0;
					//移除防御塔标志位设定为true
					removeWeap = true;

					break;
				}
			}
		}
		//循环路径数组完成，当前格子不在路径的数组里
		if(i==way.size())
		{
			//不是路径中的，把该格子设为野怪不可走
			MAP_DATA[(int)ccpxy.x][(int)(ccpxy.y+1)]=1;
			//移除防御塔的精灵的标志位设置为false
			removeWeap=false;
			//把防御塔当前处的格子行列给防御塔
			trSprite->pointColRow=ccpxy;
		}
	}
	//如果移除防御塔精灵对象
	if(removeWeap)
	{
		this->removeChild(trSprite);
	}
	else
	{	//成功购买防御塔
		int tempValue=trSprite->value;
		money-=tempValue;
		std::string overStr = "$";
		char a[6];//把int 型的分数转换成string型的 然后set
		snprintf(a, 6, "%d",money);
		moneyL->setString((overStr+a).c_str());
		//将防御塔精灵对象添加到存放子塔的数组中
		arrWeap->addObject(trSprite);
	}
	//为了让began走将移动的标志位设置为false
	touchMove=false;
}

//设置升级菜单精灵可见
void GameLayer::setUpdateTrue()
{
	//遍历存放菜单防御塔的数组
	for(int i=0;i<arrMenu->count();i++)
	{
		//拿到指向菜单防御塔的指针
		Weapon* weapon = (Weapon*)arrMenu->objectAtIndex(i);
		//设置菜单防御塔不可见
		weapon->setVisible(false);
	}
	//遍历存放金币的数组
	for(int j=0;j<arrSellUpdate->count();j++)
	{
		//拿到指向升级菜单的指针
		Sprite* update = (Sprite*)arrSellUpdate->objectAtIndex(j);
		//将升级菜单设置为可见
		update->setVisible(true);
	}
	//将显示升级所需金币数的文本标签设置为可见
	uMoneyL->setVisible(true);
	//将显示卖掉防御塔时金币收入的文本标签设置为可见
	sMoneyL->setVisible(true);
	//防御塔升级的标志位设置为true
	WeaponUpdate = true;
	if(updateWeapon!=NULL)
	{
		//将范围设置为可见
		(updateWeapon->getChildByTag(1))->setVisible(true);
		//如果要升级的防御塔的等级为4，即最高级
		if(updateWeapon->level==4)
		{	//升级到头表示升级的金币不可见
			uMoneyL->setVisible(false);
		}
		//如果剩的钱不够升级
		if(money<updateWeapon->upValue||updateWeapon->level==4||updateWeapon->updateMark==true)
		{	//升级到头||正在升级
			Sprite* update = (Sprite*)arrSellUpdate->objectAtIndex(1);
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(100));
			update->runAction(act);
			ActionInterval *act1=FadeTo::create(0.1f,(GLubyte)(100));
			uMoneyL->runAction(act1);
		}else
		{
			Sprite* update = (Sprite*)arrSellUpdate->objectAtIndex(1);
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(255));
			update->runAction(act);
			ActionInterval *act1=FadeTo::create(0.1f,(GLubyte)(255));
			uMoneyL->runAction(act1);
		}
		//设置升级的钱
		setValue();
	}
}

//设置防御塔菜单精灵可见
void GameLayer::setWeaponTrue()
{
	//遍历存放菜单防御塔的数组
	for(int i=0;i<arrMenu->count();i++)
	{
		//拿到存放在菜单防御塔数组中的防御塔对象
		Weapon* weapon = (Weapon*)arrMenu->objectAtIndex(i);
		//将其设置为可见
		weapon->setVisible(true);
	}
	//遍历存放金币的数组
	for(int j=0;j<arrSellUpdate->count();j++)
	{
		Sprite* update = (Sprite*)arrSellUpdate->objectAtIndex(j);
		update->setVisible(false);
	}
	uMoneyL->setVisible(false);
	sMoneyL->setVisible(false);
	//防御塔更新的标志位设置为false
	WeaponUpdate = false;
	//如果可更新的防御塔为空
	if(updateWeapon!=NULL)
	{
		//将防御塔周围的光圈精灵设为不可见
		(updateWeapon->getChildByTag(1))->setVisible(false);
	}
}

//设置升级的金币
void GameLayer::setValue()
{
	std::string overStr = "$";
	char a[6];//把int 型的分数转换成string型的 然后set
	snprintf(a, 6, "%d",updateWeapon->sellValue);
	sMoneyL->setString((overStr+a).c_str());
	char b[6];//把int 型的分数转换成string型的 然后set
	snprintf(b, 6, "%d",updateWeapon->upValue);
	uMoneyL->setString((overStr+b).c_str());
}

//出售武器的方法
void GameLayer::sellWeapon(Weapon* weapon)
{
	//获取武器所在格子的行列
	Point tempxy = weapon->getPosition();
	//从存放防御塔的数组中移除
	arrWeap->removeObject(weapon);
	//从layer上移除
	this->removeChild(weapon);
	//拿到该地图格子的行列号
	Point ccpxy =fromXYToColRow((int)tempxy.x,(int)tempxy.y);
	//把地图格子设为可以走
	MAP_DATA[(int)ccpxy .x][(int)(ccpxy.y)]=0;
	//将更新防御塔设置为空
	updateWeapon=NULL;
	//游戏玩家的金钱加
	setWeaponTrue();
}

//野怪到终点时的特效
void GameLayer::addParticle(Point point,int id,float time)
{
	if(!DialogLayer::isParticle)
	{
		return;
	}
	std::string picTable[6]={"pic/red.png","pic/yellow.png","pic/blue.png","pic/white.png","pic/red.png","pic/blue.png"};
	std::string picParticle[6]={"pic/hong.png","pic/orige.png","pic/zi.png","pic/liang.png","pic/hong.png","pic/zi.png"};
	int countVar=rand()%10;
	int angle=0;
	for(int i=0;i<55+countVar;i++)
	{
		float timeVar=(rand()%10)/10.0;
		float angleVar=(rand()%10)/10.0;
		Sprite* particle= Sprite::create(picTable[id-1].c_str());
		particle->setAnchorPoint(Point(0,0.5));
		particle->setPosition(point);
		particle->setScale(1.0);
		particle->setRotation(-(angle+angleVar));
		Point vocter=ccpForAngle((angle+angleVar)*3.1415926/180);
		int lange=rand()%200;
		angle+=11;
		this->addChild(particle,5);
		particle->runAction(
			Sequence::create(
					Spawn::createWithTwoActions
					(
						MoveBy::create(1+timeVar,Point((200+lange)*vocter.x,(200+lange)*vocter.y)),
						FadeOut::create(1+timeVar)
					),
					CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
					NULL
			)
		);
	}

	Sprite* ccp = Sprite::create(picParticle[id-1].c_str());
	ActionInterval *act=ScaleTo::create(0.5f,6);
	ActionInterval *activeFade=FadeOut::create(0.5f);
	ccp->setPosition(point);
	this->addChild(ccp,7);
	ccp->runAction(
		Sequence::create(
				Spawn::createWithTwoActions
				(
					act,
					activeFade
		),
		CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
		NULL
		)
	);

	cc = Sprite::create("pic/fire1.png");
	ActionInterval *act1=FadeIn::create(time*4/9);
	ActionInterval *activeFade1=FadeOut::create(time*4/9);
	cc->setPosition(point);
	this->addChild(cc,6);
	cc->setScale(3.0);
	cc->runAction(Sequence::create(
			Spawn::createWithTwoActions
						(
							act1,
							activeFade1
						),
						CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
						NULL
			)
	);
}

//野怪死时的特效
void GameLayer::addParticle1(Point point,int id,float time)
{
	if(!DialogLayer::isParticle)
	{
		return;
	}
	std::string picTable[6]={"pic/red.png","pic/yellow.png","pic/blue.png","pic/white.png","pic/red.png","pic/blue.png"};
	std::string picParticle[6]={"pic/hong.png","pic/orige.png","pic/zi.png","pic/liang.png","pic/hong.png","pic/zi.png"};
	int countVar=rand()%10;
	int angle=0;
	for(int i=0;i<35+countVar;i++)
	{
		float timeVar=(rand()%10)/10.0;
		float angleVar=(rand()%10)/10.0;
		Sprite* particle= Sprite::create(picTable[id-1].c_str());
		particle->setAnchorPoint(Point(0,0.5));
		particle->setPosition(point);
		particle->setScale(0.8);
		particle->setRotation(-(angle+angleVar));
		Point vocter=ccpForAngle((angle+angleVar)*3.1415926/180);
		int lange=rand()%200;
		angle+=11;
		this->addChild(particle,5);
		particle->runAction(
			Sequence::create(
					Spawn::createWithTwoActions
					(
						MoveBy::create(1+timeVar,Point((200+lange)*vocter.x,(200+lange)*vocter.y)),
						FadeOut::create(1+timeVar)
					),
					CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
					NULL
			)
		);
	}
	Sprite* ccp = Sprite::create(picParticle[id-1].c_str());
	ActionInterval *act=ScaleTo::create(0.5f,6);
	ActionInterval *activeFade=FadeOut::create(0.5f);
	ccp->setPosition(point);
	this->addChild(ccp,6);
	ccp->runAction(
		Sequence::create(
				Spawn::createWithTwoActions
				(
					act,
					activeFade
		),
		CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
		NULL
		)
	);
}

//第二个防御塔攻击怪时调用的特效
void GameLayer::addParticle(Point point,int id,float time,float angle)
{
	if(!DialogLayer::isParticle)
	{
		return;
	}
	std::string picTable[6]={"pic/red.png","pic/yellow.png","pic/blue.png","pic/white.png","pic/red.png","pic/blue.png"};
	//用随机数生成器生成火焰条
	int countVar=rand()%5;
	for(int i=0;i<16+countVar;i++)
	{
		float timeVar=(rand()%10)/1.0;
		float angleVar=rand()%90;
		angleVar=45-angleVar;
		Sprite* particle= Sprite::create(picTable[id-1].c_str());
		particle->setAnchorPoint(Point(0,0.5));
		particle->setPosition(point);
		particle->setScale(0.7);
		particle->setRotation(-(angle+angleVar));
		Point vocter=ccpForAngle((angle+angleVar)*3.1415926/180);
		int lange=rand()%300;
		this->addChild(particle,5);
		particle->runAction(
			Sequence::create(
					Spawn::createWithTwoActions
					(
						MoveBy::create(1+timeVar,Point((100+lange)*vocter.x,(100+lange)*vocter.y)),
						FadeOut::create(1+timeVar)
					),
					CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
					NULL
					)
				);
	}
}

//怪从action数组里出来挨个走
void GameLayer::run()
{
	//如果游戏结束则返回
	if(GameOver)
	{
		return;
	}
	//遍历防御塔菜单精灵设置菜单的透明度
	for(int k=0;k<arrMenu->count();k++)
	{
		//指向武器对象的指针
		Weapon* pWeapon =(Weapon*)arrMenu->objectAtIndex(k);
		//如果当前金币数小于安装武器时需要的金币
		if(money<pWeapon->value)
		{
			//将该防御塔设置为不可触控并边暗淡
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(100));
			pWeapon->runAction(act);

		}else
		{
			//如果当前金币数大于安装防御塔时需要的金币数，将该防御塔设置为可见可触控
			ActionInterval *act0=FadeTo::create(0.1f,(GLubyte)(255));
			pWeapon->runAction(act0);

		}
	}
	//如果升级武器不为空
	if(updateWeapon!=NULL)
	{
		setValue();
		//如果要升级的武器的等级已经为4，即最高级
		if(updateWeapon->level==4)
		{	//升级到头表示升级的金币不可见
			uMoneyL->setVisible(false);
		}
		//如果当前总金币数小于升级所需金币数或者已经升级到最高级或者升级的标志位为true
		if(money<updateWeapon->upValue||updateWeapon->level==4||updateWeapon->updateMark==true)
		{
			//声明一个指向存放金钱数组的指针
			Sprite* update = (Sprite*)arrSellUpdate->objectAtIndex(1);
			//将表示升级的箭头按钮设置为不可触控
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(100));
			update->runAction(act);
			//将表示升级的$文本标签设置为不可触控
			ActionInterval *act1=FadeTo::create(0.1f,(GLubyte)(100));
			uMoneyL->runAction(act1);
		}else
		{
			//否则将其设置为可触控可升级
			Sprite* update = (Sprite*)arrSellUpdate->objectAtIndex(1);
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(255));
			update->runAction(act);
			ActionInterval *act1=FadeTo::create(0.1f,(GLubyte)(255));
			uMoneyL->runAction(act1);
		}
	}

	//如果创建怪的标志位为true则走下面代码
	if(isfoundMonster)
	{
		//如果存放怪与存放怪action的数组都为0则走下面代码
		if(arrMon->count()==0&&arrAction->count()==0)
		{
			//回合数自加
			pass++;
			char a[6];//把int 型的分数转换成string型的 然后set
			snprintf(a, 6, "%d",pass+1);
			passL->setString(a);
			//不让怪运动
			isMonsterRun=false;
			//currMon++;
			//调用创建多个怪的方法
			foundMonsters();
		}
	}
	//如果存放怪action的数组为空则走下面代码
	if(arrAction->count()==0)
	{
		return;
	}

	//拿到起始点的坐标，横着看的x，y
	Point start = tmxLayer->positionAt(Point(source[1],source[0]));
	//将起点坐标转换到世界坐标系中
	Point startWorld = tmxLayer->convertToWorldSpaceAR(Point(start.x+trans.x,start.y+trans.y));
	//如果怪移动的标志位为true
	if(isMonsterRun)
	{
		//在起始点处添加两个特效，参数为坐标和时间间隔
		this->addParticle(startWorld,1.0);
	}
}

//怪移动到终点
void GameLayer::monsterRun(Node* node)
{
	Monsters* monster=(Monsters*) node ;
	//将怪设为可见
	monster->setVisible(true);
	//如果野怪已经走的步数等于原定路径的长度
	if(monster->way==monster->selfWay.size())
	{
		//如果生命值大于0
		if(ten>0)
		{
			//生命值自减
			ten--;
			//播放音效
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
		}
		//添加野怪到终点时的特效
		this->addParticle(monster->getPosition(),monster->id,3.0);
		char a[6];//把int 型的分数转换成string型的 然后set
		snprintf(a, 6, "%d",ten);
		//设置表示生命值的文本标签
		tenL->setString(a);
		//野怪到终点后移除精灵
		this->removeChild(monster);
		//移除数组中的怪
		arrMon->removeObject(monster);
		if(ten<=0)//失败
		{
			GameOver=true;
			this->loseGame();
		}
	}

	//如果怪已经走的路径长度小于要走路径的总长度
	if(monster->way<monster->selfWay.size())
	{
		//野怪走的步数自加
		monster->way++;
		//获取野怪当前的位置点
		Point crrPosition=monster->getPosition();
		//拿到下一个要走的点的坐标
		Point tarPosition=monster->selfWay.at(monster->selfWay.size()-monster->way);
		//拿到怪当前所处地图格子的行列号的x值
		int crrpoint_x=(int)(fromXYToColRow((int)crrPosition.x,(int)crrPosition.y).x);
		//拿到怪当前所处地图格子的行列号的y值
		int crrpoint_y=(int)(fromXYToColRow((int)crrPosition.x,(int)crrPosition.y).y);
		//拿到下一个要走的点的x值
		int tarpoint_x=(int)(fromXYToColRow((int)tarPosition.x,(int)tarPosition.y).x);
		//拿到下一个要走的点的y值
		int tarpoint_y=(int)(fromXYToColRow((int)tarPosition.x,(int)tarPosition.y).y);
		//定义一个点变量
		Point zhong;
		//拿到怪当前点与下一目标点的中点的x坐标
		zhong.x=(tarPosition.x+crrPosition.x)/2;
		//拿到怪当前点与下一目标点的中点的y坐标
		zhong.y=(tarPosition.y+crrPosition.y)/2;
		//定义一个用来表示时间的常量
		float time1=this->TIME_MAIN/2;
		float time2=this->TIME_MAIN/2;
		//如果为第二批野怪
		if(monster->id==2)
		{
			Point vacter;
			vacter.x=tarPosition.x-crrPosition.x;
			vacter.y=tarPosition.y-crrPosition.y;
			float dirction=ccpToAngle(vacter);
			monster->setRotation(-(dirction*180/3.1415926));
			monster->refresh(dirction);
		}
		//给定当前点与目标点的中点以及一个时间的MoveTo
		ActionInterval* act1=MoveTo::create(time1,zhong);
		//给定目标点及一个时间的MoveTo
		ActionInterval* act2=MoveTo::create(time2,tarPosition);
		Sequence* seqer = Sequence::create
								(
									act1,
									act2,
									CallFuncN::create(CC_CALLBACK_1(GameLayer::monsterRun,this)),
									NULL
								);
		monster->runAction(seqer);
	}
}

void GameLayer::removeSpriteAdd()
{
	//声明一个指向怪动作数组中最后一个怪的指针
	Monsters* mon = (Monsters*)arrAction->lastObject();
	//把怪添加到怪数组里
	arrMon->addObject(mon);
	//调用怪移动到终点的方法
	monsterRun(mon);
	//移动怪动作数组中的怪
	arrAction->removeLastObject();
	if(!DialogLayer::isParticle)
	{
		//删除特效精灵对象
		this->removeChild(cc,true);
	}
}

//创建多个怪的方法
void GameLayer::foundMonsters()
{
	//随机的出怪
	int id = pass%5;

	for(int i=0;i<10;i++)
	{
		//创建一个怪对象
		Monsters* mon = Monsters::create(id+1,way);
		//拿到怪起始位置的坐标,横着看的x,y
		Point start = tmxLayer->positionAt(Point(source[1],source[0]));
		//将起始点的坐标转换到世界坐标系中
		Point startWorld = tmxLayer->convertToWorldSpaceAR(Point(start.x+trans.x,start.y+trans.y));
		//设置怪的初始位置
		mon->setPosition(startWorld);
		//初始时将怪设为不可见
		mon->setVisible(false);
		//将怪添加到布景中
		this->addChild(mon,6);
		//把怪添加到action数组里
		arrAction->addObject(mon);
		if(id>2)//两个单独
		{
			break;
		}
	}
	//将创建怪的标志位设置为true
	isfoundMonster=true;
	//调用ready方法
	ready();
}

//攻击怪的方法
void GameLayer::attack()
{
	//如果怪不移动或者游戏结束
	if(!isMonsterRun||GameOver)
	{
		return;
	}
	//防御塔对怪的扫描
	for(int i=0;i<arrWeap->count();i++)
	{
		if(((Weapon*)arrWeap->objectAtIndex(i))->fire)
		{
			for(int j=0;j<arrMon->count();j++)
			{
				//拿到指向怪对象的指针
				Monsters* monster = (Monsters*)arrMon->objectAtIndex(j);
				//拿到指向防御塔的指针
				Weapon* weapon = (Weapon*)arrWeap->objectAtIndex(i);
				//获取防御塔当前的位置坐标
				Point pointWeapon=weapon->getPosition();
				//获取野怪当前的位置坐标
				Point pointMonster=monster->getPosition();
				//定义一个向量   为了使防御塔指向怪
				Point anglePoint;
				anglePoint.x=pointMonster.x-pointWeapon.x;
				anglePoint.y=pointMonster.y-pointWeapon.y;
				//根据向量求向量对应的角度
				float angle = ccpToAngle(anglePoint);
				//求距离
				float distance = sqrt((pointMonster.x-pointWeapon.x)*(pointMonster.x-pointWeapon.x)
						+(pointMonster.y-pointWeapon.y)*(pointMonster.y-pointWeapon.y));
				//定义一个设置子弹离开防御塔的位置的变量
				Point bulletPoint;
				//子弹出防御塔的位置  不是防御塔中央
				bulletPoint.x=(pointMonster.x-pointWeapon.x)*25/distance+pointWeapon.x;
				bulletPoint.y=(pointMonster.y-pointWeapon.y)*25/distance+pointWeapon.y;
				//用Cocos2dx提供的函数求向量的长度
				float lengthVector=ccpLength(Point(pointWeapon.x-pointMonster.x,pointWeapon.y-pointMonster.y));
				//如果所求长度在防御塔的攻击范围内
				if(lengthVector<=weapon->confines)
				{
					//防御塔旋转
					weapon->setRotation(-(angle*180/3.1415926));
					//改变防御塔角度属性
					weapon->angle=angle*180/3.1415926;
					//第一个防御塔攻击怪时调用减血的方法
					if(weapon->id==1)
					{
						CocosDenshion::SimpleAudioEngine::getInstance()->playEffect
						(
							"sound/sf_minigun_hit.mp3"
						);
						fireBulletOne(i,j,angle,bulletPoint,lengthVector);
						break;
					}
					//第二个防御塔攻击怪时调用减血的方法
					else if(weapon->id==2)
					{
						CocosDenshion::SimpleAudioEngine::getInstance()->playEffect
						(
							"sound/sf_laser_beam.mp3"
						);
						fireBulletTwo(i,j,angle,bulletPoint);
						weapon->fireing();
						break;
					}
					//第三个防御塔攻击怪时调用减血的方法
					else if (weapon->id==3)
					{
						CocosDenshion::SimpleAudioEngine::getInstance()->playEffect
						(
							"sound/sf_rocket_launch.mp3"
						);
						fireBulletThree(i,j,angle,bulletPoint);
						weapon->fireing();
						break;
					}

					//第四个防御塔的攻击
					else if(weapon->id==4)
					{
						//初始化防御塔的旋转角度
						weapon->setRotation(0);
						//初始化防御塔的角度
						weapon->angle=0;
						//创建一个子弹对象
						Sprite* bullet = Sprite::create("pic/ring_blue.png");
						float bulletX=bullet->getContentSize().width/2;
						//扩大的倍数
						float scaleX=weapon->confines/bulletX;
						//设置子弹的位置
						bullet->setPosition(weapon->getPosition());
						//将子弹添加到布景中
						this->addChild(bullet,4);
						//设置子弹的透明度
						bullet->setOpacity(80);
						//设置子弹为可见
						bullet->setVisible(true);
						//设置一个精灵的特效
						ActionInterval *act=ScaleTo::create(1.5f,scaleX);
						ActionInterval *activeFade=FadeOut::create(2.0f);
						bullet->runAction(
							Sequence::create(
						Spawn::createWithTwoActions
												(
													act,
													activeFade
												),
								CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
								NULL
								)

						);
						weapon->fireing();
						//怪掉血
						monster->cutBlood(weapon->hurt);
						//如果怪的血小于0
						if(monster->blood<=0)
						{
							//拿到怪的位置
							Point pointMonster=monster->getPosition();
							//拿到怪的路径
							vector <Point > tempSelfWay = monster->selfWay;
							//拿到怪已经走的路径
							int tempWay = monster->way;
							//如果怪的id为3
							if(monster->id==3)
							{
								//设置锚点
								Point Achorpoint=(Point(0.5,0.4));
								for(int i=0;i<2;i++)
								{
									//创建两个新的怪
									Monsters* mon = Monsters::create(6,tempSelfWay);
									//新怪拿到老怪已经走的路径
									mon->way=tempWay;
									//设置怪的位置
									mon->setPosition(pointMonster);
									//设置怪的锚点
									mon->setAnchorPoint(Achorpoint);
									Achorpoint=Point(Achorpoint.x+0.2,Achorpoint.y+0.2);
									//把怪添加到怪数组里
									arrMon->addObject(mon);
									//将怪添加到布景中
									this->addChild(mon,6);
									//调用怪移动到终点的方法
									monsterRun(mon);
								}
							}
							//定义一个临时存放怪死后金币的变量
							int tempMoney = monster->id*10;
							//总的金币数加上临时所得的等于当前总的金币
							money+=tempMoney;
							//定义一个临时存放怪死后所得分数的变量
							int tempScore = monster->id*15;
							//总的分数加上临时所得的等于当前总的分数
							score+=tempScore;
							//怪死时添加一个特效
							this->addParticle1(monster->getPosition(),monster->id,1);
							//播放音效
							CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
							//删除场景中的怪
							this->removeChild(monster);
							//删除怪数组中的怪
							arrMon->removeObject(monster);
							std::string overStr = "$";
							char a[6];//把int 型的分数转换成string型的 然后set
							snprintf(a, 6, "%d",money);
							//更新显示金币的文本标签
							moneyL->setString((overStr+a).c_str());
							char b[6];//把int 型的分数转换成string型的 然后set
							snprintf(b, 6, "%d",score);
							//更新显示总分数的文本标签
							scoreL->setString(b);
						}
					}
				}
			}
		}
	}
}

//第一个防御塔攻击
void GameLayer::fireBulletOne(int weap,int target,float dirction,Point position,float lengthVector)
{
	//判断存放
	if(this->bulletData[target]==0)
	{
		this->bulletData[target]=1;
	}
	//获取要攻击的怪对象
	Monsters* monster = (Monsters*)arrMon->objectAtIndex(target);
	//获取防御塔的信息
	Weapon* weapon = (Weapon*)arrWeap->objectAtIndex(weap);
	//发射子弹的数量与防御塔的等级有关
	int count[4]={1,2,3,3};
	//攻击子弹的延迟
	float delay[3]={0.01,0.1,0.05};

	//根据防御塔的等级来确定要发射子弹的数量和位置
	Point* positionByLevel=new Point[count[weapon->level-1]];

	//如果防御塔的等级为1，确定每发子弹的初始位置
	if(weapon->level==1)
	{
		positionByLevel[0]=position;
	}
	//如果防御塔的等级为2，发射两发子弹
	else if(weapon->level==2)
	{
		//设置两发子弹偏离10度
		Point vacter1=ccpForAngle(dirction+(10*3.1415926/180));
		Point vacter2=ccpForAngle(dirction-(10*3.1415926/180));
		vacter1=ccpNormalize(vacter1);
		vacter2=ccpNormalize(vacter2);
		//确定子弹的初始发射位置
		positionByLevel[0]=ccpAdd(weapon->getPosition(),ccpMult(vacter1,26));
		positionByLevel[1]=ccpAdd(weapon->getPosition(),ccpMult(vacter2,26));
	}
	//如果防御塔的等级为3级或者4级
	else if(weapon->level==3||weapon->level==4)
	{
		//设置三发子弹偏离10度
		Point vacter1=ccpForAngle(dirction+(20*3.1415926/180));
		Point vacter2=ccpForAngle(dirction-(20*3.1415926/180));
		vacter1=ccpNormalize(vacter1);
		vacter2=ccpNormalize(vacter2);
		//确定子弹的初始发射位置
		positionByLevel[0]=position;
		positionByLevel[1]=ccpAdd(weapon->getPosition(),ccpMult(vacter1,26));
		positionByLevel[2]=ccpAdd(weapon->getPosition(),ccpMult(vacter2,26));
	}

	//发射子弹，子弹直线移动，调节速度可以弥补偏差
	for(int i=0;i<count[weapon->level-1];i++)
	{
		//创建一个子弹对象
		BulletSprite* bullet = BulletSprite::create("pic/bullet.png",weapon->hurt,target);
		//设置子弹的位置
		bullet->setPosition(positionByLevel[i].x,positionByLevel[i].y);
		//将子弹添加到布景中
		this->addChild(bullet,4);
		//定义一个时间变量
		float timeTo=lengthVector/weapon->confines;
		//声明一个动作
		ActionInterval* act=MoveTo::create(timeTo/5,monster->getPosition());
		//顺序执行
		Sequence* seqer = Sequence::create
						(
							DelayTime::create(delay[i]),
							act,
							CallFuncN::create(CC_CALLBACK_1(GameLayer::cutBloodOne,this)),
								NULL
						);
		bullet->runAction(seqer);
		weapon->fireing();
	}
}

//怪掉血的方法
void GameLayer::cutBloodOne(Node*node)
{
	//拿到子弹攻击的目标怪
	Monsters* monster=(Monsters*)arrMon->objectAtIndex(((BulletSprite*)node)->target);
	//调用cutBlood方法让怪减血
	monster->cutBlood(((BulletSprite*)node)->hurt);
	//判断怪的血量是否小于等于0
	if(monster->blood<=0)
	{
		//定义一个临时变量存放怪的路径
		vector <Point > tempSelfWay = monster->selfWay;
		//定义一个临时变量存放父怪已经走的路径
		int tempWay = monster->way;
		//获取怪的位置
		Point pointMonster=monster->getPosition();
		//第三种怪死后会创建两个新怪
		if(monster->id==3)
		{
			Point Achorpoint=(Point(0.5,0.4));
			//创建两个新的怪对象
			for(int i=0;i<2;i++)
			{
				//创建怪对象
				Monsters* mon = Monsters::create(6,tempSelfWay);
				//拿到父怪已经走的路径
				mon->way=tempWay;
				//设置新怪的位置
				mon->setPosition(pointMonster);
				//设置锚点
				Achorpoint=Point(Achorpoint.x+0.2,Achorpoint.y+0.2);
				//设置新怪的锚点
				mon->setAnchorPoint(Achorpoint);
				//把怪添加到怪数组里
				arrMon->addObject(mon);
				//将新建的两个怪添加到布景中
				this->addChild(mon,6);
				//调用怪移动到终点的方法
				monsterRun(mon);
			}
		}
		//定义一个临时变量存放怪死时得到的金币
		int tempMoney = monster->id*10;
		//总的金币数加上怪死所得的金币数等于当前的总金币数
		money+=tempMoney;
		//定义一个临时变量存放怪死时得到的分数
		int tempScore = monster->id*15;
		//总的分数加上怪死时得到的分数等于当前总的分数
		score+=tempScore;
		//添加怪死时的特效
		this->addParticle1(monster->getPosition(),monster->id,1.0);
		//添加音效
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
		//删除怪对象
		this->removeChild(monster);
		//删除数组中的怪
		arrMon->removeObject(monster);
		char a[6];//把int 型的分数转换成string型的 然后set
		snprintf(a, 6, "%d",money);
		std::string overStr = "$";
		//更新显示当前总金币数的文本标签
		moneyL->setString((overStr+a).c_str());
		char b[6];//把int 型的分数转换成string型的 然后set
		snprintf(b, 6, "%d",score);
		//更新显示当前总分数的文本标签
		scoreL->setString(b);
	}
	//删除子弹对象
	this->removeChild(node);
}

//第二个防御塔的攻击
void GameLayer::fireBulletTwo(int weap,int target,float dirction,Point position)
{
	//拿到目标野怪
	Monsters* monster = (Monsters*)arrMon->objectAtIndex(target);
	//拿到防御塔
	Weapon* weapon = (Weapon*)arrWeap->objectAtIndex(weap);
	//获取野怪当前的位置
	Point pointMonster=monster->getPosition();
	//获取防御塔当前的位置
	Point pointWeapon=weapon->getPosition();
	//存放子弹精灵图片的数组
	std::string bullet[4]={"pic/weapon2-1.png","pic/weapon2-2.png","pic/weapon2-3.png","pic/weapon2-4.png"};
	//创建一个子弹精灵对象
	bullet1 = Sprite::create(bullet[weapon->level-1].c_str());
	//设置子弹的锚点
	bullet1 ->setAnchorPoint(Point(0,0.5));
	//设置子弹的位置
	bullet1->setPosition(position);
	//将子弹添加到布景中
	this->addChild(bullet1,4);
	//发射子弹的动作特效
	bullet1->setRotation(-(dirction*180/3.1415926));
	bullet1 -> runAction(
				Sequence::create(
						DelayTime::create(0.1),
						CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
						NULL
						)
				);
	//遍历存放怪的数组，计算所有被激光碰到的怪
	for(int k=0;k<arrMon->count();k++)
	{
		//拿到数组中的怪
		Monsters * mon= (Monsters*)arrMon->objectAtIndex(k);
		float x1=position.x;
		float y1=position.y;
		//拿到怪当前的x,y坐标
		float x2=pointMonster.x;
		float y2=pointMonster.y;
		//获取怪当前的x,y坐标
		float x3=(mon->getPosition()).x;
		float y3=(mon->getPosition()).y;
		float slope = (y2-y1)/(x2-x1);
		//判断是否在激光侧
		if((x2-x1)*(x3-x1)>0&&(y2-y1)*(y3-y1)>0)
		{
			float monsterDistance=(y1-y2-slope*x1+slope*x3)/
					(sqrt(slope*slope+1));
			if(monsterDistance<30)
			{
				//调用怪减血的方法
				mon->cutBlood(weapon->hurt);
				//添加一个怪被击中时的特效
				this->addParticle(mon->getPosition(),monster->id,0.5,dirction*180/3.1415926);
			}
		}
		//如果怪没血，则删除
		if(mon->blood<=0)
		{	//获取野怪的位置
			Point pointMonster=monster->getPosition();
			//获取怪当前走的路径
			vector <Point > tempSelfWay = mon->selfWay;
			//获取怪已经走的路径
			int tempWay = mon->way;
			//如果怪的id为3
			if(monster->id==3)
			{
				//设置怪的锚点
				Point Achorpoint=(Point(0.5,0.4));
				for(int i=0;i<2;i++)
				{
					//创建2个新的怪
					Monsters* mon = Monsters::create(6,tempSelfWay);
					//将已经走的路径传给新怪
					mon->way=tempWay;
					//设置新怪的位置
					mon->setPosition(pointMonster);
					//设置新怪的锚点
					mon->setAnchorPoint(Achorpoint);
					Achorpoint=Point(Achorpoint.x+0.2,Achorpoint.y+0.2);
					//把怪添加到怪数组里
					arrMon->addObject(mon);
					//将怪添加到布景中
					this->addChild(mon,6);
					//调用怪移动到终点的方法
					monsterRun(mon);
				}
			}
			//怪死后后得到对应的金币
			int tempMoney = mon->id*10;
			//总的金币数要加上怪死后得到的金币数
			money+=tempMoney;
			//怪死后会得到对应的分数
			int tempScore = monster->id*15;
			//总的分数要加上怪死后的分数
			score+=tempScore;
			//添加特效
			this->addParticle1(mon->getPosition(),monster->id,3.0);
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
			//移除怪对象
			this->removeChild(mon);
			//移除数组中的怪
			arrMon->removeObject(mon);
			std::string overStr = "$";
			char a[6];//把int 型的分数转换成string型的 然后set
			snprintf(a, 6, "%d",money);
			moneyL->setString((overStr+a).c_str());
			char b[6];//把int 型的分数转换成string型的 然后set
			snprintf(b, 6, "%d",score);
			scoreL->setString(b);
		}
	}
	//调用Weapon类中发射子弹的方法
	weapon->fireing();
}

//第三个防御塔的攻击
void GameLayer::fireBulletThree(int weap,int target,float dirction,Point position)
{
	//从存放怪的数组中拿到怪
	Monsters* monster = (Monsters*)arrMon->objectAtIndex(target);
	//从存放防御塔的数组中拿到防御塔
	Weapon* weapon = (Weapon*)arrWeap->objectAtIndex(weap);
	//确定发射子弹的数量
	int count[4]={1,2,2,3};

	Point* positionByLevel=new Point[count[weapon->level-1]];

	float angle[count[weapon->level-1]];
	//如果当前防御塔的等级为1
	if(weapon->level==1)
	{
		Point vacter1=ccpForAngle(dirction-(15*3.1415926/180));
		vacter1=ccpNormalize(vacter1);
		positionByLevel[0]=ccpAdd(weapon->getPosition(),ccpMult(vacter1,36));
		angle[0]=dirction;
	}
	//如果当前防御塔的等级为3或者为2
	else if(weapon->level==3||weapon->level==2)
	{
		Point vacter1=ccpForAngle(dirction+(15*3.1415926/180));
		Point vacter2=ccpForAngle(dirction-(15*3.1415926/180));
		vacter1=ccpNormalize(vacter1);
		vacter2=ccpNormalize(vacter2);
		positionByLevel[0]=ccpAdd(weapon->getPosition(),ccpMult(vacter1,36));
		positionByLevel[1]=ccpAdd(weapon->getPosition(),ccpMult(vacter2,36));
		angle[0]=dirction+(15*3.1415926/180);
		angle[1]=dirction-(15*3.1415926/180);
	}
	//如果防御塔当前的等级为4
	else if(weapon->level==4)
	{
		Point vacter1=ccpForAngle(dirction+(45*3.1415926/180));
		Point vacter2=ccpForAngle(dirction-(45*3.1415926/180));
		vacter1=ccpNormalize(vacter1);
		vacter2=ccpNormalize(vacter2);
		positionByLevel[0]=position;
		positionByLevel[1]=ccpAdd(weapon->getPosition(),ccpMult(vacter1,36));
		positionByLevel[2]=ccpAdd(weapon->getPosition(),ccpMult(vacter2,36));
		angle[0]=dirction;
		angle[1]=dirction+(45*3.1415926/180);
		angle[2]=dirction-(45*3.1415926/180);
	}
	//根据当前防御塔的等级来设置第三个防御塔发射子弹的数量
	for(int i=0;i<count[weapon->level-1];i++)
	{
		//创建一个子弹对象
		BulletSprite* bullet = BulletSprite::create("pic/bullet2.png",weapon->hurt,target);
		//设置子弹的位置
		bullet->setPosition(positionByLevel[i]);
		//设置子弹的旋转
		bullet->setRotation(-(angle[i]*180/3.1415926));
		//设置子弹的旋转
		bullet->angle = -(angle[i]*180/3.1415926);
		//将子弹添加到布景中
		this->addChild(bullet,4);
		//将子弹添加到子弹数组中
		arrBullet->addObject(bullet);
	}
}

//发射子弹的方法
void GameLayer::runBullet()
{
	//如果存放子弹的数组的长度为0或者游戏结束则返回
	if(arrBullet->count()==0||GameOver)
	{
		return;
	}
	//跟踪算法
	for(int i=0;i<arrBullet->count();i++)
	{
		//获得子弹对象
		BulletSprite *bullet = (BulletSprite*)arrBullet->objectAtIndex(i);
		//变化向量
		Point vecter;
		//获取子弹的位置坐标
		Point position=bullet->getPosition();
		//如果没有怪则变化向量设为0
		if(arrMon->count()==0)
		{
			vecter.x=0;
			vecter.y=0;
		}
		//如果子弹击中怪数组中的目标对象
		else if(bullet->target>arrMon->count())
		{
			//获取怪对象
			Monsters* monster=(Monsters*)arrMon->objectAtIndex(1);
			//野怪当前位置的横坐标-子弹当前位置的横坐标
			vecter.x=monster->getPosition().x-bullet->getPosition().x;
			//野怪当前位置的纵坐标-子弹当前位置的纵坐标
			vecter.y=monster->getPosition().y-bullet->getPosition().y;
			//如果计算出子弹与防御塔的距离小于20则击中目标怪
			if(ccpLength(vecter)<20)
			{
				//野怪减血
				monster->cutBlood(bullet->hurt);
				//如果怪已经没有血了，则删除怪
				if(monster->blood<=0)
				{
					//获取怪当前位置的坐标点
					Point pointMonster=monster->getPosition();
					//将路径重新给怪
					vector <Point > tempSelfWay = monster->selfWay;
					//定义怪的临时路径的长度
					int tempWay = monster->way;
					//如果怪的id为3
					if(monster->id==3)
					{
						//定义锚点
						Point Achorpoint=(Point(0.5,0.4));
						//当第3中怪别打死后会产生两个新的小怪
						for(int i=0;i<2;i++)
						{
							//创建新的小怪
							Monsters* mon = Monsters::create(6,tempSelfWay);
							//把当前已经走的路径给新的怪
							mon->way=tempWay;
							//设置怪的位置
							mon->setPosition(pointMonster);
							//设置锚点
							mon->setAnchorPoint(Achorpoint);
							//设置锚点
							Achorpoint=Point(Achorpoint.x+0.2,Achorpoint.y+0.2);
							//把怪添加到怪数组里
							arrMon->addObject(mon);
							//将怪添加到布景中
							this->addChild(mon,6);
							//调用怪移动到终点的方法
							monsterRun(mon);
						}
					}
					//拿到怪死后得到的金币数
					int tempMoney = monster->id*10;
					//总的金币数加上怪死后的金币数等于当前金币数
					money+=tempMoney;
					//拿到怪死后得到的分数
					int tempScore = monster->id*15;
					//总的分数加上怪死后的分数等于当前分数
					score+=tempScore;
					//添加怪死时的特效
					this->addParticle1(monster->getPosition(),monster->id,3.0);
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
					//删除怪对象
					this->removeChild(monster);
					//删除数组中的怪
					arrMon->removeObject(monster);
					char a[6];//把int 型的分数转换成string型的 然后set
					snprintf(a, 6, "%d",money);
					std::string overStr = "$";
					//设置金币数
					moneyL->setString((overStr+a).c_str());
					char b[6];//把int 型的分数转换成string型的 然后set
					snprintf(b, 6, "%d",score);
					//设置分数
					scoreL->setString(b);
				}
				//删除子弹数组中的子弹
				arrBullet->removeObject(bullet);
				//删除子弹精灵
				this->removeChild(bullet);

				return;
			}
		}
		else
		{
			//获取子弹指向的野怪对象
			Monsters* monster=(Monsters*)arrMon->objectAtIndex(bullet->target);
			//野怪当前位置的横坐标-子弹当前位置的横坐标
			vecter.x=monster->getPosition().x-bullet->getPosition().x;
			//野怪当前位置的纵坐标-子弹当前位置的纵坐标
			vecter.y=monster->getPosition().y-bullet->getPosition().y;
			//如果计算出子弹与防御塔的距离小于20则击中目标怪
			if(ccpLength(vecter)<20)
			{
				monster->cutBlood(bullet->hurt);
				//如果怪没血，则删除
				if(monster->blood<=0)
				{
					//获取怪当前的位置
					Point pointMonster=monster->getPosition();
					//拿到怪当前走的路径
					vector <Point > tempSelfWay = monster->selfWay;
					//拿到怪已经走的路径
					int tempWay = monster->way;
					//第三种怪死后会创建两个新怪
					if(monster->id==3)
					{
						//设置锚点
						Point Achorpoint=(Point(0.5,0.4));
						//通过for循环创建两个新的怪
						for(int i=0;i<2;i++)
						{
							//根据id和路径创建新的怪
							Monsters* mon = Monsters::create(6,tempSelfWay);
							//拿到大怪已经走的路
							mon->way=tempWay;
							//设置新怪的位置
							mon->setPosition(pointMonster);
							//设置锚点
							mon->setAnchorPoint(Achorpoint);
							Achorpoint=ccp(Achorpoint.x+0.2,Achorpoint.y+0.2);
							//把新怪添加到怪数组里
							arrMon->addObject(mon);
							//将新怪添加到布景中
							this->addChild(mon,6);
							//调用怪移动到终点的方法
							monsterRun(mon);
						}
					}
					//定义一个临时存放怪死后所得金币的变量
					int tempMoney = monster->id*10;
					//更新总的金币数
					money+=tempMoney;
					//定义一个临时存放怪死后所得分数的变量
					int tempScore = monster->id*15;
					//更新总的分数
					score+=tempScore;
					//添加怪死时的特效
					this->addParticle1(monster->getPosition(),monster->id,3.0);
					//播放怪死时的音效
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
					//删除怪对象
					this->removeChild(monster);
					//删除怪数组中的怪
					arrMon->removeObject(monster);
					char a[6];//把int 型的分数转换成string型的 然后set
					snprintf(a, 6, "%d",money);
					std::string overStr = "$";
					moneyL->setString((overStr+a).c_str());
					char b[6];//把int 型的分数转换成string型的 然后set
					snprintf(b, 6, "%d",score);
					scoreL->setString(b);
				}
				//删除子弹数组中的子弹
				arrBullet->removeObject(bullet);
				//删除子弹对象
				this->removeChild(bullet);

				return;
			}
		}
		//判断子弹是否超出屏幕，如果超出则将其删除
		if(position.x>800||position.y>480||position.x<0||position.y<0)
		{
			//删除子弹数组中的子弹
			arrBullet->removeObject(bullet);
			//删除子弹对象
			this->removeChild(bullet);

			return;
		}
		Point vector = ccpForAngle ((bullet->angle)*3.1415926/180);
		Point speed=ccpMult(ccpNormalize(ccp(vecter.x+vector.x/15,vecter.y+vector.y/15)),6);
		bullet->setPosition(ccpAdd(bullet->getPosition(),speed));
		float angle = ccpToAngle(speed);
		bullet->setRotation(-(angle*180/3.1415926));
		//设置子弹的角度
		bullet->angle=angle;
	}
}

//删除所有声明过的精灵对象
void GameLayer::removeSprite(Node*node)
{
	this->removeChild(node);
}

//添加防御塔精灵对象
void GameLayer::addMenuSprite()
{
	//添加表示金币符号的精灵对象
	Sprite* sell = Sprite::create("pic/sell.png");
	//四种武器精灵按钮
	onePlayer = Weapon::create("pic/button_0.png",1);
	twoPlayer = Weapon::create("pic/button_1.png",2);
	threePlayer = Weapon::create("pic/button_2.png",3);
	fourPlayer = Weapon::create("pic/button_3.png",4);
	//创建表示升级箭头的精灵对象
	Sprite* update = Sprite::create("pic/update.png");
	//设置升级按钮为不可见
	update->setVisible(false);
	//设置表示卖掉武器的收入金币按钮为不可见
	sell->setVisible(false);

	//设置六个精灵对象的位置
	sell->setPosition(Point(730,68));
	onePlayer->setPosition(Point(750,136));
	twoPlayer->setPosition(Point(750,204));
	threePlayer->setPosition(Point(750,272));
	fourPlayer->setPosition(Point(750,340));
	update->setPosition(Point(730,408));

	//将4个武器菜单按钮添加到布景中
	this->addChild(onePlayer, 3);
	this->addChild(twoPlayer, 3);
	this->addChild(threePlayer, 3);
	this->addChild(fourPlayer, 3);
	//将精灵对象添加到布景中
	this->addChild(sell, 3);
	this->addChild(update, 3);

	//将4个武器菜单按钮添加到相应的数组里
	arrMenu->addObject(onePlayer);
	arrMenu->addObject(twoPlayer);
	arrMenu->addObject(threePlayer);
	arrMenu->addObject(fourPlayer);
	//将金币精灵对象添加到相应的数组里
	arrSellUpdate->addObject(sell);
	//将升级精灵对象添加到布景中
	arrSellUpdate->addObject(update);
}

//添加所有label
void GameLayer::addLabel()
{
	//创建一个tempScore文本标签（临时分数）
	scoreL = LabelTTF::create("0", "Arial",28);
	//创建一个特效并播放
	ActionInterval *act= RotateBy::create(0.1,-90);
	scoreL->runAction(act);
	//设置标签字体的颜色
	scoreL->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	scoreL->setPosition(Point(40,60));
	//将文本标签添加到布景中
	this->addChild(scoreL,3);

	//创建一个用于显示回合数的文本标签
	passL = LabelTTF::create("1", "Arial",28);
	//设置动作的间隔
	ActionInterval *act1= RotateBy::create(0.1,-90);
	passL->runAction(act1);
	//设置标签字体的颜色
	passL->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	passL->setPosition(Point(40,240));
	//将文本标签添加到布景中
	this->addChild(passL,3);

	//创建一个用于显示金钱的文本标签
	moneyL = LabelTTF::create("$280", "Arial",28);
	//创建一个特效并播放
	ActionInterval *act2= RotateBy::create(0.1,-90);
	moneyL->runAction(act2);
	//设置标签字体的颜色
	moneyL->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	moneyL->setPosition(Point(40,410));
	//将文本标签添加到布景中
	this->addChild(moneyL,3);

	//创建一个用于显示生命值的ten文本标签
	tenL = LabelTTF::create("18", "Arial",28);
	//创建一个特效并播放
	ActionInterval *act3= RotateBy::create(0.1,-90);
	tenL->runAction(act3);
	//设置标签字体的颜色
	tenL->setColor (ccc3(255,255,255));
	//横着看的x，y
	Point tar = tmxLayer->positionAt(Point(targetAll[0][1],targetAll[0][0]));
	//将目标点的坐标转换到世界坐标系中
	Point targetWorld = tmxLayer->convertToWorldSpaceAR(Point(tar.x+trans.x,tar.y+trans.y));
	//设置文本标签的位置
	tenL->setPosition(targetWorld);
	//将文本标签添加到布景中
	this->addChild(tenL,6);

	//创建一个updateMoney文本标签
	uMoneyL = LabelTTF::create("$", "Arial",28);
	//将文本标签设置为不可见
	uMoneyL->setVisible(false);
	//创建并播放一个特效
	ActionInterval *act4= RotateBy::create(0.1,-90);
	uMoneyL->runAction(act4);
	//设置标签字体的颜色
	uMoneyL->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	uMoneyL->setPosition(Point(760,408));
	//将文本标签添加到布景中
	this->addChild(uMoneyL,3);

	//创建一个sellMoney文本标签
	sMoneyL = LabelTTF::create("$", "Arial",28);
	//将文本标签设置为不可见
	sMoneyL->setVisible(false);
	//创建一个特效并播放
	ActionInterval *act5= RotateBy::create(0.1,-90);
	sMoneyL->runAction(act5);
	//设置标签字体的颜色
	sMoneyL->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	sMoneyL->setPosition(Point(760,68));
	//将文本标签添加到布景中
	this->addChild(sMoneyL,3);
}

//野怪到终点时的特效
void GameLayer::addParticle2(Point point,float time)
{
	if(!DialogLayer::isParticle)
	{
		return;
	}
	int countVar=rand()%10;
	int angle=0;
	for(int i=0;i<55+countVar;i++)
	{
		float timeVar=(rand()%10)/10.0;
		float angleVar=(rand()%10)/10.0;
		//创建一个发光特效的精灵
		Sprite* particle= Sprite::create("pic/white.png");
		//设置锚点
		particle->setAnchorPoint(Point(0,0.5));
		//设置位置
		particle->setPosition(point);
		//设置大小
		particle->setScale(1.0);
		particle->setRotation(-(angle+angleVar));
		Point vocter=ccpForAngle((angle+angleVar)*3.1415926/180);
		int lange=rand()%200;
		angle+=11;
		//将精灵对象添加到布景中
		this->addChild(particle,5);
		//顺序执行动作
		particle->runAction(
			Sequence::create(
					Spawn::createWithTwoActions
					(
						MoveBy::create(1+timeVar,Point((200+lange)*vocter.x,(200+lange)*vocter.y)),
						FadeOut::create(1+timeVar)
					),
					CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
					NULL
			)
		);
	}

	//创建一个特效精灵对象
	cc = Sprite::create("pic/fire1.png");
	//声明一个渐现的动作
	ActionInterval *act1=FadeIn::create(time*4/3);
	//声明一个渐隐的动作
	ActionInterval *activeFade1=FadeOut::create(time*4/3);
	//设置动作执行的位置
	cc->setPosition(point);
	//设置精灵的大小
	cc->setScale(6.0);
	//将精灵添加到布景中
	this->addChild(cc,6);
	//顺序执行动作
	cc->runAction(Sequence::create(
			Spawn::createWithTwoActions
						(
							act1,
							activeFade1
						),
						CallFuncN::create(CC_CALLBACK_1(GameLayer::removeSprite,this)),
						NULL
			)
	);
}

//游戏结束时调用的方法
void GameLayer::loseGame()
{
	//播放游戏结束的音效
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect
	(
		"sound/sf_game_over.mp3"
	);
	//调用计分板
	AchieveLayer* al = new AchieveLayer();
	al->saveScore(score);
	//遍历防御塔数组
	for(int i=0;i<arrWeap->count();i++)
	{
		//声明一个存放4个时间的一维数组
		int a[4]={4,2,1,3};
		//拿到当前的防御塔对象
		Weapon* weap=(Weapon*)arrWeap->objectAtIndex(i);
		//添加爆炸的特效
		this->addParticle1(weap->getPosition(),a[weap->id-1],5.0);
		//删除防御塔对象
		this->removeChild(weap,true);
	}
	//删除防御塔数组中的所有对象
	arrWeap->removeAllObjects();

	//遍历存放怪的数组
	for(int i=0;i<arrMon->count();i++)
	{
		//拿到当前的所有怪
		Monsters* mon=(Monsters*)arrMon->objectAtIndex(i);
		//添加爆炸特效
		this->addParticle1(mon->getPosition(),mon->id,5.0);
		//删除怪对象
		this->removeChild(mon,true);
	}
	//删除存放怪数组中的所有怪对象
	arrMon->removeAllObjects();
	//删除所有存放在怪动作数组中的对象
	arrAction->removeAllObjects();

	//遍历所有存放子弹的数组
	for(int i=0;i<arrBullet->count();i++)
	{
		//拿到子弹对象
		BulletSprite* bullet=(BulletSprite*)arrBullet->objectAtIndex(i);
		//添加爆炸特效
		this->addParticle1(bullet->getPosition(),1,5.0);
		//删除子弹精灵
		this->removeChild(bullet,true);
	}
	//删除子弹数组中的所有对象
	arrBullet->removeAllObjects();
	//添加特效
	this->addParticle2(endWorld,5.0);

    //创建一个精灵对象，“最高分数”
    Sprite *gameOverSprite = Sprite::create("pic/gameOver.png");
	//设置精灵对象的位置
    gameOverSprite->setPosition(Point(850,240));
	//将背景精灵添加到布景中
	this->addChild(gameOverSprite,GAME_LEVEL_CGQ+4);
	gameOverSprite->runAction(MoveTo::create(4.0f,Point(400,240)));
	//停止背景音乐的播放
	CocosDenshion::SimpleAudioEngine::getInstance()->stopBackgroundMusic
	(
		"sound/bg_music.mp3"
	);
}
