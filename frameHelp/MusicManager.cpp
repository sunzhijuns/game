#include "MusicManager.h"

bool MusicManager::onSound = true;
bool MusicManager::onEffect = true;
bool MusicManager::noPlaySound = false;

void MusicManager::loadMusic()
{
	onEffect = UserDefault::getInstance()->getBoolForKey("5",true);
	onSound = UserDefault::getInstance()->getBoolForKey("6",true);
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic(
							"sound/BackOnTrack.mp3"					//加载背景音乐
						);
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic(
							"sound/menuLoop.mp3"					//加载背景音乐
						);
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadBackgroundMusic(
							"sound/CantLetGo.mp3"					//加载背景音乐
						);
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect(				//加载音效
				"sound/explode_11.ogg"
			);
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect(				//加载音效
				"sound/playSound_01.ogg"
			);
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect(				//加载音效
				"sound/quitSound_01.ogg"
			);
}

void MusicManager::resumeBackgroundMusic()//继续背景音乐
{
	onSound = true;
	UserDefault::getInstance()->setBoolForKey("6", onSound);
	UserDefault::getInstance()->flush();
	//继续背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
	return;
}
void MusicManager::pauseBackgroundMusic()//暂停背景音乐
{
	onSound = false;
	UserDefault::getInstance()->setBoolForKey("6", onSound);
	UserDefault::getInstance()->flush();
	//暂停背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
	return;
}

void MusicManager::playExplosionEffect()//音效
{
	if(onEffect == false)
	{
		return;
	}
	//暂停背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/explode_11.ogg");	//播放音效
	return;
}
void MusicManager::playFlyLayerEffect()//进入关卡时的音效
{
	if(onEffect == false)
	{
		return;
	}
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/playSound_01.ogg");	//播放音效
	return;
}
void MusicManager::playPauseMenuEffect()//暂停菜单中点击菜单时的音效
{
	if(onEffect == false)
	{
		return;
	}
	CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sound/quitSound_01.ogg");	//播放音效
	return;
}
void MusicManager::playMenuLayerMusic()//菜单背景音乐
{
	if(onSound == false)
	{
		return;
	}
	//播放背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
	(
		"sound/menuLoop.mp3",
		true
	);
	return;
}
void MusicManager::playGameLayerMusic()
{
	if(onSound == false)
	{
		return;
	}
	//播放背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
	(
		"sound/BackOnTrack.mp3",
		true
	);
	return;

}
void MusicManager::playFlyLayerMusic()//FlyLayer的背景音乐
{
	if(onSound == false)
	{
		return;
	}
	//播放背景音乐
	CocosDenshion::SimpleAudioEngine::getInstance()->playBackgroundMusic
	(
		"sound/CantLetGo.mp3",
		true
	);
	return;
}
