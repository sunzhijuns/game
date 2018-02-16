#include "Monsters.h"
#include "cocos2d.h"
#include "GameSceneManager.h"

using namespace cocos2d;

//构造函数
Monsters::Monsters(){}

//初始时创建怪对象的方法(参数为怪的id，及路径)
Monsters* Monsters::Create(int id,vector <Point > self_way)//入口参数怪物的id
{
	//几种野怪的图片路径
	std::string pic_table[6] = {"pic/square.png","pic/triangle.png","pic/circle.png",
									"pic/id_4.png","pic/id_5.png","pic/id_6.png"
	};
	//创建一个野怪对象
	Monsters* temp = new Monsters();
	//初始化野怪精灵对象
	if (temp && temp->initWithFile(pic_table[id-1].c_str()))
	{
		//自动释放
		temp->autorelease();

		//获取当前野怪的id
		temp->id_ = id;
		//根据id设置野怪的血量
		temp->blood_ =100*id;
		//根据id设置怪的最大血量值
		temp->max_blood_=100*id;
		//初始化路径
		temp->way_=0;
		//野怪拿到属于自己的路径
		temp->self_way_ = self_way;
		//创建一个表示野怪血条的精灵对象
		Sprite* blood = Sprite::create("pic/blood.png");
		//设置其锚点
		blood->setAnchorPoint(Point(0.5,0));
		//起始设置为不可见
		blood->setVisible(false);
		//设置位置
		blood->setPosition(Point(0,0));
		//设置血条的长度
		blood->setScaleY(1);
		//将血条精灵对象添加到布景中
		temp->addChild(blood,6,1);

		return temp;
	}
	else
	{
		CC_SAFE_DELETE(temp);
		return nullptr;
	}
}
//创建怪对象
Monsters* Monsters::Create(int id,int blood,int way,int max_blood,vector <Point > self_way)//入口参数怪物的id
{
	std::string pic_table[6] = {"pic/square.png","pic/triangle.png","pic/circle.png",
									"pic/id_4.png","pic/id_5.png","pic/id_6.png"
	};
	//创建一个野怪精灵对象
	Monsters* temp = new Monsters();
	//初始化野怪精灵对象
	if (temp && temp->initWithFile(pic_table[id-1].c_str()))
	{
		//自动释放
		temp->autorelease();

		//拿到当前怪的id
		temp->id_ = id;
		//拿到当前怪的血量
		temp->blood_ = blood;
		//拿到当前怪的最大血量
		temp->max_blood_=max_blood;
		//拿到当前的路径
		temp->way_=way;
		//拿到存放路径的数组
		temp->self_way_ = self_way;
		//创建一个表示血量的精灵对象
		Sprite* blood1 = Sprite::create("pic/blood.png");
		//设置锚点
		blood1->setAnchorPoint(Point(0.5,0));
		//设置为不可见
		blood1->setVisible(false);
		//设置位置
		blood1->setPosition(Point(0,0));
		//设置血条的长度
		blood1->setScaleY(1);
		//添加到布景中
		temp->addChild(blood1,6,1);

		return temp;
	}
	else
	{
		CC_SAFE_DELETE(temp);
		return nullptr;
	}
}

//怪物减血的方法
void Monsters::CutBlood(int hurt)//入口参数子弹的id
{
	//被击中后当前血量值等于原血量值减去子弹的伤害值
	this->blood_ -= hurt;
	//将血条设置为可见
	(this->getChildByTag(1))->setVisible(true);
	CCLOG("---------%p",this);
	//剩余血量值等于当前血量值比上最大血量值
	float scaleY = (float)blood_/max_blood_;
	//设置血量值
	(this->getChildByTag(1))->setScaleY(scaleY);
	//顺序执行怪血条可见与不可见的效果，延迟为0.5秒
	(this->getChildByTag(1))->runAction(
			Sequence::create(
								DelayTime::create(0.5),
								CallFuncN::create(CC_CALLBACK_0(Monsters::SetVisibleFalse,this)),
								NULL
								)

	);
}

//设置怪头顶的血条为是否可见的方法
void Monsters::SetVisibleFalse()
{
	((Sprite*)(this->getChildByTag(1)))->setVisible(false);
}

//第二种野怪运动过程中转弯时要调用的方法
void Monsters::Refresh(float angle)
{
	//怪中心到左上角点向量
	Point vector;
	//怪的属性
	Point origin;
	//怪精灵的宽
	origin.x = this->getContentSize().width/2;
	//怪精灵的高
	origin.y = this->getContentSize().height/2;
	vector.x = -origin.x;
	vector.y = -origin.y;
	//求向量的长度
	float length = vector.getLength();
	//获取此向量的角度
	float angle_origin = vector.getAngle();
	//矫正血条的角度，保证怪旋转的时候血条不旋转
	Point direction= Vec2::forAngle(-angle+angle_origin);
	//计算血条相对屏幕的位置
	Point position= origin + direction * length;
	//设置血条的位置
	((Sprite*)(this->getChildByTag(1)))->setPosition(position);
	//设置血条的角度
	((Sprite*)(this->getChildByTag(1)))->setRotation(angle*180/3.1415925);
}

//删除血条精灵对象的方法
void Monsters::RemoveSprite(Node*node)
{
	this->removeChild(node,true);
}
