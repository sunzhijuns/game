#ifndef _GameLayer_H_
#define _GameLayer_H_
#include "cocos2d.h"
#include "WarshipsFightSceneManager.h"
#include "WarShipObject.h"
#include "HKMJObject.h"
#include "Weapon.h"
#include <list>
#include <queue>
using namespace cocos2d;
class GameLayer :public Layer
{
public:
	WarshipsFightSceneManager* scene_manager_;
    TMXTiledMap* map_;                      //声明指向地图对象的指针

    Point first_touch_point_;				//第一次被触摸的点

	Point map_point_ ;						//地图的坐标
	MenuItemImage* jj_bg_sp_[5];            //声明军舰背景菜单项
	MenuItemImage* ship_weapon_ [3];		//导弹菜单项
	Sprite* selected_sp_;                   //选中框的精灵对象
	int select_index_ = -1;                 //选中军舰的索引
	Sprite* life_bar_sp_[20];               //基地的血条
	HKMJObject* player_hkmj_;               //玩家基地对象
	bool select_item_ = false;             //是否选中军舰的标志位
	int** map_data_;						//地图数据
	TMXLayer* collide_layer_;   			//地图碰撞检测层
	int row_;//行号
	int col_;//列号
	std::vector<Point>*path_;           //储存行驶路径的向量
	std::vector<Point>*temp_path_;      //储存行驶路径的向量
	WarShipObject* last_touch_sprite_;  //上一次点击的军舰对象
	WarShipObject* curr_touch_sprite_;  //当前点击的军舰对象

	bool select_ship_ = false;//是否选中军舰

	Animate* animate_action_[2];		//精灵帧动画
	int wave_num_ = 1;		//敌军进攻的波数
	Label * label_wave_index_;			//显示波数的文本标签
	int enemy_ship_count_ = 10;			//敌船的总数
	bool is_game_over_ = false;			//游戏是否结束
	bool game_result_=false;			//游戏的胜利或者失败
	int my_gold_ = 3000000;						//最初玩家金钱
	LabelAtlas* money_label_;			//显示玩家金钱的文本标签
	int sds_num_ = 0;					//杀敌数
	int curr_level_sds_=0;				//当前关卡
	int cur_get_gold_=0;				//获得金币
	LabelAtlas* sds_label_;				//显示杀敌数的文本标签

	//暂停菜单项
    MenuItemImage* pause_menu_;
	//暂停背景
	Sprite* game_pause_bb_;
	//海洋背景
	Sprite* ocean_bg1_sp_[2];
	//必杀技冷却精灵
	Sprite* cool_sp_[2];
			//导弹冷却精灵对象
			//军舰冷却精灵对象
	Sprite* (weapone_sprite_)[2];
	Sprite* (ship_cool_sprite_)[5];

	bool cool_spb_=false;
	bool cool_spp_=false;
	//结果路径记录
	std::map<std::string,int(*)[2]>* hash_map_;
	//广度优先所用队列
	std::queue<int(*)[2]>*my_queue_;
	//半个图块的大小（即界面中路线的偏移量）此处要有一个调整值（参数为瓦片图片的半边宽，瓦片图片的半边高）
	Point transform_;

	std::vector<Weapon*>* all_weapon_vector_;
	std::vector<Weapon*>* delete_weapon_vector_;

	std::vector<WarShipObject*>*all_ship_vector_;
	std::vector<WarShipObject*>*delete_ship_vector_;

	//储存方向优先顺序的数组
	int result_sequence_[8][2];
	Sprite3D* plane_3d_[2];
	//飞机飞行的标志位
	bool plane_flag_=false;
	int plane_step_ = 1;							//飞机飞行阶段

	//初始化布景方法
	virtual bool init();
	void InitMiscellaneous();
	void InitSound();
	void InitListenerTouchAndCallback();
	void InitRaining();

	virtual void update(float delta);

	//爆炸换帧精灵
	void InitBoomFrame();
	void InitBigBoomFrame();
	void ExpansionRing();
	//初始化军舰选项的方法
	void InitJJItem();
	//初始化地图
	void InitTMXMap();
	//初始化海洋背景
	void InitOceanBg();
	//初始化航母
	void InitHKMJ();
	//初始话地图碰撞检测的数组
	void InitTMXPZJCArray();
	//初始化路径搜索数组
	void initVisitedArr();
	void InitVisitedArray();
	//初始化暂停菜单项
	void InitPauseMenu();
	void PauseCallback();
	void BackCallback();
	//添加我方军舰的方法
	void AddWarShip(Point touch_point, int select_num);
	//触摸监听的方法
	bool onTouchBegan(Touch* touch, Event* event);
	void onTouchMoved(Touch* touch, Event* event);
	void onTouchEnded(Touch* touch, Event* event);

	bool OnShipTouchBegan(Touch* touch, Event* event);
	void OnShipTouchEnded(Touch* touch, Event* event);
	//点击菜单项切换选中船只的方法
	void JJBgSpCallback0();
	void JJBgSpCallback1();
	void JJBgSpCallback2();
	void JJBgSpCallback3();
	void JJBgSpCallback4();

	//切换武器方法
	void ShipWeaponCallback0();
	void ShipWeaponCallback1();
	//取消菜单项
	void ShipWeaponCallback2();
	//搜索船只路径
	void SearchPath(WarShipObject* war_ship);
	//广度优先算法寻径
    void BFSCalculatePath(WarShipObject* war_ship_object);
	void TransformBTMapAndVector(WarShipObject* war_ship_object);
	void CreateFiniteTimeActionByVector(WarShipObject* war_ship_object);

	//放到储存行驶路径的向量中
	void TransformPath();
	//转换为格子行列号
	Point TouchPointToRowCol(Point touch_point);
	//获取方向转向
	int OrderDirection(float dx, float dy);
	//给船只添加监听
	void AddTouchListener(WarShipObject * war_ship_object);
	//点击军舰出现的精灵
	void WarShipAddChild(WarShipObject* war_ship_object);

	//显示当前选中的船只的导弹
	void ShowWarShipItem(WarShipObject* war_ship_object);
	//添加导弹
	bool AddWeapon(int select_index,Point start_point, Point final_point,
				   WarShipObject* ship, int launch_form);

	//添加瞄准船只的精灵
	void AddAimSprite();
	//添加显示船只攻击范围的精灵
	void AddSaveSprite(WarShipObject* war_ship_object);
	//添加错误提示
	void AddWrongPrompt(Point location_map);
	//添加敌船
	void AddEnemyShip();
	//对搜索路径的方向优先顺序进行重新排序
	void RandomSort();
	//更新武器冷却时间的定时回调方法
	void WeaponsCoolTimeUpdate();
	//删除船只的方法
	void RemoveShipUpdate();
	//游戏开始的回调方法
	void StartCallback();
	//游戏开始前倒计时的方法
	void GameStartDaoJiShi();
	//定时更新武器状态的方法
	void WeaponeStateUpdate();
	//计算导弹Z坐标的方法
	float CalculatePointZ(float x, Weapon* weapon);
	//计算导弹Y坐标的方法
	float CalculatePointY(float x, Weapon* weapon);
	//计算导弹在运行过程中的旋转角度的方法
	float CalculateDegree(float x, Weapon*weapon);
	//导弹到达指定位置后，判断在冲击范围内是否在船只能被摧毁的方法
	void DestroyEnemyShip(Weapon* weapon);
	//删除船只时播放的爆炸效果的方法
	void RemoveShipPlayEffect(Point temp_point);
	//计算船只
	void CalculateNearestSmartIn();
	//计算两个船只距离的方法
	float Distance(WarShipObject* d1, WarShipObject* d2);
	//计算两点之间的距离
	float CalculateTwoPointDistance(Point start_point, Point end_point);
	//当敌船在航母周围时
	void EnemyShipAtHKMJUpdate();
	//航空母舰掉血
	void ReduceBlood();
	//游戏结束的方法               1为游戏失败，0为胜利
	void GameOver();
	//重新开始或继续游戏
	void RestartCallback();
	//退出游戏，回主菜单
	void tcCallback();
	void TuiChuCallback();
	//海洋效果滚屏
	void OceanUpdate();
	//瞬间杀死全部敌船
	void DestroyAllEnemyShip();
	//航空母舰的必杀技冷却定时回调
	void PlayerHKMJCoolTimeUpdate();
	//航空母舰的必杀技，飞机
	void PlaneFlyAtShip();
	void PlaneUpdate();
	void RecoveryStateCallback();
	void PlaySound();

	CREATE_FUNC(GameLayer);
public:
	int target_[2] = {0};
//	~GameLayer();
};

#endif
