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
static Dictionary s_dic_;

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

bool GameLayer::is_pause_=false;
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
priority_queue<INTPARR,vector<INTPARR>,cmp>* astar_queue;
GameLayer::GameLayer(){}
//析构函数
GameLayer::~GameLayer()
{
	//释放内存
	FreeMemory();
	//释放地图
	for(int i=0;i<row_;i++)
	{
		delete []map_data_[i];
	}
	delete []map_data_;
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
	auto gb_sprite = Sprite::create("pic/back.png");
	//设置精灵对象的位置
	gb_sprite->setPosition(Point(400,240));
	//将精灵添加到布景中
	this->addChild(gb_sprite,BACKGROUND_LEVEL_CGQ);

	//加载音效
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect
	(
		"sound/sf_button_press.mp3"
	);

    //广度优先A*算法
	astar_queue = NULL;
	hm_ = NULL;
	//获取当前屏幕的大小
	auto win_size = Director::getInstance()->getWinSize();
	//加载TMX地图
	auto map = TMXTiledMap::create("map/MyTilesMap"+
			StringUtils::format("%d",ChooseLayer::modeLevel)+".tmx");
	//设置地图的锚点
    map->setAnchorPoint(Point(0,1.0));
	//设置地图位置
    map->setPosition(Point(0, win_size.height-3));
	//将地图添加到布景中
	addChild(map,BACKGROUND_LEVEL_CGQ, kTagTileMap);
	//获得地图的宽度和高度
	int map_width = map->getMapSize().width;
	int map_height = map->getMapSize().height;
	//将地图的宽度和高度设置为二维数组的行和列
	row_ = map_width;
	col_ = map_height;
	//创建动态二维数组
	map_data_ = new int*[row_];
	for(int i = 0; i<row_; i++)
	{
		map_data_[i] = new int[col_];
	}

	//得到地图中的layer
	tmx_layer_ = map->getLayer("Layer 0");
	//获得一个图素中的属性值
	for(int i=0; i<row_; i++)
	{
		for(int j=0; j<col_; j++)
		{
			//得到layer中每一个图块的gid
			unsigned int gid = tmx_layer_->getTileGIDAt(Point(i, j));
			//通过gid得到该图块中的属性集,属性集中是以键值对的形式存在的
			auto tile_dic = map->getPropertiesForGID(gid);
			//通过键得到value
			const String mvalue = tile_dic.asValueMap()["value"].asString();
			//将mvalue转换成int变量
			int mv = mvalue.intValue();
			//初始化地图中的数据
			map_data_[i][j] = mv;

		}
	}
	//设置抗锯齿，如果需要对地图进行放大或缩小时，就可以使用
	auto children = tmx_layer_->getChildren();
	SpriteBatchNode* child = NULL;
	for(Object* object:children)
	{
		child = static_cast<SpriteBatchNode*>(object);
		child->getTexture()->setAntiAliasTexParameters();
	}
	//获得单个图块的大小，为了在绘制时得到偏移量，否则绘制出来的线条有半个图块的偏移差
	auto m_tamara = tmx_layer_->getTileAt(Point(0,0));
	auto texture = m_tamara->getTexture();
	auto block_size = texture->getContentSize();
	transform_ = Point(block_size.width/4,block_size.height/2);

	//创建“暂停”按钮精灵
	MenuItemImage *zan_ting_item = MenuItemImage::create
	(
		"pic/zanting.png",		//平时的图片
		"pic/zanting.png",		//选中时的图
		CC_CALLBACK_1(GameLayer::ZanTing, this)
	);
	//设置暂停菜单按钮的位置
	zan_ting_item->setPosition(Point(40,140));

	//创建暂停菜单对象
	menu_ = Menu::create(zan_ting_item,NULL);
	//设置菜单的位置
	menu_->setPosition(Point(0,0));
	//将菜单添加到布景中
	this->addChild(menu_,DASHBOARD_LEVEL_CGQ);

	//创建终点精灵
	target_sprite_ = Sprite::create("pic/target.png");
	//设置精灵的位置
	auto end = tmx_layer_->getPositionAt(Point(targetAll[0][1],targetAll[0][0]));
	end_world_ = tmx_layer_->convertToWorldSpaceAR(end+transform_);
	target_sprite_->setPosition(end_world_);
	//将终点精灵对象添加到布景中
	this->addChild(target_sprite_,GAME_LEVEL_CGQ+1);
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
    monsters_ = Array::create();
	monsters_ ->retain();
	//创建存放怪action的数组
    actions_ = Array::create();
	actions_ ->retain();
    //创建存放武器的数组
    weapons_ = Array::create();
	weapons_ ->retain();
    //创建存放武器菜单数组
    menus_ = Array::create();
	menus_ ->retain();
    //创建存放怪Bullet的数组
    bullets_ = Array::create();
	bullets_ ->retain();
    //创建存放金钱的数组
    sell_update_menus_ = Array::create();
	sell_update_menus_ ->retain();

	//添加所有label
	AddLabel();
	//添加防御塔菜单精灵
	AddMenuSprite();
	//初始的总金币数
	money_ = 280;
	//初始化生命值
	ten_ = 18;
	//初始化时间常量
    TIME_MAIN=0.7 ;
	//初始化升级的武器
	update_weapon_ = NULL;
	//升级防御塔的标志位
	is_weapon_update_ = false;
	//初始化游戏结束的标志位为false
	is_game_over_=false;
	//初始化野怪移动的标志位为false
	is_monster_run_ = false;
	//初始化创建怪的标志位
	is_found_monster_ = false;
    //移除防御塔精灵对象的标志位
    is_remove_weap_ = false;
    //初始化游戏中怪的批次数
    pass_ = 0;
    //初始化总分数为0
    score_ = 0;
	//设置定时回调指定方法干活
	auto director = Director::getInstance();
	auto sched = director->getScheduler();
	//定时调用run方法，以秒为单位
	sched->scheduleSelector(SEL_SCHEDULE(&GameLayer::Run), this, 1.0, false);
	//定时调用runBullet方法，以秒为单位
	sched->scheduleSelector(SEL_SCHEDULE(&GameLayer::RunBullet), this, 0.002f, false);
	//定时调用attack方法，以秒为单位
	sched->scheduleSelector(SEL_SCHEDULE(&GameLayer::Attack), this, 0.3f, false);

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
	is_caulate_over_ = false;
	//拿到目标点
	target = targetAll[0];
	//绘制路径
	if(CalculatePath())
	{
		PrintPath();
	}
	//初始化创建多个怪的方法，然后调用ready方法
	FoundMonsters();

	return true;
}

void GameLayer::ZanTing(Object* pSender)
{
	//播放音效
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_button_press.mp3");
	//判断粒子效果的标志位
	if(!is_pause_)
	{
		//暂停背景音乐
		CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
		//获取导演
		Director *director = Director::getInstance();
		//导演执行暂停音乐的工作
		director->pause();
		//创建暂停界面
		DialogLayer* dialog_layer = DialogLayer::create();
		//设置位置
		dialog_layer->setPosition(Point(0,0));
		//添加到布景中
		this->addChild(dialog_layer,6);
		//暂停键的标志位
		is_pause_=true;
	}
}

//计算路径的方法
bool GameLayer::CalculatePath()
{
	//释放内存
	FreeMemory();
	//初始化搜索列表
	InitForCalculate();
	//初始化地图
	InitVisitedArray();
	//用A*算法搜索路径
	bool b=BFSAStar();
	return b;
}

//释放内存
void GameLayer::FreeMemory()
{
	//清空hm中的键值对
	if(hm_ != NULL)
	{
		hm_->clear();
		delete hm_;
		hm_ = NULL;
	}
	//清空广度优先A*队列中的指针
	if(astar_queue != NULL)
	{
		while(!astar_queue->empty())
		{
			astar_queue->pop();
		}
		delete astar_queue;
		astar_queue = NULL;
	}
	//释放访问数组
	if(visited != NULL)
	{
		for(int i=0;i<row_;i++)
		{
			delete []visited[i];
		}
		delete []visited;
		visited = NULL;
	}
	is_caulate_over_ = false;
}

//计算之前初始化计算中用到的容器
void GameLayer::InitForCalculate()
{
	//创建动态二维数组
	visited = new int*[row_];
	for(int i = 0; i<row_; i++)
	{
		visited[i] = new int[col_];
	}
	//A*优先级队列比较器
	astar_queue = new priority_queue<INTPARR,vector<INTPARR>,cmp>();//A*用优先级队列
	//结果路径记录
	hm_ = new map<string,int(*)[2]>();
}

//初始化去过未去过的数组
void GameLayer::InitVisitedArray()
{
	for(int i=0;i<row_;i++)
	{
		for(int j=0;j<col_;j++)
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
	astar_queue->push(start);
	while(flag)
	{
		//如果栈不空
		if(astar_queue->empty())
		{
			return false;
		}
		//从队首取出边
		int(*current_edge)[2] = astar_queue->top();
		astar_queue->pop();
		//取出此边的目的点
		int* temp_target = current_edge[1];
		//判断目的点是否去过，若去过则直接进入下次循环
		if(visited[temp_target[1]][temp_target[0]] != 0)
		{
			continue;
		}
		visited[temp_target[1]][temp_target[0]] = visited[current_edge[0][1]][current_edge[0][0]]+1;
		str1 = StringUtils::format("%d", temp_target[0]);
		str2 = StringUtils::format("%d", temp_target[1]);
		//记录目的点的父节点
		hm_->insert(map<string,int(*)[2]>::value_type(str1+":"+str2,current_edge));
		//判断是否找到目的点
		if(temp_target[0] == target[0] && temp_target[1] == target[1])
		{
			is_caulate_over_= true;
			return is_caulate_over_;
		}
		//将所有可能的边入优先级队列
		int curr_col = temp_target[0];
		int curr_row = temp_target[1];
		int(*sequence)[2] = NULL;
		//根据当前图块的奇数偶数行来确定搜索的方向
		if(curr_row%2 == 0)
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
			if(curr_row+i>=0 && curr_row+i<row_ && curr_col+j>=0 && curr_col+j<col_ &&
					map_data_[curr_row+i][curr_col+j]!=1)
			{
				//创建二维数组
				int(*temp_edge)[2] = new int[2][2];
				//设置为下一个目标点
				temp_edge[0][0] = temp_target[0];
				temp_edge[0][1] = temp_target[1];
				temp_edge[1][0] = curr_col+j;
				temp_edge[1][1] = curr_row+i;
				//将二维数组添加进A*用优先级队列中
				astar_queue->push(temp_edge);
			}
		}
	}
}

//打印结果路径
void GameLayer::PrintPath()
{
	//清除以前的路径
	way_.clear();

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
		iter = hm_->find(key);
		int(*tempA)[2] = iter->second;
		//查到元素
		if(iter != hm_->end())
		{
			//拿到起始点的坐标
			Point start = tmx_layer_->getPositionAt(Point(tempA[0][1],tempA[0][0]));
			//拿到目标点的坐标
			Point end = tmx_layer_->getPositionAt(Point(tempA[1][1],tempA[1][0]));
			//将起始点转换到世界坐标系中
			Point start_world = tmx_layer_->convertToWorldSpaceAR(start + transform_);
			//将目标点转换到实际坐标系中
			Point end_world = tmx_layer_->convertToWorldSpaceAR(end + transform_);
			//将目标点添加到路径中
			way_.push_back(end_world);
			CCLOG("end_world.x=%f,end_world.y=%f",end_world.x,end_world.y);
			//排队密度
			glLineWidth( 3.0f );
			//绘制路径
			cocos2d::ccDrawColor4F(0.0f, 0.0f, 0.0f, 1.0f);
			cocos2d::ccDrawLine(start_world, end_world);
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
void GameLayer::Ready()
{
	//创建起点精灵
	start_sprite_ = Sprite::create("pic/start.png");
	//设置精灵的位置
	auto start = tmx_layer_->getPositionAt(Point(source[1],source[0]));
	start_world_ = tmx_layer_->convertToWorldSpaceAR( start + transform_);
	start_sprite_->setPosition(start_world_);
	//将精灵对象添加到布景中
	this->addChild(start_sprite_,5);

	//在起点精灵上添加百分比动作特效
	ProgressTo *to1 = ProgressTo::create(6, 100);
	left_ = ProgressTimer::create(Sprite::create("pic/ring2.png"));
	left_->setType( ProgressTimer::Type::RADIAL);
	left_->setRotation(-90);
	this->addChild(left_,5);
	left_->setPosition(start_world_);
	left_->runAction(
			Sequence::create(to1,
			CallFuncN::create(CC_CALLBACK_0(GameLayer::PlayGameCallback, this)),
			NULL
			)
	);

}

//出野怪前准备方法的回调方法
void GameLayer::PlayGameCallback()
{
	//移除精灵
	this->removeChild(left_);
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_swish.mp3");
	//怪移动的标志位设置为true
	is_monster_run_=true;
}

//出野怪时起始点要播放的两个特效
void GameLayer::AddParticle(Point point,float time)
{
	if(DialogLayer::isParticle)
	{
		int count_var=rand()%5;
		for(int i=0;i<35+count_var;i++)
		{
			float angle_var=rand()%360;
			int length_var=rand()%10;
			int count_var=rand()%10;
			int sprite_length=rand()%5;

			particle_=Sprite::create("pic/white1.png");
			particle_->setAnchorPoint(Point(0,0.5));
			particle_->setRotation(-angle_var);
			this->addChild(particle_,5);

			Point vocter= Vec2::forAngle(angle_var*3.1415926/180);
			Point monPointOne = ccpMult(vocter,(float)(length_var+30))+point;
			particle_->setPosition(monPointOne);
			particle_->setScale(0.7);
			particle_->runAction(
						Sequence::create(
								Spawn::createWithTwoActions
								(
									MoveTo::create(time*3/7.0,point),
									FadeIn::create(time/5.0)
								),
								CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),//TODO:内存泄漏
								NULL
								)
						);
		}

		cc_ = Sprite::create("pic/fire1.png");
		ActionInterval *act=FadeIn::create(time*4/5);
		ActionInterval *activeFade=FadeOut::create(time*4/5);
		cc_->setPosition(point);
		this->addChild(cc_,6);
		cc_->setScale(2.0);
		cc_->runAction(Sequence::create(
				Spawn::createWithTwoActions
							(
								act,
								activeFade
							),
							CallFuncN::create(CC_CALLBACK_0(GameLayer::RemoveSpriteAdd,this)),//TODO:泄漏
							NULL
				)
		);
	}else{
		RemoveSpriteAdd();
	}
}

//将地图格子行列号转换为对应格子的贴图坐标
Point GameLayer::FromColRowToXY(int col, int row)//入口参数//横着看的x，y
{
	row++;
	Point start = tmx_layer_->getPositionAt(Point(col,row));//横着看的x，y
	Point start_world = tmx_layer_->convertToWorldSpaceAR(start + transform_);
	return start_world;
}
//将触控点位置转换为地图格子行列号
Point GameLayer::FromXYToColRow(int xPos, int yPos)
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
	if(is_touch_move_)
	{
		return false;
	}
	//拿到当前触控点的坐标
	Point point = pTouch->getLocation();
	//如果防御塔不升级
	if(!is_weapon_update_)
	{
		//遍历存放防御塔菜单精灵的数组
		for(int k = 0; k<menus_->count(); k++)
		{
			//拿到防御塔菜单精灵
			Weapon* pWeapon =(Weapon*)menus_->getObjectAtIndex(k);
			//获取防御塔的坐标
			Point pp = pWeapon->getPosition();
			//如果是点击菜单防御塔
			if(abs(point.x- pp.x)<32&&abs(point.y- pp.y)<32)
			{
				//得到点击到的菜单防御塔的id
				int id = pWeapon->id_;
				//得到安装该防御塔所需的金币
				int tempValue=pWeapon->value_;
				//拿到触控点的地图的行列号
				Point ccpxy = FromXYToColRow((int)point.x,(int)point.y);
				//将行列号转换为对应的地图贴片的坐标
				Point ccp = FromColRowToXY(ccpxy.x,ccpxy.y);
				//如果钱不够
				if(money_<tempValue)
				{
					return false;
				}
				//根据得到的菜单防御塔的id创建一个防御塔
				Weapon *oneTa =Weapon::Create(id);
				//设置防御塔的位置
				oneTa->setPosition(ccp);
				//将防御塔添加到布景中
				this->addChild(oneTa,6);
				//设置防御塔
				s_dic_.setObject(oneTa, pTouch->getID());
				//将防御塔移动的标志位设置为true
				is_touch_move_=true;

				return true;
			}
		}
	}else{
		//循环数组看选择的是升级还是出售
		for(int k=0;k<sell_update_menus_->count();k++)
		{
			std::string overStr = "$";
			//定义一个临时变量，用来记录防御塔更新时所需的金币数
			int tempValue=update_weapon_->update_value_;
			//如果是升级
			if(abs(point.x-730)<32&&abs(point.y-408)<32)
			{
				//如果钱不够升级
				if(money_<tempValue)
				{
					return false;
				}
				//总的金币数减去升级防御塔所需的金币数
				money_ -= tempValue;
				//把int 型的数据转换成string型的 然后set
				char a[6];
				snprintf(a, 6, "%d",money_);
				//更新当前总金币数
				money_label_->setString((overStr+a).c_str());
				//调用update方法
				update_weapon_->Update();

				return true;
			}
			//如果是出售防御塔
			if(abs(point.x-730)<32&&abs(point.y-68)<32)
			{
				//总的金币数加上要出售防御塔所得的金币数
				money_ +=update_weapon_->sell_value_;
				char a[6];//把int 型的分数转换成string型的 然后set
				snprintf(a, 6, "%d",money_);
				//更新总的金币数
				money_label_->setString((overStr+a).c_str());
				//调用sellWeapon方法，出售防御塔
				SellWeapon(update_weapon_);

				return true;
			}
		}
	}
	//遍历存放子防御塔的数组
	int k = 0;
	for(; k<weapons_->count(); k++)
	{
		//拿到指向子防御塔的指针
		Weapon* pWeapon =(Weapon*)weapons_->getObjectAtIndex(k);
		//获取其位置
		Point ccWeapon = pWeapon->getPosition();
		//如果是点击了子防御塔
		if(abs(point.x-ccWeapon.x)<32&&abs(point.y-ccWeapon.y)<32)
		{
			Point ccpxy = FromXYToColRow((int)ccWeapon.x,(int)ccWeapon.y);
			//如果已经选中了一个防御塔
			if(is_weapon_update_)
			{
				//设置为不可见
				(update_weapon_->getChildByTag(1))->setVisible(false);
			}
			//把选中的防御塔给要升级的
			update_weapon_=pWeapon;
			//使得防御塔升级按钮可见
			SetUpdateTrue();
			//移动标志位设为false
			is_touch_move_=false;

			return true;
		}
	}
	if(k==weapons_->count())
	{
		//设置防御塔菜单精灵可见
		SetWeaponTrue();
		//触控移动的标志
		is_touch_move_=false;
	}
	return false;
}

//触控移动防御塔的方法
void GameLayer::onTouchMoved(Touch *pTouch, Event *pEvent)
{
	//防御塔的物理位置，看是否在路径当中，若在则重新计算路径
	if(!is_touch_move_)
	{
		return ;
	}
	//得到当前触控位置的坐标
	Point point = pTouch->getLocation();
	//根据菜单防御塔的id创建一个表示防御塔攻击范围的精灵对象
	Weapon* trSprite = (Weapon*)s_dic_.objectForKey(pTouch->getID());
	//设置表示防御塔攻击范围的精灵可见
	(trSprite->getChildByTag(1))->setVisible(true);
	//拿到触控点对应的地图的行列号
	Point ccpxy = FromXYToColRow((int)point.x,(int)point.y);
	//将该行列号转换为对应贴图的坐标
	Point ccp = FromColRowToXY(ccpxy.x,ccpxy.y);
	//设置精灵对象的位置
	trSprite->setPosition(ccp);
}

//触控抬起
void GameLayer::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	//如果点击到菜单防御塔的标志位为false则返回
	if(!is_touch_move_)
	{
		return;
	}
	//获取触控点位置的坐标
	Point point = pTouch->getLocation();
	//创建一个表示攻击范围的精灵对象
	Weapon* trSprite = (Weapon*)s_dic_.objectForKey(pTouch->getID());
	//设置表示攻击范围的精灵不可见
	(trSprite->getChildByTag(1))->setVisible(false);
	//得到触控点对应的地图的行列号
	Point ccpxy = FromXYToColRow((int)point.x,(int)point.y);
	//将该地图的行列号转换成对应贴图的坐标
	Point ccp = FromColRowToXY(ccpxy.x,ccpxy.y);
	//如果防御塔放置的图块是地图的墙
	if(map_data_[(int)ccpxy.x][(int)(ccpxy.y+1)]!=0)
	{
		//移除防御塔标志位设定为true
		is_remove_weap_ = true;
	}else//如果放置的位置为可以放置的位置
	{
		int i=0;
		//循环路径数组，看看是否为路径中的
		for(;i<way_.size();i++)
		{
			//拿到路径中的每个点
			Point ccpWay = (Point)way_.at(i);
			//如果当前放防御塔的格子为路径中的
			if(ccp.x==ccpWay.x&&ccp.y==ccpWay.y)
			{
				//重新计算路径，把当前格子设为可以放防御塔怪不可以走
				map_data_[(int)ccpxy.x][(int)(ccpxy.y+1)]=1;
				//进行路径搜索
				bool isCaulate=CalculatePath();
				//如果重新找到路径
				if(isCaulate)
				{
					//把还没到此格子的怪里的way换了,创建新的way
					PrintPath();
					//遍历存放怪的数组
					for(int j=0;j<monsters_->count();j++)
					{
						//拿到当前的怪
						Monsters* mon = (Monsters*)monsters_->objectAtIndex(j);
						//第几个路径
						int wayat = way_.size()-i;
						//如果怪没有经过该放防御塔的格子(-2为视觉和时间上的关系不是具体数据)
						if(mon->way_< wayat-2)
						{	//把新的路径给怪
							mon->self_way_=way_;
						}
					}
					//将移除防御塔的标志位设置为false
					is_remove_weap_=false;
					//把防御塔当前处的格子行列给特效精灵对象
					trSprite->point_col_row_ =ccpxy;
					break;
				}else
				{
					//如果没找到路径,把原来格子设置为怪可以走
					map_data_[(int)ccpxy.x][(int)(ccpxy.y+1)]=0;
					//移除防御塔标志位设定为true
					is_remove_weap_ = true;

					break;
				}
			}
		}
		//循环路径数组完成，当前格子不在路径的数组里
		if(i==way_.size())
		{
			//不是路径中的，把该格子设为野怪不可走
			map_data_[(int)ccpxy.x][(int)(ccpxy.y+1)]=1;
			//移除防御塔的精灵的标志位设置为false
			is_remove_weap_=false;
			//把防御塔当前处的格子行列给防御塔
			trSprite->point_col_row_=ccpxy;
		}
	}
	//如果移除防御塔精灵对象
	if(is_remove_weap_)
	{
		this->removeChild(trSprite);
	}
	else
	{	//成功购买防御塔
		int tempValue=trSprite->value_;
		money_-=tempValue;
		std::string overStr = "$";
		char a[6];//把int 型的分数转换成string型的 然后set
		snprintf(a, 6, "%d",money_);
		money_label_->setString((overStr+a).c_str());
		//将防御塔精灵对象添加到存放子塔的数组中
		weapons_->addObject(trSprite);
	}
	//为了让began走将移动的标志位设置为false
	is_touch_move_=false;
}

//设置升级菜单精灵可见
void GameLayer::SetUpdateTrue()
{
	//遍历存放菜单防御塔的数组
	for(int i=0;i<menus_->count();i++)
	{
		//拿到指向菜单防御塔的指针
		Weapon* weapon = (Weapon*)menus_->objectAtIndex(i);
		//设置菜单防御塔不可见
		weapon->setVisible(false);
	}
	//遍历存放金币的数组
	for(int j=0;j<sell_update_menus_->count();j++)
	{
		//拿到指向升级菜单的指针
		Sprite* update = (Sprite*)sell_update_menus_->objectAtIndex(j);
		//将升级菜单设置为可见
		update->setVisible(true);
	}
	//将显示升级所需金币数的文本标签设置为可见
	update_money_label_->setVisible(true);
	//将显示卖掉防御塔时金币收入的文本标签设置为可见
	sell_money_label_->setVisible(true);
	//防御塔升级的标志位设置为true
	is_weapon_update_ = true;
	if(update_weapon_!=NULL)
	{
		//将范围设置为可见
		(update_weapon_->getChildByTag(1))->setVisible(true);
		//如果要升级的防御塔的等级为4，即最高级
		if(update_weapon_->level_==4)
		{	//升级到头表示升级的金币不可见
			update_money_label_->setVisible(false);
		}
		//如果剩的钱不够升级
		if(money_<update_weapon_->update_value_||update_weapon_->level_==4||update_weapon_->is_update_mark_==true)
		{	//升级到头||正在升级
			Sprite* update = (Sprite*)sell_update_menus_->objectAtIndex(1);
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(100));
			update->runAction(act);
			ActionInterval *act1=FadeTo::create(0.1f,(GLubyte)(100));
			update_money_label_->runAction(act1);
		}else
		{
			Sprite* update = (Sprite*)sell_update_menus_->objectAtIndex(1);
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(255));
			update->runAction(act);
			ActionInterval *act1=FadeTo::create(0.1f,(GLubyte)(255));
			update_money_label_->runAction(act1);
		}
		//设置升级的钱
		SetValue();
	}
}

//设置防御塔菜单精灵可见
void GameLayer::SetWeaponTrue()
{
	//遍历存放菜单防御塔的数组
	for(int i=0;i<menus_->count();i++)
	{
		//拿到存放在菜单防御塔数组中的防御塔对象
		Weapon* weapon = (Weapon*)menus_->objectAtIndex(i);
		//将其设置为可见
		weapon->setVisible(true);
	}
	//遍历存放金币的数组
	for(int j=0;j<sell_update_menus_->count();j++)
	{
		Sprite* update = (Sprite*)sell_update_menus_->objectAtIndex(j);
		update->setVisible(false);
	}
	update_money_label_->setVisible(false);
	sell_money_label_->setVisible(false);
	//防御塔更新的标志位设置为false
	is_weapon_update_ = false;
	//如果可更新的防御塔为空
	if(update_weapon_!=NULL)
	{
		//将防御塔周围的光圈精灵设为不可见
		(update_weapon_->getChildByTag(1))->setVisible(false);
	}
}

//设置升级的金币
void GameLayer::SetValue()
{
	std::string overStr = "$";
	char a[6];//把int 型的分数转换成string型的 然后set
	snprintf(a, 6, "%d",update_weapon_->sell_value_);
	sell_money_label_->setString((overStr+a).c_str());
	char b[6];//把int 型的分数转换成string型的 然后set
	snprintf(b, 6, "%d",update_weapon_->update_value_);
	update_money_label_->setString((overStr+b).c_str());
}

//出售武器的方法
void GameLayer::SellWeapon(Weapon* weapon)
{
	//获取武器所在格子的行列
	Point temp_xy = weapon->getPosition();
	//从存放防御塔的数组中移除
	weapons_->removeObject(weapon);
	//从layer上移除
	this->removeChild(weapon);
	//拿到该地图格子的行列号
	Point ccpxy =FromXYToColRow((int)temp_xy.x,(int)temp_xy.y);
	//把地图格子设为可以走
	map_data_[(int)ccpxy .x][(int)(ccpxy.y)]=0;
	//将更新防御塔设置为空
	update_weapon_=NULL;
	//游戏玩家的金钱加
	SetWeaponTrue();
}

//野怪到终点时的特效
void GameLayer::AddParticle(Point point,int id,float time)
{
	if(!DialogLayer::isParticle)
	{
		return;
	}
	std::string pic_table[6]={"pic/red.png","pic/yellow.png","pic/blue.png","pic/white.png","pic/red.png","pic/blue.png"};
	std::string pic_particle[6]={"pic/hong.png","pic/orige.png","pic/zi.png","pic/liang.png","pic/hong.png","pic/zi.png"};
	int countVar=rand()%10;
	int angle=0;
	for(int i=0;i<55+countVar;i++)
	{
		float time_var=(rand()%10)/10.0;
		float angle_var=(rand()%10)/10.0;
		Sprite* particle= Sprite::create(pic_table[id-1].c_str());
		particle->setAnchorPoint(Point(0,0.5));
		particle->setPosition(point);
		particle->setScale(1.0);
		particle->setRotation(-(angle+angle_var));
		Point vocter=ccpForAngle((angle+angle_var)*3.1415926/180);
		int lange=rand()%200;
		angle+=11;
		this->addChild(particle,5);
		particle->runAction(
			Sequence::create(
					Spawn::createWithTwoActions
					(
						MoveBy::create(1+time_var,Point((200+lange)*vocter.x,(200+lange)*vocter.y)),
						FadeOut::create(1+time_var)
					),
					CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),//TODO:泄漏
					NULL
			)
		);
	}

	Sprite* ccp = Sprite::create(pic_particle[id-1].c_str());
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
		CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),
		NULL
		)
	);

	cc_ = Sprite::create("pic/fire1.png");
	ActionInterval *act1=FadeIn::create(time*4/9);
	ActionInterval *activeFade1=FadeOut::create(time*4/9);
	cc_->setPosition(point);
	this->addChild(cc_,6);
	cc_->setScale(3.0);
	cc_->runAction(Sequence::create(
			Spawn::createWithTwoActions
						(
							act1,
							activeFade1
						),
						CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),
						NULL
			)
	);
}

//野怪死时的特效
void GameLayer::AddParticle1(Point point,int id,float time)
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
					CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),
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
		CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),
		NULL
		)
	);
}

//第二个防御塔攻击怪时调用的特效
void GameLayer::AddParticle(Point point,int id,float time,float angle)
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
					CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),
					NULL
					)
				);
	}
}

//怪从action数组里出来挨个走
void GameLayer::Run()
{
	//如果游戏结束则返回
	if(is_game_over_)
	{
		return;
	}
	//遍历防御塔菜单精灵设置菜单的透明度
	for(int k=0;k<menus_->count();k++)
	{
		//指向武器对象的指针
		Weapon* pWeapon =(Weapon*)menus_->objectAtIndex(k);
		//如果当前金币数小于安装武器时需要的金币
		if(money_<pWeapon->value_)
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
	if(update_weapon_!=NULL)
	{
		SetValue();
		//如果要升级的武器的等级已经为4，即最高级
		if(update_weapon_->level_==4)
		{	//升级到头表示升级的金币不可见
			update_money_label_->setVisible(false);
		}
		//如果当前总金币数小于升级所需金币数或者已经升级到最高级或者升级的标志位为true
		if(money_<update_weapon_->update_value_||update_weapon_->level_==4||update_weapon_->is_update_mark_==true)
		{
			//声明一个指向存放金钱数组的指针
			Sprite* update = (Sprite*)sell_update_menus_->objectAtIndex(1);
			//将表示升级的箭头按钮设置为不可触控
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(100));
			update->runAction(act);
			//将表示升级的$文本标签设置为不可触控
			ActionInterval *act1=FadeTo::create(0.1f,(GLubyte)(100));
			update_money_label_->runAction(act1);
		}else
		{
			//否则将其设置为可触控可升级
			Sprite* update = (Sprite*)sell_update_menus_->objectAtIndex(1);
			ActionInterval *act=FadeTo::create(0.1f,(GLubyte)(255));
			update->runAction(act);
			ActionInterval *act1=FadeTo::create(0.1f,(GLubyte)(255));
			update_money_label_->runAction(act1);
		}
	}

	//如果创建怪的标志位为true则走下面代码
	if(is_found_monster_)
	{
		//如果存放怪与存放怪action的数组都为0则走下面代码
		if(monsters_->count()==0&&actions_->count()==0)
		{
			//回合数自加
			pass_++;
			char a[6];//把int 型的分数转换成string型的 然后set
			snprintf(a, 6, "%d",pass_+1);
			pass_label_->setString(a);
			//不让怪运动
			is_monster_run_=false;
			//currMon++;
			//调用创建多个怪的方法
			FoundMonsters();
		}
	}
	//如果存放怪action的数组为空则走下面代码
	if(actions_->count()==0)
	{
		return;
	}

	//拿到起始点的坐标，横着看的x，y
	Point start = tmx_layer_->positionAt(Point(source[1],source[0]));
	//将起点坐标转换到世界坐标系中
	Point start_world = tmx_layer_->convertToWorldSpaceAR(start+transform_);
	//如果怪移动的标志位为true
	if(is_monster_run_)
	{
		//在起始点处添加两个特效，参数为坐标和时间间隔
		this->AddParticle(start_world,1.0);
	}
}

//怪移动到终点
void GameLayer::MonsterRun(Node* node)
{
	Monsters* monster=(Monsters*) node ;
	//将怪设为可见
	monster->setVisible(true);
	//如果野怪已经走的步数等于原定路径的长度
	if(monster->way_==monster->self_way_.size())
	{
		//如果生命值大于0
		if(ten_>0)
		{
			//生命值自减
			ten_--;
			//播放音效
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
		}
		//添加野怪到终点时的特效
		this->AddParticle(monster->getPosition(),monster->id_,3.0);
		char a[6];//把int 型的分数转换成string型的 然后set
		snprintf(a, 6, "%d",ten_);
		//设置表示生命值的文本标签
		ten_label_->setString(a);
		//野怪到终点后移除精灵
		this->removeChild(monster);
		//移除数组中的怪
		monsters_->removeObject(monster);
		if(ten_<=0)//失败
		{
			is_game_over_=true;
			this->LoseGame();
		}
	}

	//如果怪已经走的路径长度小于要走路径的总长度
	if(monster->way_<monster->self_way_.size())
	{
		//野怪走的步数自加
		monster->way_++;
		//获取野怪当前的位置点
		Point crrPosition=monster->getPosition();
		//拿到下一个要走的点的坐标
		Point tarPosition=monster->self_way_.at(monster->self_way_.size() - monster->way_);
		//拿到怪当前所处地图格子的行列号的x值
		int crrpoint_x=(int)(FromXYToColRow((int)crrPosition.x,(int)crrPosition.y).x);
		//拿到怪当前所处地图格子的行列号的y值
		int crrpoint_y=(int)(FromXYToColRow((int)crrPosition.x,(int)crrPosition.y).y);
		//拿到下一个要走的点的x值
		int tarpoint_x=(int)(FromXYToColRow((int)tarPosition.x,(int)tarPosition.y).x);
		//拿到下一个要走的点的y值
		int tarpoint_y=(int)(FromXYToColRow((int)tarPosition.x,(int)tarPosition.y).y);
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
		if(monster->id_==2)
		{
			Point vacter;
			vacter.x=tarPosition.x-crrPosition.x;
			vacter.y=tarPosition.y-crrPosition.y;
			float dirction=ccpToAngle(vacter);
			monster->setRotation(-(dirction*180/3.1415926));
			monster->Refresh(dirction);
		}
		//给定当前点与目标点的中点以及一个时间的MoveTo
		ActionInterval* act1=MoveTo::create(time1,zhong);
		//给定目标点及一个时间的MoveTo
		ActionInterval* act2=MoveTo::create(time2,tarPosition);
		Sequence* seqer = Sequence::create
								(
									act1,
									act2,
									CallFuncN::create(CC_CALLBACK_1(GameLayer::MonsterRun,this)),
									NULL
								);
		monster->runAction(seqer);
	}
}

void GameLayer::RemoveSpriteAdd()
{
	//声明一个指向怪动作数组中最后一个怪的指针
	Monsters* mon = (Monsters*)actions_->lastObject();
	//把怪添加到怪数组里
	monsters_->addObject(mon);
	//调用怪移动到终点的方法
	MonsterRun(mon);
	//移动怪动作数组中的怪
	actions_->removeLastObject();
	if(!DialogLayer::isParticle)
	{
		//删除特效精灵对象
		this->removeChild(cc_,true);
	}
}

//创建多个怪的方法
void GameLayer::FoundMonsters()
{
	//随机的出怪
	int id = pass_%5;

	for(int i=0;i<10;i++)
	{
		//创建一个怪对象
		Monsters* mon = Monsters::Create(id+1,way_);
		//拿到怪起始位置的坐标,横着看的x,y
		Point start = tmx_layer_->positionAt(Point(source[1],source[0]));
		//将起始点的坐标转换到世界坐标系中
		Point startWorld = tmx_layer_->convertToWorldSpaceAR(start+transform_);
		//设置怪的初始位置
		mon->setPosition(startWorld);
		//初始时将怪设为不可见
		mon->setVisible(false);
		//将怪添加到布景中
		this->addChild(mon,6);
		//把怪添加到action数组里
		actions_->addObject(mon);
		if(id>2)//两个单独
		{
			break;
		}
	}
	//将创建怪的标志位设置为true
	is_found_monster_=true;
	//调用ready方法
	Ready();
}

//攻击怪的方法
void GameLayer::Attack()
{
	//如果怪不移动或者游戏结束
	if(!is_monster_run_||is_game_over_)
	{
		return;
	}
	//防御塔对怪的扫描
	for(int i=0;i<weapons_->count();i++)
	{
		if(((Weapon*)weapons_->objectAtIndex(i))->is_can_fire_)
		{
			for(int j=0;j<monsters_->count();j++)
			{
				//拿到指向怪对象的指针
				Monsters* monster = (Monsters*)monsters_->objectAtIndex(j);
				//拿到指向防御塔的指针
				Weapon* weapon = (Weapon*)weapons_->objectAtIndex(i);
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
				if(lengthVector<=weapon->confines_)
				{
					//防御塔旋转
					weapon->setRotation(-(angle*180/3.1415926));
					//改变防御塔角度属性
					weapon->angle_=angle*180/3.1415926;
					//第一个防御塔攻击怪时调用减血的方法
					if(weapon->id_==1)
					{
						CocosDenshion::SimpleAudioEngine::getInstance()->playEffect
						(
							"sound/sf_minigun_hit.mp3"
						);
						FireBulletOne(i,j,angle,bulletPoint,lengthVector);
						break;
					}
					//第二个防御塔攻击怪时调用减血的方法
					else if(weapon->id_==2)
					{
						CocosDenshion::SimpleAudioEngine::getInstance()->playEffect
						(
							"sound/sf_laser_beam.mp3"
						);
						FireBulletTwo(i,j,angle,bulletPoint);
						weapon->Fireing();
						break;
					}
					//第三个防御塔攻击怪时调用减血的方法
					else if (weapon->id_==3)
					{
						CocosDenshion::SimpleAudioEngine::getInstance()->playEffect
						(
							"sound/sf_rocket_launch.mp3"
						);
						FireBulletThree(i,j,angle,bulletPoint);
						weapon->Fireing();
						break;
					}

					//第四个防御塔的攻击
					else if(weapon->id_==4)
					{
						//初始化防御塔的旋转角度
						weapon->setRotation(0);
						//初始化防御塔的角度
						weapon->angle_=0;
						//创建一个子弹对象
						Sprite* bullet = Sprite::create("pic/ring_blue.png");
						float bulletX=bullet->getContentSize().width/2;
						//扩大的倍数
						float scaleX=weapon->confines_/bulletX;
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
								CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),//TODO:泄漏
								NULL
								)

						);
						weapon->Fireing();
						//怪掉血
						monster->CutBlood(weapon->hurt_);
						//如果怪的血小于0
						if(monster->blood_<=0)
						{
							//拿到怪的位置
							Point pointMonster=monster->getPosition();
							//拿到怪的路径
							vector <Point > tempSelfWay = monster->self_way_;
							//拿到怪已经走的路径
							int tempWay = monster->way_;
							//如果怪的id为3
							if(monster->id_==3)
							{
								//设置锚点
								Point Achorpoint=(Point(0.5,0.4));
								for(int i=0;i<2;i++)
								{
									//创建两个新的怪
									Monsters* mon = Monsters::Create(6,tempSelfWay);
									//新怪拿到老怪已经走的路径
									mon->way_=tempWay;
									//设置怪的位置
									mon->setPosition(pointMonster);
									//设置怪的锚点
									mon->setAnchorPoint(Achorpoint);
									Achorpoint=Point(Achorpoint.x+0.2,Achorpoint.y+0.2);
									//把怪添加到怪数组里
									monsters_->addObject(mon);
									//将怪添加到布景中
									this->addChild(mon,6);
									//调用怪移动到终点的方法
									MonsterRun(mon);
								}
							}
							//定义一个临时存放怪死后金币的变量
							int tempMoney = monster->id_*10;
							//总的金币数加上临时所得的等于当前总的金币
							money_+=tempMoney;
							//定义一个临时存放怪死后所得分数的变量
							int tempScore = monster->id_*15;
							//总的分数加上临时所得的等于当前总的分数
							score_+=tempScore;
							//怪死时添加一个特效
							this->AddParticle1(monster->getPosition(),monster->id_,1);
							//播放音效
							CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
							//删除场景中的怪
							this->removeChild(monster);
							//删除怪数组中的怪
							monsters_->removeObject(monster);
							std::string overStr = "$";
							char a[6];//把int 型的分数转换成string型的 然后set
							snprintf(a, 6, "%d",money_);
							//更新显示金币的文本标签
							money_label_->setString((overStr+a).c_str());
							char b[6];//把int 型的分数转换成string型的 然后set
							snprintf(b, 6, "%d",score_);
							//更新显示总分数的文本标签
							score_label_->setString(b);
						}
					}
				}
			}
		}
	}
}

//第一个防御塔攻击
void GameLayer::FireBulletOne(int weap,int target,float dirction,Point position,float length_vector)
{
	//判断存放
	if(this->bullet_data_[target]==0)
	{
		this->bullet_data_[target]=1;
	}
	//获取要攻击的怪对象
	Monsters* monster = (Monsters*)monsters_->objectAtIndex(target);
	//获取防御塔的信息
	Weapon* weapon = (Weapon*)weapons_->objectAtIndex(weap);
	//发射子弹的数量与防御塔的等级有关
	int count[4]={1,2,3,3};
	//攻击子弹的延迟
	float delay[3]={0.01,0.1,0.05};

	//根据防御塔的等级来确定要发射子弹的数量和位置
	Point* positionByLevel=new Point[count[weapon->level_-1]];

	//如果防御塔的等级为1，确定每发子弹的初始位置
	if(weapon->level_==1)
	{
		positionByLevel[0]=position;
	}
	//如果防御塔的等级为2，发射两发子弹
	else if(weapon->level_==2)
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
	else if(weapon->level_==3||weapon->level_==4)
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
	for(int i=0;i<count[weapon->level_-1];i++)
	{
		//创建一个子弹对象
		BulletSprite* bullet = BulletSprite::Create("pic/bullet.png",weapon->hurt_,target);
		//设置子弹的位置
		bullet->setPosition(positionByLevel[i].x,positionByLevel[i].y);
		//将子弹添加到布景中
		this->addChild(bullet,4);
		//定义一个时间变量
		float timeTo=length_vector/weapon->confines_;
		//声明一个动作
		ActionInterval* act=MoveTo::create(timeTo/5,monster->getPosition());
		//顺序执行
		Sequence* seqer = Sequence::create
						(
							DelayTime::create(delay[i]),
							act,
							CallFuncN::create(CC_CALLBACK_1(GameLayer::CutBloodOne,this)),
								NULL
						);
		bullet->runAction(seqer);
		weapon->Fireing();
	}
}

//怪掉血的方法
void GameLayer::CutBloodOne(Node*node)
{
	//拿到子弹攻击的目标怪
	Monsters* monster=(Monsters*)monsters_->objectAtIndex(((BulletSprite*)node)->target_);
	//调用cutBlood方法让怪减血
	monster->CutBlood(((BulletSprite*)node)->hurt_);
	//判断怪的血量是否小于等于0
	if(monster->blood_<=0)
	{
		//定义一个临时变量存放怪的路径
		vector <Point > tempSelfWay = monster->self_way_;
		//定义一个临时变量存放父怪已经走的路径
		int tempWay = monster->way_;
		//获取怪的位置
		Point pointMonster=monster->getPosition();
		//第三种怪死后会创建两个新怪
		if(monster->id_==3)
		{
			Point Achorpoint=(Point(0.5,0.4));
			//创建两个新的怪对象
			for(int i=0;i<2;i++)
			{
				//创建怪对象
				Monsters* mon = Monsters::Create(6,tempSelfWay);
				//拿到父怪已经走的路径
				mon->way_=tempWay;
				//设置新怪的位置
				mon->setPosition(pointMonster);
				//设置锚点
				Achorpoint=Point(Achorpoint.x+0.2,Achorpoint.y+0.2);
				//设置新怪的锚点
				mon->setAnchorPoint(Achorpoint);
				//把怪添加到怪数组里
				monsters_->addObject(mon);
				//将新建的两个怪添加到布景中
				this->addChild(mon,6);
				//调用怪移动到终点的方法
				MonsterRun(mon);
			}
		}
		//定义一个临时变量存放怪死时得到的金币
		int tempMoney = monster->id_*10;
		//总的金币数加上怪死所得的金币数等于当前的总金币数
		money_+=tempMoney;
		//定义一个临时变量存放怪死时得到的分数
		int tempScore = monster->id_*15;
		//总的分数加上怪死时得到的分数等于当前总的分数
		score_+=tempScore;
		//添加怪死时的特效
		this->AddParticle1(monster->getPosition(),monster->id_,1.0);
		//添加音效
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
		//删除怪对象
		this->removeChild(monster);
		//删除数组中的怪
		monsters_->removeObject(monster);
		char a[6];//把int 型的分数转换成string型的 然后set
		snprintf(a, 6, "%d",money_);
		std::string overStr = "$";
		//更新显示当前总金币数的文本标签
		money_label_->setString((overStr+a).c_str());
		char b[6];//把int 型的分数转换成string型的 然后set
		snprintf(b, 6, "%d",score_);
		//更新显示当前总分数的文本标签
		score_label_->setString(b);
	}
	//删除子弹对象
	this->removeChild(node);
}

//第二个防御塔的攻击
void GameLayer::FireBulletTwo(int weap,int target,float dirction,Point position)
{
	//拿到目标野怪
	Monsters* monster = (Monsters*)monsters_->objectAtIndex(target);
	//拿到防御塔
	Weapon* weapon = (Weapon*)weapons_->objectAtIndex(weap);
	//获取野怪当前的位置
	Point pointMonster=monster->getPosition();
	//获取防御塔当前的位置
	Point pointWeapon=weapon->getPosition();
	//存放子弹精灵图片的数组
	std::string bullet[4]={"pic/weapon2-1.png","pic/weapon2-2.png","pic/weapon2-3.png","pic/weapon2-4.png"};
	//创建一个子弹精灵对象
	bullet1_ = Sprite::create(bullet[weapon->level_-1].c_str());
	//设置子弹的锚点
	bullet1_ ->setAnchorPoint(Point(0,0.5));
	//设置子弹的位置
	bullet1_->setPosition(position);
	//将子弹添加到布景中
	this->addChild(bullet1_,4);
	//发射子弹的动作特效
	bullet1_->setRotation(-(dirction*180/3.1415926));
	bullet1_ -> runAction(
				Sequence::create(
						DelayTime::create(0.1),
						CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),//TODO:泄漏
						NULL
						)
				);
	//遍历存放怪的数组，计算所有被激光碰到的怪
	for(int k=0;k<monsters_->count();k++)
	{
		//拿到数组中的怪
		Monsters * mon= (Monsters*)monsters_->objectAtIndex(k);
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
				mon->CutBlood(weapon->hurt_);
				//添加一个怪被击中时的特效
				this->AddParticle(mon->getPosition(),monster->id_,0.5,dirction*180/3.1415926);
			}
		}
		//如果怪没血，则删除
		if(mon->blood_<=0)
		{	//获取野怪的位置
			Point pointMonster=monster->getPosition();
			//获取怪当前走的路径
			vector <Point > tempSelfWay = mon->self_way_;
			//获取怪已经走的路径
			int tempWay = mon->way_;
			//如果怪的id为3
			if(monster->id_==3)
			{
				//设置怪的锚点
				Point Achorpoint=(Point(0.5,0.4));
				for(int i=0;i<2;i++)
				{
					//创建2个新的怪
					Monsters* mon = Monsters::Create(6,tempSelfWay);
					//将已经走的路径传给新怪
					mon->way_=tempWay;
					//设置新怪的位置
					mon->setPosition(pointMonster);
					//设置新怪的锚点
					mon->setAnchorPoint(Achorpoint);
					Achorpoint=Point(Achorpoint.x+0.2,Achorpoint.y+0.2);
					//把怪添加到怪数组里
					monsters_->addObject(mon);
					//将怪添加到布景中
					this->addChild(mon,6);
					//调用怪移动到终点的方法
					MonsterRun(mon);
				}
			}
			//怪死后后得到对应的金币
			int tempMoney = mon->id_*10;
			//总的金币数要加上怪死后得到的金币数
			money_+=tempMoney;
			//怪死后会得到对应的分数
			int tempScore = monster->id_*15;
			//总的分数要加上怪死后的分数
			score_+=tempScore;
			//添加特效
			this->AddParticle1(mon->getPosition(),monster->id_,3.0);
			CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
			//移除怪对象
			this->removeChild(mon);
			//移除数组中的怪
			monsters_->removeObject(mon);
			std::string overStr = "$";
			char a[6];//把int 型的分数转换成string型的 然后set
			snprintf(a, 6, "%d",money_);
			money_label_->setString((overStr+a).c_str());
			char b[6];//把int 型的分数转换成string型的 然后set
			snprintf(b, 6, "%d",score_);
			score_label_->setString(b);
		}
	}
	//调用Weapon类中发射子弹的方法
	weapon->Fireing();
}

//第三个防御塔的攻击
void GameLayer::FireBulletThree(int weap,int target,float dirction,Point position)
{
	//从存放怪的数组中拿到怪
	Monsters* monster = (Monsters*)monsters_->objectAtIndex(target);
	//从存放防御塔的数组中拿到防御塔
	Weapon* weapon = (Weapon*)weapons_->objectAtIndex(weap);
	//确定发射子弹的数量
	int count[4]={1,2,2,3};

	Point* positionByLevel=new Point[count[weapon->level_-1]];

	float angle[count[weapon->level_-1]];
	//如果当前防御塔的等级为1
	if(weapon->level_==1)
	{
		Point vacter1=ccpForAngle(dirction-(15*3.1415926/180));
		vacter1=ccpNormalize(vacter1);
		positionByLevel[0]=ccpAdd(weapon->getPosition(),ccpMult(vacter1,36));
		angle[0]=dirction;
	}
	//如果当前防御塔的等级为3或者为2
	else if(weapon->level_==3||weapon->level_==2)
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
	else if(weapon->level_==4)
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
	for(int i=0;i<count[weapon->level_-1];i++)
	{
		//创建一个子弹对象
		BulletSprite* bullet = BulletSprite::Create("pic/bullet2.png",weapon->hurt_,target);
		//设置子弹的位置
		bullet->setPosition(positionByLevel[i]);
		//设置子弹的旋转
		bullet->setRotation(-(angle[i]*180/3.1415926));
		//设置子弹的旋转
		bullet->angle_ = -(angle[i]*180/3.1415926);
		//将子弹添加到布景中
		this->addChild(bullet,4);
		//将子弹添加到子弹数组中
		bullets_->addObject(bullet);
	}
}

//发射子弹的方法
void GameLayer::RunBullet()
{
	//如果存放子弹的数组的长度为0或者游戏结束则返回
	if(bullets_->count()==0||is_game_over_)
	{
		return;
	}
	//跟踪算法
	for(int i=0;i<bullets_->count();i++)
	{
		//获得子弹对象
		BulletSprite *bullet = (BulletSprite*)bullets_->objectAtIndex(i);
		//变化向量
		Point vecter;
		//获取子弹的位置坐标
		Point position=bullet->getPosition();
		//如果没有怪则变化向量设为0
		if(monsters_->count()==0)
		{
			vecter.x=0;
			vecter.y=0;
		}
		//如果子弹击中怪数组中的目标对象
		else if(bullet->target_>monsters_->count())
		{
			//获取怪对象
			Monsters* monster=(Monsters*)monsters_->objectAtIndex(1);
			//野怪当前位置的横坐标-子弹当前位置的横坐标
			vecter = monster->getPosition() - bullet->getPosition();
			//如果计算出子弹与防御塔的距离小于20则击中目标怪
			if(vecter.getLength()<20)
			{
				//野怪减血
				monster->CutBlood(bullet->hurt_);
				//如果怪已经没有血了，则删除怪
				if(monster->blood_<=0)
				{
					//获取怪当前位置的坐标点
					Point pointMonster=monster->getPosition();
					//将路径重新给怪
					vector <Point > tempSelfWay = monster->self_way_;
					//定义怪的临时路径的长度
					int tempWay = monster->way_;
					//如果怪的id为3
					if(monster->id_==3)
					{
						//定义锚点
						Point Achorpoint=(Point(0.5,0.4));
						//当第3中怪别打死后会产生两个新的小怪
						for(int i=0;i<2;i++)
						{
							//创建新的小怪
							Monsters* mon = Monsters::Create(6,tempSelfWay);
							//把当前已经走的路径给新的怪
							mon->way_=tempWay;
							//设置怪的位置
							mon->setPosition(pointMonster);
							//设置锚点
							mon->setAnchorPoint(Achorpoint);
							//设置锚点
							Achorpoint=Point(Achorpoint.x+0.2,Achorpoint.y+0.2);
							//把怪添加到怪数组里
							monsters_->addObject(mon);
							//将怪添加到布景中
							this->addChild(mon,6);
							//调用怪移动到终点的方法
							MonsterRun(mon);
						}
					}
					//拿到怪死后得到的金币数
					int tempMoney = monster->id_*10;
					//总的金币数加上怪死后的金币数等于当前金币数
					money_+=tempMoney;
					//拿到怪死后得到的分数
					int tempScore = monster->id_*15;
					//总的分数加上怪死后的分数等于当前分数
					score_+=tempScore;
					//添加怪死时的特效
					this->AddParticle1(monster->getPosition(),monster->id_,3.0);
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
					//删除怪对象
					this->removeChild(monster);
					//删除数组中的怪
					monsters_->removeObject(monster);
					char a[6];//把int 型的分数转换成string型的 然后set
					snprintf(a, 6, "%d",money_);
					std::string overStr = "$";
					//设置金币数
					money_label_->setString((overStr+a).c_str());
					char b[6];//把int 型的分数转换成string型的 然后set
					snprintf(b, 6, "%d",score_);
					//设置分数
					score_label_->setString(b);
				}
				//删除子弹数组中的子弹
				bullets_->removeObject(bullet);
				//删除子弹精灵
				this->removeChild(bullet);

				return;
			}
		}
		else
		{
			//获取子弹指向的野怪对象
			Monsters* monster=(Monsters*)monsters_->objectAtIndex(bullet->target_);
			//野怪当前位置的横坐标-子弹当前位置的横坐标
			vecter.x=monster->getPosition().x-bullet->getPosition().x;
			//野怪当前位置的纵坐标-子弹当前位置的纵坐标
			vecter.y=monster->getPosition().y-bullet->getPosition().y;
			//如果计算出子弹与防御塔的距离小于20则击中目标怪
			if(ccpLength(vecter)<20)
			{
				monster->CutBlood(bullet->hurt_);
				//如果怪没血，则删除
				if(monster->blood_<=0)
				{
					//获取怪当前的位置
					Point pointMonster=monster->getPosition();
					//拿到怪当前走的路径
					vector <Point > tempSelfWay = monster->self_way_;
					//拿到怪已经走的路径
					int tempWay = monster->way_;
					//第三种怪死后会创建两个新怪
					if(monster->id_==3)
					{
						//设置锚点
						Point Achorpoint=(Point(0.5,0.4));
						//通过for循环创建两个新的怪
						for(int i=0;i<2;i++)
						{
							//根据id和路径创建新的怪
							Monsters* mon = Monsters::Create(6,tempSelfWay);
							//拿到大怪已经走的路
							mon->way_=tempWay;
							//设置新怪的位置
							mon->setPosition(pointMonster);
							//设置锚点
							mon->setAnchorPoint(Achorpoint);
							Achorpoint=ccp(Achorpoint.x+0.2,Achorpoint.y+0.2);
							//把新怪添加到怪数组里
							monsters_->addObject(mon);
							//将新怪添加到布景中
							this->addChild(mon,6);
							//调用怪移动到终点的方法
							MonsterRun(mon);
						}
					}
					//定义一个临时存放怪死后所得金币的变量
					int tempMoney = monster->id_*10;
					//更新总的金币数
					money_+=tempMoney;
					//定义一个临时存放怪死后所得分数的变量
					int tempScore = monster->id_*15;
					//更新总的分数
					score_+=tempScore;
					//添加怪死时的特效
					this->AddParticle1(monster->getPosition(),monster->id_,3.0);
					//播放怪死时的音效
					CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/sf_creep_die_0.mp3");
					//删除怪对象
					this->removeChild(monster);
					//删除怪数组中的怪
					monsters_->removeObject(monster);
					char a[6];//把int 型的分数转换成string型的 然后set
					snprintf(a, 6, "%d",money_);
					std::string overStr = "$";
					money_label_->setString((overStr+a).c_str());
					char b[6];//把int 型的分数转换成string型的 然后set
					snprintf(b, 6, "%d",score_);
					score_label_->setString(b);
				}
				//删除子弹数组中的子弹
				bullets_->removeObject(bullet);
				//删除子弹对象
				this->removeChild(bullet);

				return;
			}
		}
		//判断子弹是否超出屏幕，如果超出则将其删除
		if(position.x>800||position.y>480||position.x<0||position.y<0)
		{
			//删除子弹数组中的子弹
			bullets_->removeObject(bullet);
			//删除子弹对象
			this->removeChild(bullet);

			return;
		}
		Point vector = ccpForAngle ((bullet->angle_)*3.1415926/180);
		Point speed=ccpMult(ccpNormalize(ccp(vecter.x+vector.x/15,vecter.y+vector.y/15)),6);
		bullet->setPosition(ccpAdd(bullet->getPosition(),speed));
		float angle = ccpToAngle(speed);
		bullet->setRotation(-(angle*180/3.1415926));
		//设置子弹的角度
		bullet->angle_=angle;
	}
}

//删除所有声明过的精灵对象
void GameLayer::RemoveSprite(Node*node)
{
	this->removeChild(node);
}

//添加防御塔精灵对象
void GameLayer::AddMenuSprite()
{
	//添加表示金币符号的精灵对象
	Sprite* sell = Sprite::create("pic/sell.png");
	//四种武器精灵按钮
	one_player_ = Weapon::Create("pic/button_0.png",1);
	two_player_ = Weapon::Create("pic/button_1.png",2);
	three_player_ = Weapon::Create("pic/button_2.png",3);
	four_player_ = Weapon::Create("pic/button_3.png",4);
	//创建表示升级箭头的精灵对象
	Sprite* update = Sprite::create("pic/update.png");
	//设置升级按钮为不可见
	update->setVisible(false);
	//设置表示卖掉武器的收入金币按钮为不可见
	sell->setVisible(false);

	//设置六个精灵对象的位置
	sell->setPosition(Point(730,68));
	one_player_->setPosition(Point(750,136));
	two_player_->setPosition(Point(750,204));
	three_player_->setPosition(Point(750,272));
	four_player_->setPosition(Point(750,340));
	update->setPosition(Point(730,408));

	//将4个武器菜单按钮添加到布景中
	this->addChild(one_player_, 3);
	this->addChild(two_player_, 3);
	this->addChild(three_player_, 3);
	this->addChild(four_player_, 3);
	//将精灵对象添加到布景中
	this->addChild(sell, 3);
	this->addChild(update, 3);

	//将4个武器菜单按钮添加到相应的数组里
	menus_->addObject(one_player_);
	menus_->addObject(two_player_);
	menus_->addObject(three_player_);
	menus_->addObject(four_player_);
	//将金币精灵对象添加到相应的数组里
	sell_update_menus_->addObject(sell);
	//将升级精灵对象添加到布景中
	sell_update_menus_->addObject(update);
}

//添加所有label
void GameLayer::AddLabel()
{
	//创建一个tempScore文本标签（临时分数）
	score_label_ = LabelTTF::create("0", "Arial",28);
	//创建一个特效并播放
	ActionInterval *act= RotateBy::create(0.1,-90);
	score_label_->runAction(act);
	//设置标签字体的颜色
	score_label_->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	score_label_->setPosition(Point(40,60));
	//将文本标签添加到布景中
	this->addChild(score_label_,3);

	//创建一个用于显示回合数的文本标签
	pass_label_ = LabelTTF::create("1", "Arial",28);
	//设置动作的间隔
	ActionInterval *act1= RotateBy::create(0.1,-90);
	pass_label_->runAction(act1);
	//设置标签字体的颜色
	pass_label_->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	pass_label_->setPosition(Point(40,240));
	//将文本标签添加到布景中
	this->addChild(pass_label_,3);

	//创建一个用于显示金钱的文本标签
	money_label_ = LabelTTF::create("$280", "Arial",28);
	//创建一个特效并播放
	ActionInterval *act2= RotateBy::create(0.1,-90);
	money_label_->runAction(act2);
	//设置标签字体的颜色
	money_label_->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	money_label_->setPosition(Point(40,410));
	//将文本标签添加到布景中
	this->addChild(money_label_,3);

	//创建一个用于显示生命值的ten文本标签
	ten_label_ = LabelTTF::create("18", "Arial",28);
	//创建一个特效并播放
	ActionInterval *act3= RotateBy::create(0.1,-90);
	ten_label_->runAction(act3);
	//设置标签字体的颜色
	ten_label_->setColor (ccc3(255,255,255));
	//横着看的x，y
	Point tar = tmx_layer_->positionAt(Point(targetAll[0][1],targetAll[0][0]));
	//将目标点的坐标转换到世界坐标系中
	Point targetWorld = tmx_layer_->convertToWorldSpaceAR(tar + transform_);
	//设置文本标签的位置
	ten_label_->setPosition(targetWorld);
	//将文本标签添加到布景中
	this->addChild(ten_label_,6);

	//创建一个updateMoney文本标签
	update_money_label_ = LabelTTF::create("$", "Arial",28);
	//将文本标签设置为不可见
	update_money_label_->setVisible(false);
	//创建并播放一个特效
	ActionInterval *act4= RotateBy::create(0.1,-90);
	update_money_label_->runAction(act4);
	//设置标签字体的颜色
	update_money_label_->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	update_money_label_->setPosition(Point(760,408));
	//将文本标签添加到布景中
	this->addChild(update_money_label_,3);

	//创建一个sellMoney文本标签
	sell_money_label_ = LabelTTF::create("$", "Arial",28);
	//将文本标签设置为不可见
	sell_money_label_->setVisible(false);
	//创建一个特效并播放
	ActionInterval *act5= RotateBy::create(0.1,-90);
	sell_money_label_->runAction(act5);
	//设置标签字体的颜色
	sell_money_label_->setColor (ccc3(255,255,255));
	//设置文本标签的位置
	sell_money_label_->setPosition(Point(760,68));
	//将文本标签添加到布景中
	this->addChild(sell_money_label_,3);
}

//野怪到终点时的特效
void GameLayer::AddParticle2(Point point,float time)
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
					CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),//TODO:泄漏
					NULL
			)
		);
	}

	//创建一个特效精灵对象
	cc_ = Sprite::create("pic/fire1.png");
	//声明一个渐现的动作
	ActionInterval *act1=FadeIn::create(time*4/3);
	//声明一个渐隐的动作
	ActionInterval *activeFade1=FadeOut::create(time*4/3);
	//设置动作执行的位置
	cc_->setPosition(point);
	//设置精灵的大小
	cc_->setScale(6.0);
	//将精灵添加到布景中
	this->addChild(cc_,6);
	//顺序执行动作
	cc_->runAction(Sequence::create(
			Spawn::createWithTwoActions
						(
							act1,
							activeFade1
						),
						CallFuncN::create(CC_CALLBACK_1(GameLayer::RemoveSprite,this)),
						NULL
			)
	);
}

//游戏结束时调用的方法
void GameLayer::LoseGame()
{
	//播放游戏结束的音效
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect
	(
		"sound/sf_game_over.mp3"
	);
	//调用计分板
	AchieveLayer* al = new AchieveLayer();
	al->saveScore(score_);
	//遍历防御塔数组
	for(int i=0;i<weapons_->count();i++)
	{
		//声明一个存放4个时间的一维数组
		int a[4]={4,2,1,3};
		//拿到当前的防御塔对象
		Weapon* weap=(Weapon*)weapons_->objectAtIndex(i);
		//添加爆炸的特效
		this->AddParticle1(weap->getPosition(),a[weap->id_-1],5.0);
		//删除防御塔对象
		this->removeChild(weap,true);
	}
	//删除防御塔数组中的所有对象
	weapons_->removeAllObjects();

	//遍历存放怪的数组
	for(int i=0;i<monsters_->count();i++)
	{
		//拿到当前的所有怪
		Monsters* mon=(Monsters*)monsters_->objectAtIndex(i);
		//添加爆炸特效
		this->AddParticle1(mon->getPosition(),mon->id_,5.0);
		//删除怪对象
		this->removeChild(mon,true);
	}
	//删除存放怪数组中的所有怪对象
	monsters_->removeAllObjects();
	//删除所有存放在怪动作数组中的对象
	actions_->removeAllObjects();

	//遍历所有存放子弹的数组
	for(int i=0;i<bullets_->count();i++)
	{
		//拿到子弹对象
		BulletSprite* bullet=(BulletSprite*)bullets_->objectAtIndex(i);
		//添加爆炸特效
		this->AddParticle1(bullet->getPosition(),1,5.0);
		//删除子弹精灵
		this->removeChild(bullet,true);
	}
	//删除子弹数组中的所有对象
	bullets_->removeAllObjects();
	//添加特效
	this->AddParticle2(end_world_,5.0);

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
