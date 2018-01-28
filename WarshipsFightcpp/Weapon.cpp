#include "cocos2d.h"
#include "Weapon.h"
using namespace cocos2d;

Weapon* Weapon::create(int index,Point p,int launchForm)
{
	Weapon* tempWp = new Weapon(launchForm);
	std::string objPath = StringUtils::format("obj/daodan%d.obj",index);
	std::string pngPath = StringUtils::format("obj/daodan%d.png",index);
	tempWp->initWithFile(objPath);
	tempWp->setTexture(pngPath);
	tempWp->setRotation3D(Vertex3F(-90,90,0 ));
	tempWp->setPosition(p);
	return tempWp;
}
Weapon::Weapon(int launchForm)
{
	this->launchForm = launchForm;
}
