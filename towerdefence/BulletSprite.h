#ifndef  _BulletSprite_H_
#define  _BulletSprite_H_

#include "cocos2d.h"
#include "Monsters.h"

//自定义的精灵类
class BulletSprite : public cocos2d::Sprite
{
public:
	//构造函数
    BulletSprite();
	//子弹的编号，有三种1，2，3
	int id_;
	//子弹的伤害值
	int hurt_;
	//目标野怪
	int target_;
	//子弹的角度
	float angle_;
    //创建子弹的方法(参数为图片和子弹的id)
    static BulletSprite* Create(const char* pic,int id);
    //创建子弹对象(参数为图片伤害值和目标野怪)
    static BulletSprite* Create(const char* pic,int hurt,int target);
    //怪物减血方法
    void Update(int bullet_id);

    //做好相应的初始化与释放工作
    CREATE_FUNC(BulletSprite);
};

#endif
