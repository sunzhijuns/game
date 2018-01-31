#include "GameLayer.h"
#include "cocos2d.h"
#include "HKMJObject.h"
#include "WarShipObject.h"
#include "Weapon.h"
#include "AppMacros.h"
#include <list>
#include <queue>
#include <audio/include/SimpleAudioEngine.h>
#include "ChooseLevelLayer.h"
#include "Constant.h"
using namespace cocos2d;

int** visited;
//
//GameLayer::~GameLayer() {
//}

bool GameLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}

	//创建一些列向量
	all_weapon_vector_=new std::vector<Weapon*>();
	delete_weapon_vector_=new std::vector<Weapon*>();

	all_ship_vector_=new std::vector<WarShipObject*>();
	delete_ship_vector_=new std::vector<WarShipObject*>();

	path_=new std::vector<Point>();
	temp_path_=new std::vector<Point>();

	//初始化地图中的
	InitTMXMap();
	InitTMXPZJCArray();
	InitHKMJ();
	InitJJItem();
	InitBoomFrame();
	InitBigBoomFrame();
	InitPauseMenu();

	map_->runAction(
			Sequence::create(
					DelayTime::create(1.0f),
					CallFunc::create(CC_CALLBACK_0(GameLayer::GameStartDaoJiShi,this)),
					CallFunc::create(CC_CALLBACK_0(GameLayer::InitListenerTouchAndCallback,this)),
					NULL
			)
	);

	InitSound();
	InitOceanBg();
	InitMiscellaneous();

	if(ChooseLevelLayer::bacMusicFlag == true)
	{
		CocosDenshion::SimpleAudioEngine::getInstance()//播放背景音乐
								->playBackgroundMusic("mysound/sound_bgm_battlefield.mp3");
	}
	Texture2D* texture1 = Director::getInstance()->getTextureCache()->addImage("gamePic/coolpic.png");
	//添加武器冷却的精灵
	for(int i = 0;i<2;i++)
	{
		weapone_sprite_[i] = Sprite::createWithTexture(texture1);
		weapone_sprite_[i]->setTextureRect(Rect(0,0,90,0));
		weapone_sprite_[i]->setAnchorPoint(Vec2(0.5,1));
		weapone_sprite_[i]->setPosition(55+i*100, 105);
		this->addChild(weapone_sprite_[i],100);
	}
	//添加船只冷却时间的精灵
	for(int i = 0;i<5;i++)
	{
		ship_cool_sprite_[i] = Sprite::createWithTexture(texture1);
		ship_cool_sprite_[i]->setTextureRect(Rect(0,0,90,0));
		ship_cool_sprite_[i]->setAnchorPoint(Vec2(0.5,1));
		ship_cool_sprite_[i]->setPosition(55+i*100, 105);
		this->addChild(ship_cool_sprite_[i],100);
	}

	InitRaining();
	return true;
}
void GameLayer::InitListenerTouchAndCallback()
{
	//创建一个单点触摸监听
	EventListenerTouchOneByOne*listener_touch_onebyone = EventListenerTouchOneByOne::create();
	//设置下传触摸
	listener_touch_onebyone->setSwallowTouches(false);

	listener_touch_onebyone->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan,this);
	listener_touch_onebyone->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved,this);
	listener_touch_onebyone->onTouchEnded = CC_CALLBACK_2(GameLayer::onTouchEnded,this);

	//添加到监听器
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener_touch_onebyone,map_);
	//定时回调
	scheduleUpdate();
	Scheduler* scheduler = _director->getScheduler();
	scheduler->schedule(SEL_SCHEDULE(&GameLayer::CalculateNearestSmartIn),this,0.1,false);
	scheduler->schedule(SEL_SCHEDULE(&GameLayer::WeaponsCoolTimeUpdate),this,0.1,false);
	scheduler->schedule(SEL_SCHEDULE(&GameLayer::RemoveShipUpdate),this,0.1,false);
	scheduler->schedule(SEL_SCHEDULE(&GameLayer::WeaponeStateUpdate),this,0.01f,false);
	scheduler->schedule(SEL_SCHEDULE(&GameLayer::EnemyShipAtHKMJUpdate),this,0.01f,false);
	scheduler->schedule(SEL_SCHEDULE(&GameLayer::OceanUpdate),this,0.01f,false);
	scheduler->schedule(SEL_SCHEDULE(&GameLayer::PlayerHKMJCoolTimeUpdate),this,3.0f,false);
	scheduler->schedule(SEL_SCHEDULE(&GameLayer::PlaneUpdate),this,0.01f,false);
}

void GameLayer::update(float delta) {

}

void GameLayer::InitRaining()
{
	ParticleSystemQuad* particle_system_quad;
	if(ChooseLevelLayer::levelNum == 1)
	{
		particle_system_quad = ParticleSystemQuad::create("lzxt/snow.plist");

	}else if(ChooseLevelLayer::levelNum == 2)
	{
		particle_system_quad = ParticleSystemQuad::create("lzxt/raining.plist");

	}else if(ChooseLevelLayer::levelNum == 3)
	{
		particle_system_quad = ParticleSystemQuad::create("lzxt/snow.plist");

	}else if(ChooseLevelLayer::levelNum == 4)
	{
		particle_system_quad = ParticleSystemQuad::create("lzxt/raining.plist");

	}
//	particle_system_quad->retain();								//保持引用
	particle_system_quad->setBlendAdditive(true);					//设置混合方式为增加
	particle_system_quad->setPosition(Point(480,270));
	this->addChild(particle_system_quad, kParticleZOrder);		//向布景层中的精灵添加粒子系统
}

void GameLayer::InitMiscellaneous()
{
	Sprite* money_sprite = Sprite::create("gamePic/money.png");
	money_sprite->setPosition(Point(940,520));
	this->addChild(money_sprite,kItemZOrder);

	Sprite* sds_sprite = Sprite::create("gamePic/sds.png");
	sds_sprite->setPosition(Point(940,490));
	this->addChild(sds_sprite,kItemZOrder);


	std::string my_gold_string = StringUtils::format("%d",my_gold_);
	money_label_ = LabelAtlas::create(				//创建一个LabelAtlas对象
			my_gold_string,"gamePic/labelatlas.png",15,19,'0');
	money_label_->setAnchorPoint(Point(1,0.5));
	money_label_->setPosition(money_sprite->getPosition().x-20,money_sprite->getPosition().y);
	this->addChild(money_label_,kItemZOrder);

	sds_num_ = UserDefault::getInstance()->getIntegerForKey(Constant::SHADISHU.c_str());
	std::string sds_string = StringUtils::format("%d",sds_num_);

	sds_label_ = LabelAtlas::create(				//创建一个LabelAtlas对象
			sds_string,"gamePic/labelatlas.png",15,19,'0');
	sds_label_->setAnchorPoint(Point(1,0.5));
	sds_label_->setPosition(sds_sprite->getPosition().x-20,sds_sprite->getPosition().y);
	this->addChild(sds_label_,kItemZOrder);
}
void GameLayer::InitOceanBg()
{

	Sprite* ocean_bg0_sprite = Sprite::create("map/oceanbg0.png");
	ocean_bg0_sprite->setPosition(568,384);
	map_->addChild(ocean_bg0_sprite,kOceanBg0ZOrder);
	for(int i=0;i<2;i++)
	{
		ocean_bg1_sp_[i] = Sprite::create("map/oceanbg1.png");
		ocean_bg1_sp_[i]->setPosition(Point(568-i*1136,384));
		map_->addChild(ocean_bg1_sp_[i],kOceanBg1ZOrder);
	}
}
void GameLayer::InitSound()
{
	CocosDenshion::SimpleAudioEngine::getInstance()							//背景音效
						->preloadBackgroundMusic("mysound/sound_bgm_battlefield.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_explode_general.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_nuclear.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_missile.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_star_1.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_star_2.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_star_3.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_break.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_error.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/sound_sfx_destination.mp3");
	CocosDenshion::SimpleAudioEngine::getInstance()							//即使音效
						->preloadEffect("mysound/flyby2.wav");
}
void GameLayer::InitPauseMenu()
{

	pause_menu_ = MenuItemImage::create(
				"gamePic/pause.png",
				"gamePic/pausep.png",
				CC_CALLBACK_0(GameLayer::PauseCallback, this)
		);
	pause_menu_->setPosition(30,510);

	//创建一个菜单对象
	Menu* menu = Menu::create(pause_menu_,NULL);
	//设置其位置
	menu->setPosition(Point::ZERO);
	//将其添加到布景中
	this->addChild(menu, 10);
}
void GameLayer::PauseCallback()
{
	PlaySound();
	is_game_over_=true;
	for(int i =0;i<5;i++)
	{
		jj_bg_sp_[i]->setEnabled(false);
	}
	for(int i =0;i<3;i++)
	{
		ship_weapon_[i]->setEnabled(false);
	}
	pause_menu_->setEnabled(false);
	Director::getInstance()->pause();

	game_pause_bb_ = Sprite::create("puase/btmbb.png");
	game_pause_bb_->setPosition(Point(480,270));
	this->addChild(game_pause_bb_,kGamePauseBBZOrder);

	Sprite* game_over_title_sp = Sprite::create("puase/pausett.png");
	game_over_title_sp->setPosition(Point(480,410));
	game_pause_bb_->addChild(game_over_title_sp,12);

	Sprite* game_over_bb_sp = Sprite::create("puase/bbp.png");
	game_over_bb_sp->setAnchorPoint(Point(0,1));
	game_over_bb_sp->setPosition(0,0);
	game_over_title_sp->addChild(game_over_bb_sp,9);

	MenuItemImage* back_item = MenuItemImage::create(
			"puase/back.png",
			"puase/backp.png",
			CC_CALLBACK_0(GameLayer::BackCallback, this)
	);
	back_item->setPosition(230,90);

	MenuItemImage* restart_item = MenuItemImage::create(
			"gameover/cxks.png",
			"gameover/cxksp.png",
			CC_CALLBACK_0(GameLayer::RestartCallback, this)
	);
	restart_item->setPosition(90,90);

	MenuItemImage* tui_chu_item = MenuItemImage::create(
			"gameover/tc.png",
			"gameover/tcp.png",
			CC_CALLBACK_0(GameLayer::TuiChuCallback, this)
	);
	tui_chu_item->setPosition(Point(370,90));

	//创建一个菜单对象
	Menu* menu = Menu::create(back_item,restart_item,tui_chu_item,NULL);
	//设置其位置
	menu->setPosition(Point::ZERO);
	//将其添加到布景中
	game_over_bb_sp->addChild(menu, 2);
}
void GameLayer::BackCallback()
{
	PlaySound();
	is_game_over_=false;
	for(int i =0;i<5;i++)
	{
		jj_bg_sp_[i]->setEnabled(true);
	}
	for(int i =0;i<3;i++)
	{
		ship_weapon_[i]->setEnabled(true);
	}
	this->removeChild(game_pause_bb_);
	pause_menu_->setEnabled(true);
	Director::getInstance()->resume();
}
void GameLayer::GameStartDaoJiShi()
{
	if(wave_num_ == 1)
	{
		Sprite* wave_sprite= Sprite::create("gamePic/wave.png");
		wave_sprite->setPosition(Point(380,520));
		this->addChild(wave_sprite, kWaveZOrder);

		Sprite* bo_shu = Sprite::create("gamePic/bs.png");
		bo_shu->setPosition(460,520);
		this->addChild(bo_shu,kWaveZOrder);


		label_wave_index_ = Label::createWithTTF(
						StringUtils::format("%d/5",wave_num_),"fonts/FZKATJW.ttf",50
				);
		label_wave_index_->setPosition(540,520);
		label_wave_index_->enableOutline(Color4B(0,0,0,255),2);
		this->addChild(label_wave_index_,kWaveZOrder);
	}else
	{
		label_wave_index_->setString(StringUtils::format("%d/5",wave_num_));
	}
	//左侧的倒计时
	Sprite* sp1 = Sprite::create("gamePic/nextFlag0.png");
	sp1->setPosition(50,270);
	this->addChild(sp1,kWaveZOrder);
	sp1->runAction(
			Sequence::create(
					Repeat::create(
							Sequence::create(
									ScaleTo::create(0.5,1.1),
									ScaleTo::create(0.5,0.9),
									NULL),
							10),
					RemoveSelf::create(true),
					NULL
			)
	);

	ProgressTo* action_to = ProgressTo::create(10, 100);		//创建一个持续两秒从0%到100%的动作
	ProgressTimer* radial_progress_timer = ProgressTimer::create(			//创建一个包装着精灵的ProgressTimer对象
								 Sprite::create("gamePic/nextFlag1.png"));
	radial_progress_timer->setPosition(55, 270); 	//设置包装着精灵的ProgressTimer对象的位置
	radial_progress_timer->setMidpoint(Point(0.5f, 0.5f)); 	//设置百分比效果的参考点
	radial_progress_timer->setType(ProgressTimer::Type::RADIAL);	//设置为半径模式
	this->addChild(radial_progress_timer,kWaveZOrder+1);				//将包装着精灵的ProgressTimer对象添加到布景中

	radial_progress_timer->runAction(
			Sequence::create(
					Spawn::create(
							action_to->clone(),
							Repeat::create(
									Sequence::create(
										ScaleTo::create(0.5,1.1),
										ScaleTo::create(0.5,0.9),
										NULL
										),
								10),NULL
					),
					CallFunc::create(CC_CALLBACK_0(GameLayer::StartCallback,this)),
					RemoveSelf::create(true),
					NULL
					)
	);		//重复执行百分比动作
	//第几波敌船即将来袭
	Sprite* ship_wave = Sprite::create("gamePic/shipwave.png");
	ship_wave->setAnchorPoint(Point(0,0.5));
	ship_wave->setPosition(Point(960,270));
	this->addChild(ship_wave,kShipWaveZOrder);
	ship_wave->runAction(
			Sequence::create(
					MoveTo::create(10,Point(-530,270))
					,RemoveSelf::create(true)
					,NULL
			)
	);

	Label* wave_index = Label::createWithTTF(
					StringUtils::format("%d",wave_num_),"fonts/FZKATJW.ttf",60
			);
	wave_index->setTextColor(Color4B(255,0,0,255));
	wave_index->enableOutline(Color4B(255,255,255,255),1);
	wave_index->setPosition(105,35);
	ship_wave->addChild(wave_index,2);
}
void GameLayer::StartCallback()
{
	enemy_ship_count_=10;
	//添加敌船
	AddEnemyShip();
	wave_num_ ++;
}
void GameLayer::InitBoomFrame()
{
	std::string sa[6]={						//将所有动画图片的路径储存在数组中
		"hzjl/boom1.png","hzjl/boom2.png","hzjl/boom3.png",
		"hzjl/boom4.png","hzjl/boom5.png","hzjl/boom6.png"
	};
	Vector<SpriteFrame*> animate_frames;//创建存放动画帧的列表对象

	SpriteFrame *f0=SpriteFrame::create(sa[0],
						Rect(0,0,104,102));
	animate_frames.pushBack(f0);

	SpriteFrame *f1=SpriteFrame::create(sa[1],
						Rect(0,0,107,102));
	animate_frames.pushBack(f1);

	SpriteFrame *f2=SpriteFrame::create(sa[2],
						Rect(0,0,97,95));
	animate_frames.pushBack(f2);

	SpriteFrame *f3=SpriteFrame::create(sa[3],
						Rect(0,0,93,92));
	animate_frames.pushBack(f3);

	SpriteFrame *f4=SpriteFrame::create(sa[4],
						Rect(0,0,53,54));
	animate_frames.pushBack(f4);

	SpriteFrame *f5=SpriteFrame::create(sa[5],
						Rect(0,0,53,53));
	animate_frames.pushBack(f5);

	Animation *animation=Animation::createWithSpriteFrames
			(animate_frames, 0.1f);					//创建指向动画对象的指针

	animate_action_[1]=Animate::create(animation);			//创建动画动作对象
	animate_action_[1]->retain();						//因为暂时不用，所以保持引用，防止被自动释放
}
void GameLayer::InitBigBoomFrame()
{
	std::string sa[4]={						//将所有动画图片的路径储存在数组中
		"hzjl/bigBoom1.png","hzjl/bigBoom2.png",
		"hzjl/bigBoom3.png","hzjl/bigBoom4.png"
	};
	Vector<SpriteFrame*> animate_frames;		//创建存放动画帧的列表对象
	SpriteFrame *f0 = NULL;
	for(int i=0;i<4;i++)
	{
		f0 = SpriteFrame::create(sa[i],
							Rect(0,0,166,160));
		animate_frames.pushBack(f0);
	}
	Animation *animation=Animation::createWithSpriteFrames
			(animate_frames, 0.2f);					//创建指向动画对象的指针
	animate_action_[0]=Animate::create(animation);			//创建动画动作对象
	animate_action_[0]->retain();						//因为暂时不用，所以保持引用，防止被自动释放
}
void GameLayer::ExpansionRing()
{
	Sprite* ring_sprite = Sprite::create("hzjl/bigBoom5.png");
	ring_sprite->setPosition(568,384);
	map_->addChild(ring_sprite,1000);
	ring_sprite->runAction(
			Sequence::create(
					ScaleTo::create(2,3),
					RemoveSelf::create(true),
					NULL)
	);

	std::vector<WarShipObject*>::iterator all_ship ;
	for(all_ship =all_ship_vector_->begin();all_ship!= all_ship_vector_->end();all_ship++)
	{
		if((*all_ship)->shipType == WarShipObject::kEnemy)
		{
			(*all_ship)->lifeValue =0;
		}
	}
}
//初始化TMX地图的方法
void GameLayer::InitTMXMap()
{
	std::string map_path =StringUtils::format("map/map_%d.tmx",ChooseLevelLayer::levelNum);
	//加载TMX地图
	map_ = TMXTiledMap::create(map_path);

	//获取用于碰撞检测的层
	collide_layer_ = map_->getLayer("pzjcLayer");

	//设置碰撞检测层不可见
	collide_layer_->setVisible(true);
	map_->setAnchorPoint(Point(1,0));
	map_->setPosition(Point(960,0));
	//将TMX地图添加进布景
	this->addChild(map_, 0);
	map_point_ = map_->getPosition();
	int map_width = map_->getMapSize().width;
	int map_height = map_->getMapSize().height;
	row_ = map_width;		//71
	col_ = map_height;   	//48
	//获得单个图块的大小，为了在绘制时得到偏移量，否则绘制出来的线条有半个图块的偏移差
	auto tile = collide_layer_->getTileAt(Vec2(0,0));
	auto texture = tile->getTexture();
	auto block_size = texture->getContentSize();
	transform_ = Point(0,block_size.height/2);
}
void GameLayer::InitTMXPZJCArray()
{
	//创建动态二维数组
	map_data_ = new int* [row_];
	for(int i = 0; i<row_; i++)
	{
		map_data_[i] = new int[col_];
	}
	//获得一个图素中的属性值
	for(int i=0; i<row_; i++)
	{
		for(int j=0; j<col_; j++)
		{
			//得到layer中每一个图块的gid
			unsigned int gid = collide_layer_->getTileGIDAt(Vec2(i,j));
			//通过gid得到该图块中的属性集,属性集中是以键值对的形式存在的
			auto tile_dic = map_->getPropertiesForGID(gid);
			//通过键得到value
			const __String value = tile_dic.asValueMap()["sfpz"].asString();
			//将value转换成int变量
			int mv = value.intValue();
			//初始化地图中的数据
			map_data_[i][j] = mv;
		}
	}
	//创建数组
	visited = new int*[row_];
	for(int i = 0; i<row_; i++)
	{
		visited[i] = new int[col_];
	}
	InitVisitedArray();
}
//初始化寻径的数组-----将其设置为没有走过
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
bool GameLayer::OnShipTouchBegan(Touch* touch, Event* event)
{
	if(is_game_over_ == true)
	{
		return false;
	}
	return true;
}

void GameLayer::OnShipTouchEnded(Touch* touch, Event* event)
{
	WarShipObject* target_ship = static_cast<WarShipObject*>(	//获取当前触摸对象，并转化为精灵类型
							event->getCurrentTarget());
	Point location = target_ship->convertToNodeSpace(touch->getLocation());//获取当前坐标
	Size size = target_ship->getContentSize();			//获取精灵的大小
	Rect rect = Rect(0,0,size.width, size.height);//创建一个矩形对象，其大小与精灵相同
	if(rect.containsPoint(location))
	{
		//设置船只冷却时间不可见
		for(int k = 0;k<5;k++)
		{
			ship_cool_sprite_[k]->setVisible(false);
		}
		//判断当前点击的船只是否为上次点击的船只
		if(last_touch_sprite_ == target_ship && target_ship->shipType == WarShipObject::kPlayer)
		{
			if(target_ship->state == WarShipObject::kNotSelected)
			{
				target_ship->state = WarShipObject::kSelected;
				ShowWarShipItem(target_ship);
				WarShipAddChild(target_ship);
				select_ship_ = true;
			}
			last_touch_sprite_ = target_ship;
			curr_touch_sprite_ = target_ship;
		}else if(last_touch_sprite_ != target_ship && target_ship->shipType == WarShipObject::kPlayer)
		{	//若不为上次点击的船只
			if(last_touch_sprite_ != NULL)
			{
				last_touch_sprite_->state = WarShipObject::kNotSelected;
				ShowWarShipItem(last_touch_sprite_);
				last_touch_sprite_->removeChildByTag(DWPTAG);
				last_touch_sprite_->removeChildByTag(DWSPRITETAG);
				last_touch_sprite_->removeChildByTag(COMPASSSPRITE);
				last_touch_sprite_->removeChildByTag(BQSPRITETAG);
//				select_ship_ = false;
			}
			target_ship->state = WarShipObject::kSelected;//标记为选中状态
			ShowWarShipItem(target_ship);
			WarShipAddChild(target_ship);
			select_ship_ = true;
			last_touch_sprite_ = target_ship;
			curr_touch_sprite_ = target_ship;
		}
	}
}
//添加船只的子精灵---范围圈----瞄准点
void GameLayer::WarShipAddChild(WarShipObject *war_ship_object)
{
	Size size = war_ship_object->getContentSize();
	Sprite* compass_sprite = Sprite::create("gamePic/compass.png");
	compass_sprite->setPosition(size.width/2,size.height/2);
	war_ship_object->addChild(compass_sprite,1,COMPASSSPRITE);
	compass_sprite->runAction(RepeatForever::create(RotateBy::create(8,360)));

	Sprite* bq_sprite = Sprite::create("gamePic/bq.png");
	bq_sprite->setAnchorPoint(Point(0,0));
	bq_sprite->setPosition(size.width/2,size.height/2);
	war_ship_object->addChild(bq_sprite,1,BQSPRITETAG);

	AddSaveSprite(war_ship_object);
	CCLOG("WarShipAddChild--END");
}
//显示当前应该显示的项
void GameLayer::ShowWarShipItem(WarShipObject *war_ship_object)
{
	bool flag ;
	if(war_ship_object->state == WarShipObject::kNotSelected)
	{
		flag = true;
	}else if(war_ship_object->state == WarShipObject::kSelected)
	{
		flag = false;
	}
	for(int i = 0;i<5;i++)
	{
		jj_bg_sp_[i] ->setVisible(flag);
	}
	ship_weapon_[0] ->setVisible(!flag);
	ship_weapon_[2] ->setVisible(!flag);
	if(war_ship_object->itemCount == 2)
	{
        ship_weapon_[1]->setVisible(!flag);
	}
}
void GameLayer::AddTouchListener(WarShipObject *war_ship_object)
{
	//创建一个单点触摸监听
	EventListenerTouchOneByOne* listener_touch_ship = EventListenerTouchOneByOne::create();
	//设置下传触摸
	listener_touch_ship->setSwallowTouches(false);
	listener_touch_ship->onTouchBegan = CC_CALLBACK_2(GameLayer::OnShipTouchBegan, this);
	listener_touch_ship->onTouchEnded = CC_CALLBACK_2(GameLayer::OnShipTouchEnded, this);
	//添加到监听器
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener_touch_ship,war_ship_object);
}
//添加军舰的方法
void GameLayer::AddWarShip(Point touch_point, int select_num)
{
	//将触摸点装换为格子行列号
	Point temp_point ;
	temp_point = TouchPointToRowCol(touch_point);
	//判断点击的格子是否能走
	if(map_data_[(int)temp_point.x][(int)temp_point.y] == 1)
	{

		AddWrongPrompt(touch_point);
		return;
	}
	int my_gold_temp = my_gold_-(select_index_ + 1)*400;
	if(my_gold_temp < 0)
	{
		AddWrongPrompt(touch_point);
		money_label_->runAction(
				Sequence::create(
						ScaleTo::create(1,2),
						ScaleTo::create(1,1),
						NULL
				)
		);
		return ;
	}
	my_gold_ = my_gold_temp;
	std::string rest_money = StringUtils::format("%d",my_gold_);
	money_label_->setString(rest_money);

	Sprite* mz_sprite = Sprite::create("gamePic/MZ.png");
	mz_sprite->setPosition(touch_point);
	map_->addChild(mz_sprite,10);
	if(ChooseLevelLayer::froMusicFlag == true)
	{
		CocosDenshion::SimpleAudioEngine::getInstance()
										->playEffect("mysound/sound_sfx_destination.mp3");
	}

	mz_sprite->runAction(
			Sequence::create(
					ScaleTo::create(0.5,1.2),
					ScaleTo::create(0.5,0.9),
					RemoveSelf::create(true),
					NULL)
	);


	//能走，则设置其为目标点
	target_[0] = (int)temp_point.y;
	target_[1] = (int)temp_point.x;
	//创建一个船只
	WarShipObject* war_ship_object = WarShipObject::create(
			StringUtils::format("gamePic/warShip%d.png",select_num),Point(1120,368),select_num,1);

	//给当前船只赋值方向优先顺序
	for(int i = 0;i<8;i++)
	{
		for(int j = 0;j<2;j++)
		{
			war_ship_object->sequence[i][j] = SEQUENCEZARRAY[i][j];
		}
	}
	war_ship_object->shipNum = select_num;
	//设置船只的目标点
	war_ship_object->targetRow = temp_point.y;
	war_ship_object->targetCol = temp_point.x;
	map_->addChild(war_ship_object,10);

	//给船只添加监听
	AddTouchListener(war_ship_object);
	//添加到所有船只里面
	all_ship_vector_->push_back(war_ship_object);
	//调用搜索路径的方法，搜索行走路径
	SearchPath(war_ship_object);
	//冷却时间复赋值
	switch(select_num)
	{
	case 0:
		player_hkmj_->coolTime0 = 104;
		break;
	case 1:
		player_hkmj_->coolTime1 = 104;
		break;
	case 2:
		player_hkmj_->coolTime2 = 104;
		break;
	case 3:
		player_hkmj_->coolTime3 = 104;
		break;
	case 4:
		player_hkmj_->coolTime4 = 104;
		break;
	}
}
//随机重置搜索路径的方向优先顺序
void GameLayer::RandomSort()
{
	int temp_sequence[8][2] =
		{
			{0,1},{-1, 0},{1, 0},
			{0,-1},{-1, 1},
			{-1, -1},{1, -1},{1, 1}
		};
	int temp_index[] = {0,1,2,3,4,5,6,7};
	int random = 0;
	for (int i = 8; i > 0; --i) {
		random = rand()%i;//0, i-1
		int temp = temp_index[i];
		temp_index[i] = temp_index[random];
		temp_index[random] = temp;
	}
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 2; ++j) {
			result_sequence_[i][j] = temp_sequence[temp_index[i]][j];
		}
	}
}
//搜索路径
void GameLayer::SearchPath(WarShipObject *war_ship)
{
    BFSCalculatePath(war_ship);
	war_ship->warShipPath = path_;
	CreateFiniteTimeActionByVector(war_ship);
}
//将触摸点装换为格子行列号
Point GameLayer::TouchPointToRowCol(Point touch_point)
{
	Point result_point;
	int row = (int)(touch_point.x/CELLSIZE); //+ ((((int)touch_point.x)%CELLSIZE == 0)?0:1);
	int col = (int)((MAPWIDTH-touch_point.y)/CELLSIZE);// + (((int)(MAPWIDTH-touch_point.y)%CELLSIZE == 0)?0:1);
	result_point = Point(row,col);
	return result_point;
}
//广度优先算法
void GameLayer::BFSCalculatePath(WarShipObject* war_ship)
{
	my_queue_ = new std::queue<int(*)[2]>();
	hash_map_ =new std::map<std::string, int(*)[2]>();
	hash_map_->clear();
	for(int i=0;i<row_;i++)
	{
		for(int j=0;j<col_;j++)
		{
			visited[i][j] = 0;
		}
	}

	bool flag = true;
	//储存开始点和目标点的数组
	int (*start)[2] = new int[2][2];
	if((war_ship->targetRow != -1&&war_ship->targetCol != -1 && war_ship->state == WarShipObject::kSelected))
	{
		war_ship->initRow = war_ship->targetRow;
		war_ship->initCol = war_ship->targetCol;
	}
	//设置起始点
	start[0][0] = war_ship->initRow;
	start[0][1] = war_ship->initCol;
	//设置目标点
	start[1][0] = war_ship->initRow;
	start[1][1] = war_ship->initCol;

	//将开始点放进队列中
	my_queue_->push(start);
	while(flag)
	{
		//取出所有点
		int(*current_edge)[2] = my_queue_->front();
		//弹出队列中的数据
		my_queue_->pop();
		//获取目标点-------下标为1
		int* temp_target = current_edge[1];
		//判断目标点是否访问过
		if(visited[temp_target[1]][temp_target[0]] == 1)
		{
			continue;
		}
		//标记目的点为已访问过
		visited[temp_target[1]][temp_target[0]] = 1;
		//将数字转换成字符串
		std::string str1;
		std::string str2;
		str1 = StringUtils::format("%d", temp_target[0]);
		str2 = StringUtils::format("%d", temp_target[1]);
		//记录此临时目的地的点的父节点
		hash_map_->insert(std::map<std::string,int(*)[2]>::value_type(str1+":"+str2,current_edge));
		//判断是否找到目标点
		if(temp_target[0]==target_[0] && temp_target[1]==target_[1])
		{
			break;
		}
		//将所有可能的点的行列号入队列
		int curr_col = temp_target[0];
		int curr_row = temp_target[1];
		int(*sequence)[2] = NULL;
		sequence = ((war_ship->sequence));

		//循环8个方向进行搜索
		for(int m = 0; m<8; m++)
		{
			int* rc = sequence[m];
			int i = rc[1];
			int j = rc[0];
			if(i==0 && j==0)
			{
				continue;
			}
			//如果该行列在地图范围内则进行下一步的计算
			if(curr_row+i>=0 && curr_row+i<row_ && curr_col+j>=0 && curr_col+j<col_ &&
					map_data_[curr_row+i][curr_col+j]!=1)
			{
				//创建二维数组
				int (*temp_edge)[2] = new int[2][2];
				temp_edge[0][0] = temp_target[0];
				temp_edge[0][1] = temp_target[1];
				temp_edge[1][0] = curr_col+j;
				temp_edge[1][1] = curr_row+i;
				//将二维数组添加进队列中
				my_queue_->push(temp_edge);
			}
		}
	}
	TransformBTMapAndVector(war_ship);
}
void GameLayer::TransformBTMapAndVector(WarShipObject *war_ship_object)
{
	temp_path_->clear();
	std::string str1;
	std::string str2;
//	auto tar = colliLayer->positionAt(Point(target_[1],target_[0]));//
	//绘制最终的搜索结果路径
	std::map<std::string, int(*)[2]>::iterator iter;	//声明map容器迭代器
	int* temp = target_;									//记录终点
	while(true)
	{
//		double xx;
//		double yy;
		//将数字转换成字符串
		str1 = StringUtils::format("%d", temp[0]);
		str2 = StringUtils::format("%d", temp[1]);
		std::string key = str1+":"+str2;
		iter = hash_map_->find(key);
		int (*tempA)[2] = iter->second;
		//如果查找到了元素
		Point start;
		Point end;
		if(iter != hash_map_->end())
		{

			//根据行列得到colliLayer中该图块的位置
			end = collide_layer_->getPositionAt(Vec2(tempA[1][1],tempA[1][0]));

			end.x = end.x+transform_.x;
			end.y = end.y+transform_.y;
			temp_path_->push_back(end);
		}
		//判断有否到出发点
		if(tempA[0][0]==war_ship_object->initRow&&tempA[0][1]==war_ship_object->initCol)
		{
			//根据行列得到colliLayer中该图块的位置
			start = collide_layer_->getPositionAt(Vec2(tempA[0][1],tempA[0][0]));
			start.x = start.x+ transform_.x;
			start.y = start.y+ transform_.y;
			temp_path_->push_back(start);
			break;
		}
		temp = tempA[0];//线段的起点数组
	}
	TransformPath();
}
//放到储存行驶路径的向量中
void GameLayer::TransformPath()
{
	path_ ->clear();
	std::vector<Point>::iterator temp_point;
	for(temp_point = temp_path_->end()-1;temp_point != temp_path_->begin()-1;temp_point--)
	{
		path_->push_back(*temp_point);
	}
}
void GameLayer::CreateFiniteTimeActionByVector(WarShipObject *war_ship_object)
{
	//计数器，记录当前的路线条数
	int count_temp=1;
	FiniteTimeAction* pre_action;
	Point last_point=war_ship_object->startPoint;
	Point curr_point=war_ship_object->startPoint;
	int for_count = 0;
	std::vector<Point>::iterator temp_point;
	for(temp_point = war_ship_object->warShipPath->begin();temp_point != war_ship_object->warShipPath->end();temp_point++)
	{
		if(for_count == 0)
		{
			last_point = (*temp_point);
			for_count = 1;
			continue;
		}
		curr_point=(*temp_point);
		float dx=curr_point.x-last_point.x;
		float dy=curr_point.y-last_point.y;
		float dis =(float)sqrt(dx*dx+dy*dy);
		float move_time =0;
		move_time = dis/war_ship_object->speed;
		int temp_rotation = OrderDirection(dx,dy);
		if(war_ship_object->rotation != temp_rotation)
		{
			war_ship_object->rotation = temp_rotation;
			RotateTo* rtemp = RotateTo::create(move_time,war_ship_object->rotation);

			if(count_temp==1)
			{
				pre_action=rtemp;
			}else if(count_temp>1)
			{
				pre_action=Sequence::createWithTwoActions(pre_action,rtemp);
			}
			MoveTo* mtemp=MoveTo::create(move_time,*temp_point);
			pre_action=Sequence::createWithTwoActions(pre_action,mtemp);
		}else
		{
			if(count_temp==1)
			{
				MoveTo* mtemp=MoveTo::create(move_time,*temp_point);
				pre_action=mtemp;
			}
			else if(count_temp>1)
			{
				MoveTo* mtemp=MoveTo::create(move_time,*temp_point);
				pre_action=Sequence::createWithTwoActions(pre_action,mtemp);
			}
		}
		count_temp++;
		last_point=curr_point;
	}
	war_ship_object->runAction(Sequence::create(
			pre_action,NULL));
}
//获取旋转角度的方法
int  GameLayer::OrderDirection(float dx, float dy)
{
	if(dx == 0 && dy>0)
	{
		return 90;
		//上
	}else if(dx == 0 && dy<0)
	{
		return -90;
		//下
	}else if(dy == 0 && dx >0)
	{
		return 180;
		//右
	}else if(dy == 0 && dx <0)
	{
		return 0;
		//左
	}else if(dx <0 && dy >0)
	{
		return 45;
		//左上
	}else if(dx >0 && dy >0)
	{
		return 135;
		//右上
	}
	else if(dx <0 && dy <0)
	{
		return -45;
		//左下
	}
	else if(dx >0 && dy <0)
	{
		return -135;
		//右下
	}
	return 0;
}
//武器、船只的冷却时间的回调方法
void GameLayer::WeaponsCoolTimeUpdate()
{
	std::vector<WarShipObject*>::iterator war_ship ;
	for(war_ship = all_ship_vector_->begin();war_ship != all_ship_vector_->end();war_ship++)
	{
		if((*war_ship)->weapon0CoolTime > 0)
		{
			(*war_ship)->weapon0CoolTime --;
		}
		if((*war_ship)->weapon1CoolTime > 0)
		{
			(*war_ship)->weapon1CoolTime --;
		}
	}
	if(curr_touch_sprite_ != NULL)
	{
		weapone_sprite_[0]->setVisible(true);
		weapone_sprite_[1]->setVisible(true);
		weapone_sprite_[0]->setTextureRect(Rect(0, 0, 90, curr_touch_sprite_->weapon0CoolTime));
		weapone_sprite_[1]->setTextureRect(Rect(0, 0, 90, curr_touch_sprite_->weapon1CoolTime));
	}else
	{
		weapone_sprite_[0]->setVisible(false);
		weapone_sprite_[1]->setVisible(false);
	}
	if(player_hkmj_->coolTime0 >0)
	{
		player_hkmj_->coolTime0--;
		ship_cool_sprite_[0]->setTextureRect(Rect(0, 0, 90, player_hkmj_->coolTime0));
	}
	if(player_hkmj_->coolTime1 >0)
	{
		player_hkmj_->coolTime1--;
		ship_cool_sprite_[1]->setTextureRect(Rect(0, 0, 90, player_hkmj_->coolTime1));
	}
	if(player_hkmj_->coolTime2 >0)
	{
		player_hkmj_->coolTime2--;
		ship_cool_sprite_[2]->setTextureRect(Rect(0, 0, 90, player_hkmj_->coolTime2));
	}
	if(player_hkmj_->coolTime3 >0)
	{
		player_hkmj_->coolTime3--;
		ship_cool_sprite_[3]->setTextureRect(Rect(0, 0, 90, player_hkmj_->coolTime3));
	}
	if(player_hkmj_->coolTime4 >0)
	{
		player_hkmj_->coolTime4--;
		ship_cool_sprite_[4]->setTextureRect(Rect(0, 0, 90, player_hkmj_->coolTime4));
	}
}
//添加导弹
bool GameLayer::AddWeapon(int select_index,Point start_point,Point final_point,WarShipObject* ship,int launch_form)
{
	//判断选择的为导弹1还是2
	if(select_index == 0)
	{
		if(ship->weapon0CoolTime != 0)
		{
			return false;
		}
	}else if(select_index == 1)
	{
		if(ship->weapon1CoolTime != 0)
		{
			return false;
		}
	}
	if(ChooseLevelLayer::froMusicFlag == true)
	{
		CocosDenshion::SimpleAudioEngine::getInstance()
									->playEffect("mysound/sound_sfx_missile.mp3");
	}

	//创建导弹，并将其添加到向量中
	Point pos_ship = start_point;
	Weapon* weapon_sprite = Weapon::create(select_index,pos_ship,launch_form);
	weapon_sprite->weaponFromShip = ship;
	weapon_sprite->power = ship->attackPower;

	switch(select_index)
	{
	case 0:		weapon_sprite->setScale(0.03);	break;
	case 1:   	weapon_sprite->setScale(0.4);	break;
	}

	weapon_sprite->setPositionZ(50);
	map_->addChild(weapon_sprite,WQTAG);
	all_weapon_vector_->push_back(weapon_sprite); //===添加到导弹向量中

	weapon_sprite->startPoint = pos_ship;
	weapon_sprite->targetPoint = final_point;
	weapon_sprite->currX = weapon_sprite->startPoint.x;
	weapon_sprite->currY = weapon_sprite->startPoint.y;
	float midx = 0;
	float midy = 0;
	//判断发射方向
	if(weapon_sprite->startPoint.x - weapon_sprite->targetPoint.x <0)
	{//朝右
		midx = fabs(weapon_sprite->targetPoint.x -weapon_sprite->startPoint.x)/2+weapon_sprite->startPoint.x;
		weapon_sprite->forwardLeft = false;
	}else
	{//朝左
		midx = fabs(weapon_sprite->startPoint.x -weapon_sprite->targetPoint.x)/2+weapon_sprite->targetPoint.x;
		weapon_sprite->forwardLeft = true;
	}
	midy = ZMAX;
	//记录当前导弹抛物线的中间点
	weapon_sprite->midx = midx;
	weapon_sprite->midy = midy;

	float a =0;
	// y = a*(x-x1)*(x-x2);

	//x-z面的抛物线
	//求a
	a = midy/((midx-weapon_sprite->startPoint.x)*(midx-weapon_sprite->targetPoint.x));
	weapon_sprite->a = a;
	weapon_sprite->degreeB = -a*(weapon_sprite->startPoint.x+weapon_sprite->targetPoint.x);

	//y =  k*x + b----------x-y面
	//斜率
	float k = (weapon_sprite->startPoint.y-weapon_sprite->targetPoint.y)/(weapon_sprite->startPoint.x-weapon_sprite->targetPoint.x);
	//b
	float b = weapon_sprite->startPoint.y-k*weapon_sprite->startPoint.x;

	//记录在到导弹相关信息
	weapon_sprite ->k = k;
	weapon_sprite ->b = b;

	//求旋转角度
	float dir = (float)atan(weapon_sprite->k);
	if(weapon_sprite->forwardLeft == true)
	{
		weapon_sprite->rotationZ = 180 + dir/PI*180 ;
	}else
	{
		weapon_sprite->rotationZ = dir/PI*180 ;
	}
	weapon_sprite->rotationZ = -weapon_sprite->rotationZ;
	if(weapon_sprite->rotationZ < 0 &&weapon_sprite->rotationZ > -90)
	{
		weapon_sprite->rotationZ = 270 + 90 + weapon_sprite->rotationZ ;
	}else if(weapon_sprite->rotationZ < -90 &&weapon_sprite->rotationZ > -180)
	{
		weapon_sprite->rotationZ = 180 + weapon_sprite->rotationZ ;
	}else if(weapon_sprite->rotationZ < -180 &&weapon_sprite->rotationZ > -270)
	{
		weapon_sprite->rotationZ = 180 + weapon_sprite->rotationZ ;
	}
	//设置当前对象的武器冷却时间
	if(select_index == 0)
	{
		ship->weapon0CoolTime = 104;
	}else if(select_index == 1)
	{
		ship->weapon1CoolTime = 104;
	}
	return true;
}

float GameLayer::CalculatePointZ(float x, Weapon *weapon)
{
	float result = 0;
	result = weapon->a*(x-weapon->startPoint.x)*(x-weapon->targetPoint.x);
	return result;
}
float GameLayer::CalculatePointY(float x, Weapon *weapon)
{
	float result = 0;
	result = weapon->k*x+weapon->b;
	return result;
}
float GameLayer::CalculateDegree(float x, Weapon *weapon)
{
	float result = 0;
	//求微分  y= a*x*x + b*x +c
//	result = 2*weaponSp->a*x+weaponSp->degreeB;

	float k = (weapon->midy - weapon->currZ)/(weapon->midx - weapon->currX);
	result = k*2;
	return result;
}
float GameLayer::CalculateTwoPointDistance(Point start_point, Point end_point)
{
	float result = 0;
	float dx = start_point.x - end_point.x;
	float dy = start_point.y - end_point.y;
	result = sqrt(dx*dx+dy*dy);
	return result;
}
void GameLayer::WeaponeStateUpdate()
{
	std::vector<Weapon*>::iterator weapon ;
	for(weapon = all_weapon_vector_->begin();weapon != all_weapon_vector_->end();)
	{
		(*weapon)->count ++;
		if((*weapon)->count % 20 == 0)
		{
			(*weapon)->step = (*weapon)->step +1;
		}
		if((*weapon)->forwardLeft == false)
		{
			(*weapon)->currX = (*weapon)->currX+(*weapon)->step;
		}else if((*weapon)->forwardLeft == true)
		{
			(*weapon)->currX = (*weapon)->currX-(*weapon)->step;
		}

		float z = CalculatePointZ((*weapon)->currX,(*weapon));
		float y = CalculatePointY((*weapon)->currX,(*weapon));
		float degree = CalculateDegree((*weapon)->currX,(*weapon));
		(*weapon)->currZ = z;
		(*weapon)->setPositionX((*weapon)->currX);
		(*weapon)->setPositionY(y);
		(*weapon)->setPositionZ(z);

		float direction=(float)atan(degree);
		float angleY =0;

		if((*weapon)->forwardLeft == true)
		{
			angleY = 90 - direction/PI*180 ;
		}else
		{
			angleY = 270 - direction/PI*180;
		}
		(*weapon)->setRotation3D(Vertex3F(-90,angleY,(*weapon)->rotationZ));
		if((*weapon)->forwardLeft == true&&(*weapon)->currX <= (*weapon)->targetPoint.x)
		{
			delete_weapon_vector_->push_back((*weapon));
			weapon = all_weapon_vector_->erase(weapon);
			continue;
		}else if((*weapon)->forwardLeft == false&&(*weapon)->currX >= (*weapon)->targetPoint.x)
		{
			delete_weapon_vector_->push_back((*weapon));
			weapon = all_weapon_vector_->erase(weapon);
			continue;
		}
		weapon++;
	}
	std::vector<Weapon*>::iterator delete_weapon ;
	for(delete_weapon = delete_weapon_vector_->begin();delete_weapon != delete_weapon_vector_->end();)
	{
		DestroyEnemyShip((*delete_weapon));
        RemoveShipPlayEffect((*delete_weapon)->getPosition());
		map_->removeChild((*delete_weapon),true);
        delete_weapon = delete_weapon_vector_->erase(delete_weapon);
	}
}
void GameLayer::DestroyEnemyShip(Weapon *weapon)
{
	Point weapon_position = weapon->getPosition();
	std::vector<WarShipObject*>::iterator ship ;
	for(ship = all_ship_vector_->begin();ship != all_ship_vector_->end();)
	{
		Point ship_position = (*ship)->getPosition();
		float dx = ship_position.x - weapon_position.x;
		float dy = ship_position.y - weapon_position.y;
		float distance = sqrt(dx*dx+dy*dy);
        if(distance < ATTACKRANGE)
        {
            if(((weapon->launchForm == Weapon::kEnemy)&&((*ship)->shipType == WarShipObject::kPlayer))
                    ||((weapon->launchForm == Weapon::kPlayer)&&((*ship)->shipType == WarShipObject::kEnemy)))
            {

				(*ship)->lifeValue = (*ship)->lifeValue - weapon->power;

                //加等级
				if((*ship)->lifeValue <= 0 && weapon->weaponFromShip->shipType == WarShipObject::kPlayer)
				{
					weapon->weaponFromShip->junxian += 1;
					if(weapon->weaponFromShip->junxian % 4 == 0)
					{
						if(weapon->weaponFromShip->junxian/4 <= 3)
						{
							int jxdj = weapon->weaponFromShip->junxian/4;
							weapon->weaponFromShip->removeChildByTag(JUNXIANSPTAG);//error 可能没有
							std::string path = StringUtils::format("gamePic/junx%d.png",jxdj);
							Sprite* junxian_sprite = Sprite::create(path.c_str());
							Size size = weapon->weaponFromShip->getContentSize();
                            junxian_sprite->setPosition(size.width,size.height);
							weapon->weaponFromShip->addChild(junxian_sprite,10,JUNXIANSPTAG);
						}
					}
				}
			}
		}
		ship++;
	}
}

//若点击的地方为小岛，则显示X
void GameLayer::AddWrongPrompt(Point location_map)
{
	if(ChooseLevelLayer::froMusicFlag == true)
	{
		CocosDenshion::SimpleAudioEngine::getInstance()
										->playEffect("mysound/sound_sfx_error.mp3");
	}

	Sprite* wrong_sprite = Sprite::create("gamePic/wrong.png");
    wrong_sprite->setPosition(location_map);
	map_->addChild(wrong_sprite,10);
    wrong_sprite->runAction(
			Sequence::create(
					Repeat::create(
							Sequence::create(
									ScaleTo::create(0.5,1.5),
									ScaleTo::create(0.5,0.5),
									NULL
							),
                            2
                    ),
					Spawn::create(
							ScaleTo::create(0.5,0.1),
							FadeOut::create(0.5),
							NULL
					),
					RemoveSelf::create(true),
					NULL));
}
void GameLayer::PlayerHKMJCoolTimeUpdate()
{
	if(player_hkmj_->coolTimeBSJ != 100)
	{
        player_hkmj_->coolTimeBSJ = player_hkmj_->coolTimeBSJ +1;
	}else
	{
        player_hkmj_->coolTimeBSJ = 100;
        cool_spb_ = false;
        player_hkmj_->hkmjSprite->removeChild(cool_sp_[0],true);
	}
	if(cool_spb_ == true)
	{
        cool_sp_[0]->setTextureRect(Rect(0, 0, player_hkmj_->coolTimeBSJ/2, 37));
	}

	if(player_hkmj_->coolTimePlane != 100)
	{
        player_hkmj_->coolTimePlane = player_hkmj_->coolTimePlane +5;
	}else
	{
        player_hkmj_->coolTimePlane = 100;
        cool_spp_ = false;
        player_hkmj_->hkmjSprite->removeChild(cool_sp_[1],true);
	}
	if(cool_spp_ == true)
	{
        cool_sp_[1]->setTextureRect(Rect(0, 0, (player_hkmj_->coolTimePlane*0.9)/2, 37));
	}
}
void GameLayer::DestroyAllEnemyShip()
{
    player_hkmj_->coolTimeBSJ = 0;
	Sprite* sp = Sprite::create();
	sp->setPosition(Point(1136/2,768/2));
	map_->addChild(sp,1000);

	sp->runAction(
			Sequence::create(
					animate_action_[0],
					CallFunc::create(CC_CALLBACK_0(GameLayer::ExpansionRing,this)),
					RemoveSelf::create(true),
					NULL)
    );

    player_hkmj_->hkmjSprite->removeChildByTag(HKMJITEMSPTAG);
    player_hkmj_->hkmjSprite->removeChildByTag(PLANEITEMSPTAG);

	Texture2D* texture = Director::getInstance()->getTextureCache()->addImage("gamePic/yulei.png");
	cool_sp_[0] = Sprite::createWithTexture(texture);
    cool_sp_[0]->setTextureRect(Rect(0, 0, player_hkmj_->coolTimeBSJ/2, 37));
    cool_sp_[0]->setPosition(40,-30);
    player_hkmj_->hkmjSprite->addChild(cool_sp_[0],20);

    cool_spb_ = true;				//表示正在冷却中
    player_hkmj_->openMenu = false;	//表示不能发大招
}
void GameLayer::PlaneFlyAtShip()
{
    player_hkmj_->coolTimePlane = 0;
    plane_flag_ = true;
	for(int i =0;i<2;i++)
	{
        plane_3d_[i] = Sprite3D::create("startPic/zdj3D.obj");
        plane_3d_[i]->setTexture("startPic/HE162R.jpg");
        plane_3d_[i]->setPosition(Point(1240,100+i*500));
        plane_3d_[i]->setScale(0.007f);
        plane_3d_[i]->setRotation3D(Vertex3F(90,0,90));
		map_->addChild(plane_3d_[i],30);
	}
	if(ChooseLevelLayer::froMusicFlag == true)
	{
		CocosDenshion::SimpleAudioEngine::getInstance()
							->playEffect("mysound/flyby2.wav");
	}
    player_hkmj_->hkmjSprite->removeChildByTag(PLANEITEMSPTAG);//error
    player_hkmj_->hkmjSprite->removeChildByTag(HKMJITEMSPTAG);//error
    player_hkmj_->openMenu = false;
    cool_spp_ = true;

	Texture2D* texture = Director::getInstance()->getTextureCache()->addImage("gamePic/planeMenu.png");
	cool_sp_[1] = Sprite::createWithTexture(texture);
    cool_sp_[1]->setTextureRect(Rect(0, 0, (player_hkmj_->coolTimePlane*0.9)/2, 47));
    cool_sp_[1]->setPosition(Point(40,-80));
    player_hkmj_->hkmjSprite->addChild(cool_sp_[1],20);
}
void GameLayer::PlaneUpdate()
{
    if (!plane_flag_) return;

    switch(plane_step_)
    {
        case 1:
            plane_3d_[0]->setPosition(plane_3d_[0]->getPosition().x-4,plane_3d_[0]->getPosition().y);
            plane_3d_[1]->setPosition(plane_3d_[1]->getPosition().x-4,plane_3d_[1]->getPosition().y);
            if(plane_3d_[0]->getPosition().x<=200)
            {
                plane_step_ = 2;
            }
            break;
        case 2:
            plane_3d_[0]->setPosition(plane_3d_[0]->getPosition().x-4,plane_3d_[0]->getPosition().y + 4);
            plane_3d_[1]->setPosition(plane_3d_[1]->getPosition().x-4,plane_3d_[1]->getPosition().y - 4);
            if(plane_3d_[0]->getPosition().x<=100&&plane_3d_[0]->getRotation3D().z>=180)
            {
                plane_step_ = 3;
            }else
            {
                plane_3d_[0]->setRotation3D(Vertex3F(90,plane_3d_[0]->getRotation3D().y - 2,plane_3d_[0]->getRotation3D().z + 2 ));
                plane_3d_[1]->setRotation3D(Vertex3F(90,plane_3d_[1]->getRotation3D().y + 2,plane_3d_[1]->getRotation3D().z - 2 ));
            }
            break;
        case 3:
            plane_3d_[0]->setPosition(plane_3d_[0]->getPosition().x+4,plane_3d_[0]->getPosition().y + 4);
            plane_3d_[1]->setPosition(plane_3d_[1]->getPosition().x+4,plane_3d_[1]->getPosition().y - 4);
            if(plane_3d_[0]->getPosition().x>=200&&plane_3d_[0]->getRotation3D().z>=270)
            {
                plane_step_ = 4;
            }else
            {
                plane_3d_[0]->setRotation3D(Vertex3F(90,plane_3d_[0]->getRotation3D().y + 2,plane_3d_[0]->getRotation3D().z + 2 ));
                plane_3d_[1]->setRotation3D(Vertex3F(90,plane_3d_[1]->getRotation3D().y - 2,plane_3d_[1]->getRotation3D().z - 2 ));
            }
            break;
        case 4:
            plane_3d_[0]->setPosition(plane_3d_[0]->getPosition().x+4,plane_3d_[0]->getPosition().y);
            plane_3d_[1]->setPosition(plane_3d_[1]->getPosition().x+4,plane_3d_[1]->getPosition().y);
            break;
    }
    std::vector<WarShipObject*>::iterator ship ;
    for(ship = all_ship_vector_->begin();ship != all_ship_vector_->end();)
    {
        if((*ship)->shipType == WarShipObject::kEnemy)
        {
            Vec2 ship_position = (*ship)->getPosition();
            Vec2 plane0_position = plane_3d_[0]->getPosition();
            Vec2 plane1_position = plane_3d_[1]->getPosition();

            float dis0 = (plane0_position - ship_position).getLength();
            float dis1 = (plane1_position - ship_position).getLength();

            if(dis0 < R/2||dis1 < R/2)
            {
                (*ship)->lifeValue = 0;
            }
        }
        ship++;
    }
    if(plane_3d_[0]->getPosition().x>1500&&plane_3d_[1]->getPosition().x>1500)
    {
        plane_flag_ = false;
        plane_step_ = 1;
        map_->removeChild(plane_3d_[0],true);
        map_->removeChild(plane_3d_[1],true);
    }

}
bool GameLayer::onTouchBegan(Touch* touch, Event* event)
{
    if (is_game_over_ == true)  return false;

	TMXTiledMap* target_map = static_cast<TMXTiledMap*>(	//获取当前触摸对象，并转化为精灵类型
								event->getCurrentTarget());
	first_touch_point_ = touch->getLocation();
	Vec2 location_map = target_map->convertToNodeSpace(touch->getLocation());//获取当前坐标

	if(fabs(location_map.x-1050)<= 25&&fabs(location_map.y-270) <= 25)
	{
        PlaySound();
		if(player_hkmj_->openMenu == false)
		{
            //显示导弹
			if(player_hkmj_->coolTimeBSJ == 100)
			{
                player_hkmj_->openMenu = true;
				MenuItemImage* hkmj_item = MenuItemImage::create(
						"gamePic/yulei.png",
						"gamePic/yuleip.png",
						CC_CALLBACK_0(GameLayer::DestroyAllEnemyShip, this));
				hkmj_item->setPosition(Point(40,-10));

				//创建一个菜单对象
				Menu* menu = Menu::create(hkmj_item,NULL);
				//设置其位置
				menu->setPosition(Point::ZERO);
				//将其添加到布景中
                player_hkmj_->hkmjSprite->addChild(menu,20,HKMJITEMSPTAG);

				hkmj_item->runAction(MoveBy::create(0.3,Point(0,-20)));
			}
            //显示飞机
			if(player_hkmj_->coolTimePlane == 100)
			{
				player_hkmj_->openMenu = true;
				MenuItemImage* plane_item = MenuItemImage::create(
						"gamePic/planeMenu.png",
						"gamePic/planeMenup.png",
						CC_CALLBACK_0(GameLayer::PlaneFlyAtShip, this));
				plane_item->setPosition(Point(40,-10));

				//创建一个菜单对象
				Menu* menu = Menu::create(plane_item,NULL);
				//设置其位置
				menu->setPosition(Point::ZERO);
				//将其添加到布景中
                player_hkmj_->hkmjSprite->addChild(menu,20,PLANEITEMSPTAG);

				plane_item->runAction(MoveBy::create(0.3,Point(0,-80)));
			}
		}else
		{
            player_hkmj_->hkmjSprite->removeChildByTag(PLANEITEMSPTAG);
            player_hkmj_->hkmjSprite->removeChildByTag(HKMJITEMSPTAG);
            player_hkmj_->openMenu = false;
		}
		return false;
	}

	Size map_size = map_->getContentSize();
	//添加船只
	if(select_item_ == true&& (select_index_ >=0&&select_index_ <=4))
	{//添加我方船只
        //地图外面
		if(location_map.y<40||location_map.x>map_size.width-40||location_map.y>map_size.height-40||location_map.x<40)
		{
			AddWrongPrompt(location_map);
			return false;
		}
		//查看船只是否在冷却中
		switch(select_index_)
		{
		case 0:
			if(player_hkmj_->coolTime0 != 0)
			{
				select_index_ = -1;
				selected_sp_->setVisible(false);
				select_item_ = false;
				return false;
			}
			break;
		case 1:
			if(player_hkmj_->coolTime1 != 0)
			{
				select_index_ = -1;
				selected_sp_->setVisible(false);
				select_item_ = false;
				return false;
			}
			break;
		case 2:
			if(player_hkmj_->coolTime2 != 0)
			{
                select_index_ = -1;
                selected_sp_->setVisible(false);
                select_item_ = false;
				return false;
			}
			break;
		case 3:
			if(player_hkmj_->coolTime3 != 0)
			{
                select_index_ = -1;
                selected_sp_->setVisible(false);
                select_item_ = false;
				return false;
			}
			break;
		case 4:
			if(player_hkmj_->coolTime4 != 0)
			{
                select_index_ = -1;
                selected_sp_->setVisible(false);
                select_item_ = false;
                return false;
			}
			break;
		}
        AddWarShip(target_map->convertToNodeSpace(touch->getLocation()),select_index_);
		select_index_ = -1;
		selected_sp_->setVisible(false);
		select_item_ = false;
	}else if(select_index_ == 5||select_index_ == 6 )
	{//发射炮弹
        AddWeapon(
                select_index_-5,
                last_touch_sprite_->getPosition(),
                target_map->convertToNodeSpace(touch->getLocation()),
                last_touch_sprite_,
                Weapon::kPlayer
        );

        select_index_ = -2;
		selected_sp_->setVisible(false);
		select_item_ = false;
	}
	//点击船，让其移动
	if(select_ship_ == true && select_index_ == -1)
	{
		Vec2 temp_position ;
		temp_position = TouchPointToRowCol(target_map->convertToNodeSpace(touch->getLocation()));
		//停止当前正在进行的动作
		last_touch_sprite_->stopAllActions();
		//获取当前对象的位置
		Vec2 ship_current_position = last_touch_sprite_->getPosition();
		//将X,Y坐标转换为格子的行列号
		Vec2 ship_row_col = TouchPointToRowCol(ship_current_position);
		last_touch_sprite_->targetRow = (int)(ship_row_col.y - 1);
		last_touch_sprite_->targetCol = (int)(ship_row_col.x - transform_.x);//error
		//判断点击的地方是否能走------是否为地图中的岛屿
		if(map_data_[(int)temp_position.x][(int)temp_position.y] == 1)
		{
            AddWrongPrompt(location_map);
			return false;
		}else
		{
			Sprite* mz_sprite = Sprite::create("gamePic/MZ.png");
			mz_sprite->setPosition(target_map->convertToNodeSpace(touch->getLocation()));
			map_->addChild(mz_sprite,10);

			mz_sprite->runAction(
					Sequence::create(
							ScaleTo::create(0.5,1.2),
							ScaleTo::create(0.5,0.9),
							RemoveSelf::create(true),
							NULL)
			);
			if(ChooseLevelLayer::froMusicFlag == true)
			{
				CocosDenshion::SimpleAudioEngine::getInstance()
												->playEffect("mysound/sound_sfx_destination.mp3");
			}
		}
		//目标点
		//设置目标点
		target_[0] = (int)temp_position.y;
		target_[1] = (int)temp_position.x;
		//寻找路径
        SearchPath(last_touch_sprite_);
		//设置当前对象的目标格子行列号
		last_touch_sprite_->targetRow = temp_position.y;
		last_touch_sprite_->targetCol = temp_position.x;
		//设置其选中状态---------0为不选中
		last_touch_sprite_->state = WarShipObject::kNotSelected;
		//显示
        ShowWarShipItem(last_touch_sprite_);
		//删除船只身上挂的精灵
        last_touch_sprite_->removeChildByTag(DWPTAG);
        last_touch_sprite_->removeChildByTag(DWSPRITETAG);
        last_touch_sprite_->removeChildByTag(COMPASSSPRITE);
        last_touch_sprite_->removeChildByTag(BQSPRITETAG);
        last_touch_sprite_ = NULL;
		//设置选中状态为未选中船只
		select_ship_ = false;//error
		//显示船只冷却进度
		for(int k = 0;k<5;k++)
		{
            ship_cool_sprite_[k]->setVisible(true);
		}
		curr_touch_sprite_ = NULL;
		return false;
	}
	//判断是否点击地图
	Rect rect_map = Rect(0,0,map_size.width, map_size.height);//创建一个矩形对象，其大小与精灵相同
	if(rect_map.containsPoint(location_map))
	{
		return true;
	}else
	{
		return false;
	}
}
void GameLayer::onTouchMoved(Touch* touch, Event* event)
{
	//移动地图
	float dx = touch->getLocation().x-first_touch_point_.x;
	float x = map_point_.x+dx;

	float dy = touch->getLocation().y-first_touch_point_.y;
	float y = map_point_.y+dy;

	if(x>=960&&x<=1136)
	{
		map_->setPosition(x,map_->getPosition().y);
	}
	if(y <=0&&y >=-228)
	{
		map_->setPosition(map_->getPosition().x,y);
	}
}
void GameLayer::onTouchEnded(Touch* touch, Event* event)
{
	map_point_ = map_->getPosition();
}
//添加一波敌船
void GameLayer::AddEnemyShip()
{
	//产生一个1-8的随机数
	srand((unsigned)time(NULL));
	for(int i=0;i<10;i++)
	{
		int index = rand()%(wave_num_*2)+1;
		int y = 20+(rand()%500+1);
		y = y/16*16;
		WarShipObject* enemy_ship = WarShipObject::create(
				StringUtils::format("enemyShip/E%d.png",index),
				Point(16,y),
				index,WarShipObject::kEnemy);

        enemy_ship->isMoved = true;
		//设置敌船的血量
        enemy_ship->lifeValue = wave_num_*100+index*50;
		map_->addChild(enemy_ship,10+i);
		//将船只放在存所有船只向量中
		all_ship_vector_->push_back(enemy_ship);
        RandomSort();
		for(int i = 0;i<8;i++)
		{
			for(int j = 0;j<2;j++)
			{
				enemy_ship->sequence[i][j] = result_sequence_[i][j];
			}
		}
		int row;
		int col;
		if(i <5)
		{
			row = 25;
			col = 63+i;
		}else
		{
			row = 35;
			col = 63+i-5;
		}
        enemy_ship->enemyShipTargetRow = row;
        enemy_ship->enemyShipTargetCol = col;

		target_[0] = row;
		target_[1] = col;
        SearchPath(enemy_ship);
        enemy_ship->isMoved = true;
	}
}
void GameLayer::RemoveShipPlayEffect(Point temp_pos)
{
	if(ChooseLevelLayer::froMusicFlag == true)
	{
		CocosDenshion::SimpleAudioEngine::getInstance()
								->playEffect("mysound/sound_sfx_nuclear.mp3");
	}

	Sprite* sp = Sprite::create();
	sp->setPosition(temp_pos);
	map_->addChild(sp ,100);
	sp->runAction(
			Sequence::create(
					animate_action_[1]->clone(),
					RemoveSelf::create(true),
					NULL
			));
}
void GameLayer::RemoveShipUpdate()
{
	std::vector<WarShipObject*>::iterator ship ;
	for(ship =all_ship_vector_->begin();ship!= all_ship_vector_->end();)
	{
		if((*ship)->lifeValue <= 0)
		{
			if((*ship)->shipType == WarShipObject::kEnemy)
			{
				if(ChooseLevelLayer::froMusicFlag == true)
				{
					CocosDenshion::SimpleAudioEngine::getInstance()
												->playEffect("mysound/sound_sfx_break.mp3");
				}

				enemy_ship_count_ --;
				sds_num_ ++;
				std::string sds_string = StringUtils::format("%d",sds_num_);
				sds_label_->setString(sds_string);

				curr_level_sds_ ++;

				my_gold_ = my_gold_ + ((*ship)->shipNum+1)*30;
				std::string my_gold_string = StringUtils::format("%d",my_gold_);
				money_label_->setString(my_gold_string);

				if(enemy_ship_count_ == 0)
				{

					if(wave_num_ <= 5)
					{
                        GameStartDaoJiShi();
					}else
					{
                        game_result_ = true;
                        is_game_over_ = true;
                        GameOver();
					}
				}
			}
            delete_ship_vector_->push_back(*ship);
            ship = all_ship_vector_->erase(ship);;
			continue;
		}
		ship ++;
	}
	std::vector<WarShipObject*>::iterator delete_ship ;
	for(delete_ship =delete_ship_vector_->begin();delete_ship!= delete_ship_vector_->end();)
	{
        RemoveShipPlayEffect((*delete_ship)->getPosition());
		map_->removeChild((*delete_ship),true);
        delete_ship = delete_ship_vector_->erase(delete_ship);
	}
}
//初始化航母
void GameLayer::InitHKMJ()
{
    player_hkmj_ = new HKMJObject(1050,270);
	map_->addChild(player_hkmj_->hkmjSprite,30);

	ParticleSystemQuad* particle_one = ParticleSystemQuad::create("lzxt/tji.plist");//从文件中加载粒子系统
    particle_one->setPositionType(ParticleSystem::PositionType::RELATIVE);
//    particle_one->retain();								//error
    particle_one->setBlendAdditive(true);				//设置混合方式为增加
    particle_one->setScale(0.5);
    particle_one->setPosition(Point(37,35));
	player_hkmj_->hkmjSprite->addChild(particle_one, 3);		//向布景层中的精灵添加粒子系统
    particle_one->runAction(
			RepeatForever::create(
					RotateBy::create(0.2,-30)
            )
    );
	ParticleSystemQuad* particle_two = ParticleSystemQuad::create("lzxt/tji.plist");//从文件中加载粒子系统
    particle_two->setPositionType(ParticleSystem::PositionType::RELATIVE);
//    particle_two->retain();								//error
    particle_two->setBlendAdditive(true);				//设置混合方式为增加
    particle_two->setScale(0.5);
    particle_two->setRotation(180);
    particle_two->setPosition(Point(37,35));
	player_hkmj_->hkmjSprite->addChild(particle_two, 3);		//向布景层中的精灵添加粒子系统
    particle_two->runAction(
            RepeatForever::create(
                    RotateBy::create(0.2,-30)
			)
    );

	int temp_i = 0;
	for(int i =0;i<20;i++)
	{
		if(i%2==0)
		{
			life_bar_sp_[i] = Sprite::create("gamePic/lifeBarY.png");
            life_bar_sp_[i]->setPosition(0+temp_i*8,100);
			player_hkmj_->hkmjSprite->addChild(life_bar_sp_[i],0);
		}else if(i%2==1)
		{
            life_bar_sp_[i] = Sprite::create("gamePic/lifeBarG.png");
            life_bar_sp_[i]->setPosition(0+temp_i*8,100);
            player_hkmj_->hkmjSprite->addChild(life_bar_sp_[i],1);
            temp_i ++;
		}
	}
}
//初始化军舰选项
void GameLayer::InitJJItem()
{
    jj_bg_sp_[0] = MenuItemImage::create(
			"gamePic/jjbg.png",
			"gamePic/jjbg.png",
			CC_CALLBACK_0(GameLayer::JJBgSpCallback0, this)
	);
    jj_bg_sp_[0]->setPosition(55+0*100,50);

    jj_bg_sp_[1] = MenuItemImage::create(
			"gamePic/jjbg.png",
			"gamePic/jjbg.png",
			CC_CALLBACK_0(GameLayer::JJBgSpCallback1, this)
	);
    jj_bg_sp_[1]->setPosition(55+1*100,50);

    jj_bg_sp_[2] = MenuItemImage::create(
			"gamePic/jjbg.png",
			"gamePic/jjbg.png",
			CC_CALLBACK_0(GameLayer::JJBgSpCallback2, this)
	);
    jj_bg_sp_[2]->setPosition(55+2*100,50);

    jj_bg_sp_[3] = MenuItemImage::create(
			"gamePic/jjbg.png",
			"gamePic/jjbg.png",
			CC_CALLBACK_0(GameLayer::JJBgSpCallback3, this)
	);
    jj_bg_sp_[3]->setPosition(55+3*100,50);

    jj_bg_sp_[4] = MenuItemImage::create(
			"gamePic/jjbg.png",
			"gamePic/jjbg.png",
			CC_CALLBACK_0(GameLayer::JJBgSpCallback4, this)
	);
    jj_bg_sp_[4]->setPosition(55+4*100,50);

	ship_weapon_ [0] = MenuItemImage::create(
				"gamePic/jjbg.png",
				"gamePic/jjbg.png",
				CC_CALLBACK_0(GameLayer::ShipWeaponCallback0, this)
		);
    ship_weapon_ [0]->setPosition(55+0*100,50);
    ship_weapon_ [0]->setVisible(false);

    ship_weapon_ [1] = MenuItemImage::create(
				"gamePic/jjbg.png",
				"gamePic/jjbg.png",
				CC_CALLBACK_0(GameLayer::ShipWeaponCallback1, this)
		);
    ship_weapon_ [1]->setPosition(55+1*100,50);
    ship_weapon_ [1]->setVisible(false);

    ship_weapon_ [2] = MenuItemImage::create(
				"gamePic/cancleg.png",
				"gamePic/cancleg.png",
				CC_CALLBACK_0(GameLayer::ShipWeaponCallback2, this)
		);
    ship_weapon_ [2]->setPosition(85+8*100,65);
    ship_weapon_ [2]->setVisible(false);

	//创建一个菜单对象
	Menu* menu = Menu::create(
			jj_bg_sp_[0],jj_bg_sp_[1],jj_bg_sp_[2],
            jj_bg_sp_[3],jj_bg_sp_[4],ship_weapon_ [0],
            ship_weapon_ [1],ship_weapon_ [2],NULL);
	//设置其位置
	menu->setPosition(Point::ZERO);
	//将其添加到布景中
	this->addChild(menu, 2);

	//初始化武器
	for(int i =0 ;i<2;i++)
	{
		Sprite* temp_sp = Sprite::create(StringUtils::format("gamePic/wq%d.png",i));
        temp_sp->setPosition(45,52);
        ship_weapon_[i]->addChild(temp_sp,2);
	}

	for(int i =0 ;i<5;i++)
	{
		Sprite* jun_jian_sp = Sprite::create(StringUtils::format("gamePic/jj%d.png",i));
        jun_jian_sp->setPosition(45,52);
		jj_bg_sp_[i]->addChild(jun_jian_sp,2);
		Sprite* jin_bi_sp = Sprite::create("gamePic/jinbi.png");
        jin_bi_sp->setPosition(20,15);
        jj_bg_sp_[i]->addChild(jin_bi_sp,2);
		Label* price_label = Label::createWithTTF(
				StringUtils::format("%d",(i+1)*400),"fonts/FZKATJW.ttf",20
		);
        price_label->setPosition(60,15);
        jj_bg_sp_[i]->addChild(price_label,2);
	}

	Label* jj_name0 = Label::createWithTTF(
			"\u767b\u9646\u8247","fonts/FZKATJW.ttf",15
	);
    jj_name0->setAnchorPoint(Point(0,0.5));
    jj_name0->setPosition(20,95);
    jj_bg_sp_[0]->addChild(jj_name0,2);

	Label* jj_name1 = Label::createWithTTF(
			"\u96f7\u9706\u8230","fonts/FZKATJW.ttf",15
	);
    jj_name1->setAnchorPoint(Point(0,0.5));
    jj_name1->setPosition(20,95);
    jj_bg_sp_[1]->addChild(jj_name1,2);

	Label* jj_name2 = Label::createWithTTF(
			"\u6218\u795e\u8230","fonts/FZKATJW.ttf",15
	);
    jj_name2->setAnchorPoint(Point(0,0.5));
    jj_name2->setPosition(20,95);
    jj_bg_sp_[2]->addChild(jj_name2,2);

	Label* jj_name3 = Label::createWithTTF(
			"\u731b\u79bd\u8230","fonts/FZKATJW.ttf",15
	);
    jj_name3->setAnchorPoint(Point(0,0.5));
    jj_name3->setPosition(20,95);
    jj_bg_sp_[3]->addChild(jj_name3,2);

	Label* jj_name4 = Label::createWithTTF(
			"\u96f7\u795e\u822a\u6bcd","fonts/FZKATJW.ttf",15
	);
    jj_name4->setAnchorPoint(Point(0,0.5));
    jj_name4->setPosition(20,95);
    jj_bg_sp_[4]->addChild(jj_name4,12);

    selected_sp_ = Sprite::create("gamePic/selected.png");
    selected_sp_->setVisible(false);
    this->addChild(selected_sp_,2);
}
//武器0的回调方法
void GameLayer::ShipWeaponCallback0()
{
    PlaySound();

	if(select_item_ == true&&select_index_ == 5)
	{
		last_touch_sprite_->removeChildByTag(DWPTAG);
        AddSaveSprite(last_touch_sprite_);
        select_item_ = false;
        select_index_ = -1;
		selected_sp_->setVisible(false);
	}else
	{
        selected_sp_->setVisible(true);
        selected_sp_->setPosition(ship_weapon_[0]->getPosition());
        select_index_ = 5;
		last_touch_sprite_->removeChildByTag(DWPTAG);
        last_touch_sprite_->removeChildByTag(DWSPRITETAG);
        AddAimSprite();
        select_item_ = true;
	}
}
//武器1的回调方法
void GameLayer::ShipWeaponCallback1()
{
    PlaySound();

	if(select_item_ == true&&select_index_ == 6)
	{
		last_touch_sprite_->removeChildByTag(DWPTAG);
		AddSaveSprite(last_touch_sprite_);
        select_item_ = false;
        select_index_ = -1;
		selected_sp_->setVisible(false);
	}else
	{
        selected_sp_->setVisible(true);
        selected_sp_->setPosition(ship_weapon_[1]->getPosition());
		select_index_ = 6;
		last_touch_sprite_->removeChildByTag(DWPTAG);
        last_touch_sprite_->removeChildByTag(DWSPRITETAG);
        AddAimSprite();
		select_item_ = true;
	}
}
//船中心的瞄准点
void GameLayer::AddAimSprite()
{
	Sprite* dwp = Sprite::create("gamePic/dwp.png");
	last_touch_sprite_->removeChildByTag(DWSPRITETAG);
	Size size = last_touch_sprite_->getContentSize();
	dwp->setPosition(size.width/2,size.height/2);
	last_touch_sprite_->addChild(dwp,-1,DWPTAG);
	dwp->runAction(RepeatForever::create(RotateBy::create(4,360)));
}
//船的小圈
void GameLayer::AddSaveSprite(WarShipObject *ship)
{
	Size size = ship->getContentSize();
	Sprite* dw = Sprite::create("gamePic/dw.png");
	dw->setPosition(size.width/2,size.height/2);
	ship->addChild(dw,-1,DWSPRITETAG);
	dw->runAction(RepeatForever::create(RotateBy::create(4,360)));
}
//取消菜单项的回调方法
void GameLayer::ShipWeaponCallback2()
{
    PlaySound();

	last_touch_sprite_->state = WarShipObject::kNotSelected;
    ShowWarShipItem(last_touch_sprite_);
    last_touch_sprite_->removeChildByTag(DWPTAG);
    last_touch_sprite_->removeChildByTag(DWSPRITETAG);
    last_touch_sprite_->removeChildByTag(COMPASSSPRITE);
    last_touch_sprite_->removeChildByTag(BQSPRITETAG);
	select_ship_ = false;
	select_item_ = false;
	selected_sp_->setVisible(false);
	select_index_ = -1;
	curr_touch_sprite_ = NULL;
	last_touch_sprite_ = NULL;
	for(int k = 0;k<5;k++)
	{
		ship_cool_sprite_[k]->setVisible(true);
	}
}
//点击军舰0的回调方法
void GameLayer::JJBgSpCallback0()
{
	PlaySound();

	if(select_item_ == true&&select_index_ == 0)
	{
		select_item_ = false;
        select_index_ = -1;
		selected_sp_->setVisible(false);
	}else
	{
        selected_sp_->setVisible(true);
        selected_sp_->setPosition(jj_bg_sp_[0]->getPosition());
        select_index_ = 0;
		select_item_ = true;
	}
}
//点击军舰1的回调方法
void GameLayer::JJBgSpCallback1()
{
	PlaySound();

	if(select_item_ == true&&select_index_ == 1)
	{
        select_item_ = false;
        select_index_ = -1;
        selected_sp_->setVisible(false);
	}else
	{
		selected_sp_->setVisible(true);
        selected_sp_->setPosition(jj_bg_sp_[1]->getPosition());
        select_index_ = 1;
        select_item_ = true;
	}
}
//点击军舰2的回调方法
void GameLayer::JJBgSpCallback2()
{
	PlaySound();

	if(select_item_ == true&&select_index_ == 2)
	{
        select_item_ = false;
        select_index_ = -1;
        selected_sp_->setVisible(false);
	}else
	{
		selected_sp_->setVisible(true);
        selected_sp_->setPosition(jj_bg_sp_[2]->getPosition());
        select_index_ = 2;
        select_item_ = true;
	}
}
//点击军舰3的回调方法
void GameLayer::JJBgSpCallback3()
{
    PlaySound();

    if(select_item_ == true&&select_index_ == 3)
    {
        select_item_ = false;
        select_index_ = -1;
        selected_sp_->setVisible(false);
    }else
    {
        selected_sp_->setVisible(true);
        selected_sp_->setPosition(jj_bg_sp_[3]->getPosition());
        select_index_ = 3;
        select_item_ = true;
    }
}
//点击军舰4的回调方法
void GameLayer::JJBgSpCallback4()
{
    PlaySound();

    if(select_item_ == true&&select_index_ == 4)
    {
        select_item_ = false;
        select_index_ = -1;
        selected_sp_->setVisible(false);
    }else
    {
        selected_sp_->setVisible(true);
        selected_sp_->setPosition(jj_bg_sp_[4]->getPosition());
        select_index_ = 4;
        select_item_ = true;
    }
}
//聪明的计算子方法
void GameLayer::CalculateNearestSmartIn()
{

	std::vector<WarShipObject*>* enemy_ship_vector =new std::vector<WarShipObject*>();	//所有军舰
	std::vector<WarShipObject*>* player_ship_vector =new std::vector<WarShipObject*>();

	std::vector<WarShipObject*>::iterator ship_temp ;
	for(ship_temp = all_ship_vector_->begin();ship_temp != all_ship_vector_->end();ship_temp++)
	{
		//必须为敌船
		if((*ship_temp)->shipType == 2)
		{
			enemy_ship_vector->push_back((*ship_temp));
		}else
		{
			player_ship_vector->push_back((*ship_temp));
		}
	}
	std::vector<WarShipObject*>::iterator enemy_temp ;
	for(enemy_temp = enemy_ship_vector->begin();enemy_temp != enemy_ship_vector->end();enemy_temp++)
	{
		if((*enemy_temp)->lifeValue <= 0)
		{
			continue;
		}
		int countTT = 0;
		std::vector<WarShipObject*>::iterator player_temp ;
		for(player_temp = player_ship_vector->begin();player_temp != player_ship_vector->end();player_temp++)
		{
			if((*player_temp)->lifeValue <= 0)
			{
				continue;
			}
			float distance = Distance((*player_temp),(*enemy_temp));
			if(distance < R)
			{
				countTT = 1;
				if((*enemy_temp)->isMoved == true)
				{
					(*enemy_temp)->stopAllActions();	//停止敌船运动
					(*enemy_temp)->isMoved = false;	//标志敌船不可动
				}
				Point player_pos = (*player_temp)->getPosition();
				Point enemy_pos = (*enemy_temp)->getPosition();
                //敌人发射导弹
                AddWeapon(0,enemy_pos,player_pos,(*enemy_temp),Weapon::kEnemy);

                //攻击敌人
				if((*player_temp)->shipNum == 3||(*player_temp)->shipNum == 4)
				{
					Sprite* bullet_sprite = Sprite::create("gamePic/weapon2.png");
                    bullet_sprite->setPosition(player_pos);
					map_->addChild(bullet_sprite,15);

					if(ChooseLevelLayer::froMusicFlag == true)
					{
						CocosDenshion::SimpleAudioEngine::getInstance()
											->playEffect("mysound/sound_sfx_explode_general.mp3");
					}
                    bullet_sprite->runAction(
						Sequence::create(
								MoveTo::create(0.1,enemy_pos),
								RemoveSelf::create(true),
								NULL
							)
                    );
                    //生命值减少
                    (*enemy_temp)->lifeValue = (*enemy_temp)->lifeValue - (*player_temp)->attackPower;
				}
				//军衔
				if((*enemy_temp)->lifeValue <= 0)
				{
					if((*enemy_temp)->isDie == false)
					{
						(*player_temp)->junxian ++;
						if((*player_temp)->junxian % 4 == 0)
						{
							if((*player_temp)->junxian/4 <= 3)
							{
                                if(ChooseLevelLayer::froMusicFlag == true)
                                {
                                    std::string musicPath = StringUtils::format("mysound/sound_sfx_star_%d.mp3",(*player_temp)->junxian/4);
                                    CocosDenshion::SimpleAudioEngine::getInstance()
                                                        ->playEffect(musicPath.c_str());
                                }

                                (*player_temp)->removeChildByTag(JUNXIANSPTAG);
                                std::string path = StringUtils::format("gamePic/junx%d.png",(*player_temp)->junxian/4);
                                Sprite* junxianSp = Sprite::create(path.c_str());

								Size size = (*player_temp)->getContentSize();
								junxianSp->setPosition(size.width,size.height);
								(*player_temp)->addChild(junxianSp,10,JUNXIANSPTAG);
							}
							(*player_temp)->attackPower = (*player_temp)->attackPower + 50;
						}
					}
					(*enemy_temp)->isDie = true;
				}
			}
		}
		if(countTT == 0)
		{
			//若不在打击范围，并当前敌法军舰为停止运行状态，则 重新搜索路径，到目标地点
			if((*enemy_temp)->isMoved == false)
			{
				(*enemy_temp)->isMoved = true;
				Point eTempPoint = (*enemy_temp)->getPosition();
				Point RC = TouchPointToRowCol(eTempPoint);
				(*enemy_temp)->initRow = (int)RC.y;
				(*enemy_temp)->initCol = (int)RC.x;

				target_[0] = (*enemy_temp)->enemyShipTargetRow;
				target_[1] = (*enemy_temp)->enemyShipTargetCol;
				SearchPath((*enemy_temp));
			}
		}
	}
}
//计算两个点距离的方法
float GameLayer::Distance(WarShipObject* d1,WarShipObject* d2)
{
	return (d1->getPosition() - d2->getPosition()).getLength();
}
void GameLayer::EnemyShipAtHKMJUpdate()
{
	std::vector<WarShipObject*>::iterator ship;
	for(ship = all_ship_vector_->begin();ship != all_ship_vector_->end();ship++)
	{
		if(is_game_over_ == true)
		{
			return;
		}
		if((*ship)->shipType == WarShipObject::kEnemy)
		{
			Point enemy_pos = (*ship)->getPosition();
			Point hkmj_pos = player_hkmj_->hkmjSprite->getPosition();
			float dis = (enemy_pos - hkmj_pos).getLength();
			if(dis <= R)
			{
				if(is_game_over_ == true)
				{
					return;
				}
                bool b = AddWeapon(0,enemy_pos,hkmj_pos,(*ship),Weapon::kEnemy);
				if(b == true&&is_game_over_ == false)
				{
					(*ship)->runAction(
							Sequence::createWithTwoActions(
									DelayTime::create(1.5f),
									CallFunc::create(CC_CALLBACK_0(GameLayer::ReduceBlood,this))
                            )
					);
				}
			}
		}
	}
}
void GameLayer::RecoveryStateCallback()
{
	auto director = Director::getInstance();
	Scheduler* sched = director->getScheduler();
    _scheduler->schedule(SEL_SCHEDULE(&GameLayer::CalculateNearestSmartIn),this,0.1,false);
    _scheduler->schedule(SEL_SCHEDULE(&GameLayer::WeaponsCoolTimeUpdate),this,0.1,false);
	is_game_over_ = false;
}
void GameLayer::ReduceBlood()
{
	player_hkmj_->lifeValue =player_hkmj_->lifeValue - 1;
	if(player_hkmj_->lifeValue >=0)
	{
		life_bar_sp_[player_hkmj_->lifeValue]->runAction(RemoveSelf::create(true));
	}
	if(player_hkmj_->lifeValue ==10&&player_hkmj_->qhFlag == false)
	{
        player_hkmj_->qhFlag = true;
		//停止冷却时间回调
		this->unschedule(SEL_SCHEDULE(&GameLayer::WeaponsCoolTimeUpdate));
		//停止寻径回调
		this->unschedule(SEL_SCHEDULE(&GameLayer::CalculateNearestSmartIn));
		is_game_over_ = true;
		std::vector<WarShipObject*>::iterator ship ;
		for(ship =all_ship_vector_->begin();ship!= all_ship_vector_->end();ship++)
		{
			//停止所用船只的动作
			(*ship)->stopAllActions();
			(*ship)->isMoved = false;
		}

		Point map_position = map_->getPosition();
		map_->runAction(
				Sequence::create(
						MoveTo::create(1,Point(960,0)),
						DelayTime::create(1.5),
						MoveTo::create(1,map_position),
						CallFunc::create(CC_CALLBACK_0(GameLayer::RecoveryStateCallback,this)),
						NULL
				));

		ParticleSystemQuad* particle = ParticleSystemQuad::create("lzxt/huoyan.plist");//从文件中加载粒子系统
        particle->setPositionType(ParticleSystem::PositionType::RELATIVE);
//        particle->retain();								//error
        particle->setBlendAdditive(true);				//设置混合方式为增加
        particle->setScale(0.4);
        particle->setPosition(48,45);
        player_hkmj_->hkmjSprite->addChild(particle, 3);		//向布景层中的精灵添加粒子系统
	}
	if(player_hkmj_->lifeValue == 0)
	{
		map_->runAction(MoveTo::create(1,Point(960,0)));
		game_result_ = false;
		is_game_over_ = true;
        RemoveShipPlayEffect(player_hkmj_->hkmjSprite->getPosition());
		map_->removeChild(player_hkmj_->hkmjSprite);
        GameOver();
	}
}
void GameLayer::GameOver()
{
	this->unschedule(SEL_SCHEDULE(&GameLayer::PlayerHKMJCoolTimeUpdate));
	pause_menu_->setEnabled(false);
	UserDefault::getInstance()->setIntegerForKey(Constant::SHADISHU.c_str(),sds_num_);

	for(int i=0;i<5;i++)
	{
		jj_bg_sp_[i]->setEnabled(false);
	}

	std::string menu_path_0;
	std::string menu_path_t;
	Sprite* game_over_tt_sp;
	Sprite* game_over_bb_sp;
	if(game_result_ == false)
	{
        game_over_tt_sp = Sprite::create("gameover/titlesb.png");
        game_over_tt_sp->setPosition(480,430);
		this->addChild(game_over_tt_sp,12);
        menu_path_0 = "gameover/cxks.png";
        menu_path_t = "gameover/cxksp.png";

        game_over_bb_sp = Sprite::create("gameover/bbq.png");
        game_over_bb_sp->setAnchorPoint(Point(0,1));
        game_over_bb_sp->setPosition(0,0);
        game_over_tt_sp->addChild(game_over_bb_sp,9);

		int index = rand()%4;
		Sprite* tips = Sprite::create(StringUtils::format("gamePic/ts%d.png",index));
        tips->setPosition(Point(170,260));
        game_over_bb_sp->addChild(tips,5);

		int temp = UserDefault::getInstance()->getIntegerForKey(Constant::LOSECOUNT.c_str());
		UserDefault::getInstance()->setIntegerForKey(Constant::LOSECOUNT.c_str(),temp+1);
	}else if(game_result_ == true)
	{
		int temp_money = UserDefault::getInstance()->getIntegerForKey(Constant::COUNTGOLD.c_str());
		UserDefault::getInstance()->setIntegerForKey(Constant::COUNTGOLD.c_str(),my_gold_+temp_money);

		//记录总金钱
		int all_money = UserDefault::getInstance()->getIntegerForKey(Constant::GETGOLDCOUNT.c_str());
		UserDefault::getInstance()->setIntegerForKey(Constant::GETGOLDCOUNT.c_str(),all_money+my_gold_);
		//继续游戏
		int number = UserDefault::getInstance()->getIntegerForKey(Constant::LEVELNUM.c_str());

		if(ChooseLevelLayer::levelNum == number && ChooseLevelLayer::levelNum < 4)
		{
			UserDefault::getInstance()->setIntegerForKey(Constant::LEVELNUM.c_str(),ChooseLevelLayer::levelNum+1);
		}

		if(curr_level_sds_ >= 50)
		{
			std::string ss = StringUtils::format("LEVEL%d",ChooseLevelLayer::levelNum);
			UserDefault::getInstance()->setIntegerForKey(ss.c_str(),11);
		}else if(curr_level_sds_ >= 25)
		{
			std::string ss = StringUtils::format("LEVEL%d",ChooseLevelLayer::levelNum);
			UserDefault::getInstance()->setIntegerForKey(ss.c_str(),1);
		}else
		{
			std::string ss = StringUtils::format("LEVEL%d",ChooseLevelLayer::levelNum);
			UserDefault::getInstance()->setIntegerForKey(ss.c_str(),0);
		}

		int temp = UserDefault::getInstance()->getIntegerForKey(Constant::WINCOUNT.c_str());
		UserDefault::getInstance()->setIntegerForKey(Constant::WINCOUNT.c_str(),temp+1);

		game_over_tt_sp = Sprite::create("gameover/titlesl.png");
        game_over_tt_sp->setPosition(Point(480,430));
		this->addChild(game_over_tt_sp,12);
		menu_path_0 = "gameover/jx.png";
		menu_path_t = "gameover/jxp.png";

		game_over_bb_sp = Sprite::create("gameover/bbq.png");
        game_over_bb_sp->setAnchorPoint(Point(0,1));
        game_over_bb_sp->setPosition(Point(0,0));
        game_over_tt_sp->addChild(game_over_bb_sp,9);

		int star_num = player_hkmj_->lifeValue / 8 + 1;

		int divide_line = game_over_bb_sp->getContentSize().width / (star_num+1);
		if(ChooseLevelLayer::froMusicFlag == true)
		{
			std::string musicPath = StringUtils::format("mysound/sound_sfx_star_%d.mp3",star_num);

			CocosDenshion::SimpleAudioEngine::getInstance()		//即使音效
										->playEffect(musicPath.c_str());
		}

		for(int i=0;i<star_num;i++)
		{
			Sprite* star = Sprite::create("gameover/star.png");
            star->setPosition(divide_line+i*divide_line-30*(1-i),260);
            star->setScale(2.0f);
            game_over_bb_sp->addChild(star,5);

            star->runAction(
					Spawn::create(
							MoveTo::create(1.0f,Point(divide_line+i*divide_line,260)),
							ScaleTo::create(1.0f,1),
							NULL)
			);
		}
	}

	MenuItemImage* restart_item = MenuItemImage::create(
            menu_path_0,
            menu_path_t,
			CC_CALLBACK_0(GameLayer::RestartCallback, this)
	);
    restart_item->setPosition(80,100);

	MenuItemImage* tui_chu_item = MenuItemImage::create(
			"gameover/tc.png",
			"gameover/tcp.png",
			CC_CALLBACK_0(GameLayer::TuiChuCallback, this)
	);
    tui_chu_item->setPosition(260,95);

	//创建一个菜单对象
	Menu* menu = Menu::create(restart_item,tui_chu_item,NULL);
	//设置其位置
	menu->setPosition(Point::ZERO);
	//将其添加到布景中
	game_over_bb_sp->addChild(menu, 2);
}
//重新开始回调
void GameLayer::RestartCallback()
{
	PlaySound();
	is_game_over_ = false;
	if(game_result_ == false)//重新开始游戏
	{
        _director->resume();
		this->removeAllChildren();
		select_index_ = -1;
		select_item_ = false;
		wave_num_ = 1;
		enemy_ship_count_ = 10;
		is_game_over_ = false;
		my_gold_ = 3000;//00000000
		init();
	}else if(game_result_ == true)
	{
		Director::getInstance()->resume();
		scene_manager_->goChooseLevelScene();
	}
}
void GameLayer::TuiChuCallback()
{
	PlaySound();
	_director->resume();
	//返回主菜单
	scene_manager_->goStartScene();
}
//海水移动
void GameLayer::OceanUpdate()
{
	if(ocean_bg1_sp_[0]->getPosition().x >=1704)
	{
        ocean_bg1_sp_[0]->setPosition(Point(-567,384));
	}else
	{
        ocean_bg1_sp_[0]->setPosition(Point(ocean_bg1_sp_[0]->getPosition().x+1,384));
	}

	if(ocean_bg1_sp_[1]->getPosition().x >=1704)
	{
        ocean_bg1_sp_[1]->setPosition(Point(-567,384));
	}else
	{
        ocean_bg1_sp_[1]->setPosition(Point(ocean_bg1_sp_[1]->getPosition().x+1,384));
	}
}
void GameLayer::PlaySound()
{
	if(ChooseLevelLayer::froMusicFlag == true)
	{
		CocosDenshion::SimpleAudioEngine::getInstance()
								->playEffect("mysound/sound_sfx_click.mp3");
	}
}
