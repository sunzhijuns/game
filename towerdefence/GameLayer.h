#ifndef _GameLayer_H_
#define _GameLayer_H_
#include "GameSceneManager.h"
#include "Weapon.h"
#include "DialogLayer.h"
#include "BulletSprite.h"
#include "cocos2d.h"

using namespace std;
using namespace cocos2d;

class GameLayer : public cocos2d::Layer
{
public:
	//构造函数
	GameLayer();
	//析构函数
	virtual ~GameLayer();
	//重载的父类的初始方法
	virtual bool init();
	Menu* menu_;
	void ZanTing(Ref* pSender);
	//计算路径之前的一些容器的初始化
	void InitForCalculate();
	//添加所有label
	void AddLabel();
	//广度优先A*算法
    bool BFSAStar();
	//初始化去过未去过的数组
	void InitVisitedArray();
	//释放内存
	void FreeMemory();
	//声明计算路径的方法
    bool CalculatePath();
	//打印最后的路径
	void PrintPath();
	//添加防御塔菜单精灵
	void AddMenuSprite();
	//设置防御塔菜单精灵可见
	void SetWeaponTrue();
    //准备方法--画圆型
    void Ready();
    //声明怪从action数组里出来挨个走的方法
    void Run();
    //创建多个怪
    void FoundMonsters();
    //出怪的方法
    void MonsterRun(Node* node);
    void RemoveSpriteAdd();
    //删除精灵的方法
    void RemoveSprite(Node* node);
    //将地图格子行列号转换为对应格子的贴图坐标
    Point FromColRowToXY(int col, int row);
    //将触控点位置转换为地图格子行列号
    Point FromXYToColRow(int xPos, int yPos);
	//出售防御塔的方法
	void SellWeapon(Weapon* weapon);
	//设置升级菜单精灵可见
	void SetUpdateTrue();
	//设置升级金币
	void SetValue();
	//触控开始的方法
    bool onTouchBegan(Touch *pTouch, Event *pEvent);
    //触控移动的方法
    void onTouchMoved(Touch *pTouch, Event *pEvent);
    //触控抬起的方法
    void onTouchEnded(Touch *pTouch, Event *pEvent);
    //出野怪时的特效
	void AddParticle(Point point,float time);
	//野怪到终点时的特效
	void AddParticle(Point point,int id,float time);
	//野怪死时的特效
	void AddParticle1(Point point,int id,float time);
	//游戏结束时的爆炸特效
	void AddParticle(Point point,int id,float time,float angle);
	void AddParticle2(Point point,float time);
	//回调方法
	void MenuCallbackItem0(Object *pSender);
	//攻击怪的方法
    void Attack();
    //第一个防御塔攻击怪掉血的方法
	void FireBulletOne(int weap,int target,float direction,Point position,float length_vector);
	//第二个防御塔攻击怪掉血的方法
	void FireBulletTwo(int weap,int target,float direction,Point position);
	//第三个防御塔攻击怪掉血的方法
	void FireBulletThree(int weap,int target,float direction,Point position);
	//发射子弹的方法
	void RunBullet();
	//怪掉血的方法
	void CutBloodOne(Node* node);
    //执行特效菜单的回调方法
    void PlayGameCallback();
    //游戏结束的方法
	void LoseGame();
public:
	int bullet_data_[10];
	//声明指向DrawNode类对象的指针
	DrawNode* dn_;
	//TMXLayer指针
	TMXLayer* tmx_layer_;
	Point end_world_;
	//创建动作指针
	ProgressTimer *left_;
	Sprite* particle_;
	Sprite* bullet1_;
	Sprite* cc_;
	float TIME_MAIN;
	//半个图块的大小（即界面中路线的偏移量）此处要有一个调整值（参数为瓦片图片的半边宽，瓦片图片的半边高）
	Point transform_;
	//当前分数的label
	LabelTTF *score_label_;
	//当前回合数的label
	LabelTTF *pass_label_;
	//当前金钱
	LabelTTF *money_label_;
	//当前生命数
	LabelTTF *ten_label_;
	//显示升级武器所需金钱的label
	LabelTTF *update_money_label_;
	//显示出售武器获得的金钱的label
	LabelTTF *sell_money_label_;
	//存放怪的数组
	Array* monsters_;
	//存放防御塔
	Array* weapons_;
	//存放菜单防御塔精灵
	Array* menus_;
	//存放action动作
	Array* actions_;
	//存放跟踪
	Array* bullets_;
	//存放升级的菜单按钮精灵
	Array* sell_update_menus_;
	//设置世界坐标系
	Point start_world_;
	//怪运动标志位
	bool is_monster_run_;
	//创建怪的标志位
	bool is_found_monster_;
	//场景管理制指针
	GameSceneManager* scene_manager_;
	//算法计算完毕的标志位
	bool is_caulate_over_;
	//声明表示生命值的对象
	int ten_;
	//玩家的金币
	int money_;
	//开始标志精灵
	Sprite* start_sprite_;
	//野怪精灵对象
	Sprite *ye_guai_sprite_;
	//目标精灵
	Sprite* target_sprite_;
	//地图的row
	int row_;
	//地图的col
	int col_;
	//以竖放向来看（从左至右，从上至下）以下为动态二维数组的创建
	int** map_data_;
	//存放路径的数组
	vector <Point > way_;
	vector <int> bullet_array_;
	//记录路径长度
	int** length_;
	//广度优先所用队列
	queue<int(*)[2]>* my_queue_;
	//结果路径记录
	map<string, int(*)[2]>* hm_;
	//typedef自定义类型
	typedef vector<int(*)[2]>* INTPVECTORP;
	//第一次点击在菜单位置标志位
	bool is_touch_move_ = false;
	//在touchEnd里是否移除防御塔的标志位
	bool is_remove_weap_;
	//多少批怪计数
	int pass_;
	//当前总得分数
	int score_;
	//升级的武器
	Weapon* update_weapon_;
	//武器升级的标志位
	bool is_weapon_update_;
	//声明游戏结束的标志位
	bool is_game_over_;
	//暂停游戏的标志位
	static bool is_pause_;

	//声明创建防御塔时的光圈精灵对象
	Weapon* tr_sprite_;
	//声明四个菜单防御塔的精灵对象
	Weapon *one_player_;
	Weapon *two_player_;
	Weapon *three_player_;
	Weapon *four_player_;

	CREATE_FUNC(GameLayer);
};

#endif
