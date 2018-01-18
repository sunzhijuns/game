#ifndef __MusicUtil_H__
#define __MusicUtil_H__

#include <audio/include/SimpleAudioEngine.h>
#include "AppMacros.h"

using namespace std;

//白键播放音效
void playMusic(int number, string instrument)
{
	string route;
	switch(number)
	{
	case 0:
		route = music_RESOURE_PATH + instrument + "00.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 1:
		route = music_RESOURE_PATH + instrument + "01.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 2:
		route = music_RESOURE_PATH + instrument + "02.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 3:
		route = music_RESOURE_PATH + instrument + "03.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 4:
		route = music_RESOURE_PATH + instrument + "04.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 5:
		route = music_RESOURE_PATH + instrument + "05.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 6:
		route = music_RESOURE_PATH + instrument + "06.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 7:
		route = music_RESOURE_PATH + instrument + "07.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 8:
		route = music_RESOURE_PATH + instrument + "08.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 9:
		route = music_RESOURE_PATH + instrument + "09.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 10:
		route = music_RESOURE_PATH + instrument + "10.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 11:
		route = music_RESOURE_PATH + instrument + "11.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 12:
		route = music_RESOURE_PATH + instrument + "12.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 13:
		route = music_RESOURE_PATH + instrument + "13.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 14:
		route = music_RESOURE_PATH + instrument + "14.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 15:
		route = music_RESOURE_PATH + instrument + "15.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 16:
		route = music_RESOURE_PATH + instrument + "16.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 17:
		route = music_RESOURE_PATH + instrument + "17.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 18:
		route = music_RESOURE_PATH + instrument + "18.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 19:
		route = music_RESOURE_PATH + instrument + "19.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 20:
		route = music_RESOURE_PATH + instrument + "20.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 21:
		route = music_RESOURE_PATH + instrument + "21.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 22:
		route = music_RESOURE_PATH + instrument + "22.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 23:
		route = music_RESOURE_PATH + instrument + "23.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 24:
		route = music_RESOURE_PATH + instrument + "24.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 25:
		route = music_RESOURE_PATH + instrument + "25.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 26:
		route = music_RESOURE_PATH + instrument + "26.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 27:
		route = music_RESOURE_PATH + instrument + "27.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 28:
		route = music_RESOURE_PATH + instrument + "28.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 29:
		route = music_RESOURE_PATH + instrument + "29.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 30:
		route = music_RESOURE_PATH + instrument + "30.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 31:
		route = music_RESOURE_PATH + instrument + "31.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 32:
		route = music_RESOURE_PATH + instrument + "32.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 33:
		route = music_RESOURE_PATH + instrument + "33.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 34:
		route = music_RESOURE_PATH + instrument + "34.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 35:
		route = music_RESOURE_PATH + instrument + "35.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 36:
		route = music_RESOURE_PATH + instrument + "36.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	case 37:
		route = music_RESOURE_PATH + instrument + "37.mid";
		CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(route.c_str());
		break;
	}
}

string getInstrumentChinese(int number)
{
	string instrument;
	switch(number)
	{
	case 1:
		instrument = "\u94a2\u7434";
		break;
	case 2:
		instrument = "\u8428\u514b\u65af";
		break;
	case 3:
		instrument = "\u97f3\u4e50\u76d2";
		break;
	case 4:
		instrument = "\u5409\u4ed6";
		break;
	case 5:
		instrument = "\u5c0f\u63d0\u7434";
		break;
	case 6:
		instrument = "\u957f\u7b1b";
		break;
	case 7:
		instrument = "\u6c34\u6676\u6548\u679c";
		break;
	case 8:
		instrument = "\u53e4\u7b5d";
		break;
	}
	return instrument;
}

string getInstrument(int number)
{
	string instrument;
	switch(number)
	{
	case 1:
		instrument = "piano";
		break;
	case 2:
		instrument = "sax";
		break;
	case 3:
		instrument = "mb";
		break;
	case 4:
		instrument = "guitar";
		break;
	case 5:
		instrument = "violin";
		break;
	case 6:
		instrument = "flute";
		break;
	case 7:
		instrument = "fxc";
		break;
	case 8:
		instrument = "koto";
		break;
	}
	return instrument;
}

#endif
