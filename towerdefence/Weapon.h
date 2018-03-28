#ifndef  _Weapon_H_
#define  _Weapon_H_
#include "cocos2d.h"

using namespace cocos2d;

//自定义的精灵类
class Weapon : public cocos2d::Sprite
{
private:
	Sprite* _scope;
public:
	bool IsCanUpdate(){
		return level_ != 4;
	}
	void ShowScope(){
		if(_scope != NULL){
			_scope->setVisible(true);
		}
	}
	void HideScope(){
		if(_scope != NULL){
			_scope->setVisible(false);
		}
	}
    Weapon();
	//防御塔的id， 1，2，3，4（白绿红蓝）
	int id_;
	//武器的级别，1，2，3，4
	int level_;
	//发射子弹的速率
	float update_time_;
	//防御塔的攻击范围
	int confines_;
	//防御塔的伤害值
	int hurt_;
	//防御塔的角度
	float angle_;
	//安装防御塔需要的金币数
	int value_;
	//升级防御塔需要的金币数
	int update_value_;
	//卖掉防御塔得到的金币数
	int sell_value_;
	//发射子弹的标志位
	bool is_can_fire_;
	//更新防御塔的标志位
	bool is_update_mark_;

    //创建防御塔的几个方法
	static Weapon* Create(int id, int level, float angle);
	static Weapon* Create(int id);
	static Weapon* Create(const char* pic, int id);
    //升级防御塔的方法
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
