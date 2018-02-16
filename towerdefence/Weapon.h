#ifndef  _Weapon_H_
#define  _Weapon_H_
#include "cocos2d.h"

using namespace cocos2d;

//自定义的精灵类
class Weapon : public cocos2d::Sprite
{
public:
	//构造函数
    Weapon();
	//防御塔的id， 1，2，3，4（白绿红蓝）
//	int id;
	int id_;
	//武器的级别，1，2，3，4
//	int level;
	int level_;
	//发射子弹的速率
//	int updatetime;
	float update_time_;
	//防御塔的攻击范围
//	int confines;
	int confines_;
	//防御塔的伤害值
//	int hurt;
	int hurt_;
	//防御塔的角度
//	float angle;
	float angle_;
	//安装防御塔需要的金币数
//	int value;
	int value_;
	//升级防御塔需要的金币数
//	int upValue;
	int update_value_;
	//卖掉防御塔得到的金币数
//	int sellValue;
	int sell_value_;
	//发射子弹的标志位
//	bool fire;
	bool is_can_fire_;
	//更新防御塔的标志位
//	bool updateMark;
	bool is_update_mark_;
    //创建防御塔的几个方法
//    static Weapon* create(int id,int level,float angle);
	static Weapon* Create(int id, int level, float angle);
//    static Weapon* create(int id);
	static Weapon* Create(int id);
//    static Weapon* create(const char* pic,int id);
	static Weapon* Create(const char* pic, int id);
    //升级防御塔的方法
//    void updateData();
	void UpdateData();
    //升级防御塔前的准备方法
    void Update();
    //发射子弹的方法
    void Fireing();
    //设置发射子弹的方法
    void SetCanFire();
    Point point_col_row_;
    ProgressTimer *left_;

    //系统定义的一个宏，做好相应的初始化与释放工作
    CREATE_FUNC(Weapon);
};

#endif
