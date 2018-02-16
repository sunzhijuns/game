#ifndef  _Monsters_H_
#define  _Monsters_H_
#include "cocos2d.h"
#include <vector>

using namespace cocos2d;
using namespace std;
//自定义的精灵类
class Monsters : public cocos2d::Sprite
{
public:
	//怪物的当前血量
	int blood_;
	//怪物的编号，有三种怪物 1，2，3
	int id_;
	//怪所走的第几个路径
	int way_;
	//存放路径的数组
	vector <Point > self_way_;
	//最大血量
	int max_blood_;
	//构造函数
    Monsters();
    //创建野怪的方法
    static Monsters* Create(int id,vector <Point > self_way);
    //创建野怪的方法
    static Monsters* Create(int id,int blood,int way,int max_blood,vector <Point > self_way);
    //怪物减血的方法
    void CutBlood(int);//入口参数子弹的id
    //设置怪头顶的血条为是否可见的方法
    void SetVisibleFalse();
    //声明第二种野怪运动过程中转弯时要调用的方法
    void Refresh(float angle);
    //删除精灵对象的方法
    void RemoveSprite(Node* node);

    //并做好相应的初始化与释放工作
    CREATE_FUNC(Monsters);
};

#endif
