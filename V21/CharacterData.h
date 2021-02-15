#pragma once

#include "Offsets.h"
#include "CharacterProperties.h"

namespace V21
{
	enum class Character : unsigned int
	{
		Aatrox = 0x71097207,
		Ahri = 0xD4BD813E,
		Akali = 0x5F193A14,
		Alistar = 0x7D004170,
		Amumu = 0xC60111F1,
		Anivia = 0xE6318952,
		Annie = 0xF111751D,
		Ashe = 0xDA1E294F,
		AurelionSol = 0x60C9D8ED,
		Azir = 0xDD919622,
		Bard = 0xFFD1E571,
		Blitzcrank = 0x9D92842A,
		Brand = 0xE8B74EC7,
		Braum = 0xE8BE5089,
		Caitlyn = 0x601C4B58,
		Camille = 0x6FF5CBEB,
		Cassiopeia = 0x205BB971,
		Chogath = 0x3094E898,
		Corki = 0xA96B4264,
		Darius = 0x52186A32,
		Diana = 0xCDD2B60F,
		Draven = 0xC42DA22C,
		DrMundo = 0xEE5915A3,
		Ekko = 0x904BF10A,
		Elise = 0xA1490494,
		Evelynn = 0x6089E761,
		Ezreal = 0x84DCF93,
		Fiddlesticks = 0x5783643F,
		Fiora = 0x5C944E1B,
		Fizz = 0xBDE5A683,
		Galio = 0x2AC6F678,
		Gangplank = 0x33D903EF,
		Garen = 0x2DB75281,
		Gnar = 0xEEC9BE98,
		Gragas = 0xFF4BDC63,
		Graves = 0x6B2C5EE,
		Hecarim = 0xDEFD19BB,
		Heimerdinger = 0xD77595D9,
		Illaoi = 0x90FF0372,
		Irelia = 0xE83E3148,
		Ivern = 0x804569D4,
		Janna = 0xF78D33AA,
		JarvanIV = 0xB1F7EC95,
		Jax = 0x349383C1,
		Jayce = 0xFCECDB84,
		Jhin = 0x7771D5C3,
		Jinx = 0x77F4E689,
		Kaisa = 0x39084661,
		Kalista = 0xE0DB4F6D,
		Karma = 0x3D70D070,
		Karthus = 0x72C6D76E,
		Kassadin = 0x9A84477E,
		Katarina = 0x381260D,
		Kayle = 0x40E23CBC,
		Kayn = 0xA2963DEB,
		Kennen = 0x454B9583,
		Khazix = 0xBA24D3E5,
		Kindred = 0x30E7F07D,
		Kled = 0xA7ECE380,
		KogMaw = 0xA65585E0,
		Leblanc = 0xEB2D6693,
		LeeSin = 0xBB34EE8C,
		Leona = 0x39FF4429,
		Lissandra = 0x884BF49,
		Lucian = 0x97F8A01C,
		Lulu = 0xDAE94192,
		Lux = 0x35A3A7AF,
		Malphite = 0xC1DCEAAA,
		Malzahar = 0x22D7D75A,
		Maokai = 0x8EFB7D38,
		MasterYi = 0xE5A429F2,
		MissFortune = 0x1E5A725,
		MonkeyKing = 0x205D23CA,
		Mordekaiser = 0x166C307E,
		Morgana = 0xF537FDDD,
		Nami = 0x2E1EAD2F,
		Nasus = 0x9BBFEFE,
		Nautilus = 0x6C318333,
		Nidalee = 0x7DA436B4,
		Nocturne = 0xC4AADC06,
		Nunu = 0x37F8E38E,
		Olaf = 0x62042582,
		Orianna = 0x70018CA6,
		Ornn = 0x650585C3,
		Pantheon = 0xE51EFF33,
		Poppy = 0x1B74F7BA,
		Pyke = 0x96FBC243,
		Quinn = 0x7315CE25,
		Rakan = 0x1568FA09,
		Rammus = 0xA1DBE5ED,
		RekSai = 0x57601883,
		Renekton = 0x1C0BE672,
		Rengar = 0xDD0BD0BD,
		Riven = 0x8F0E2B88,
		Rumble = 0xEADFA531,
		Ryze = 0xF4186772,
		Sejuani = 0x887A3ADF,
		Shaco = 0x9A1A73B8,
		Shen = 0x1A2B2B7E,
		Shyvana = 0xF45FC062,
		Singed = 0x2EDAEEE,
		Sion = 0x1AB33D75,
		Sivir = 0xD2FE8B89,
		Skarner = 0x24FE8072,
		Sona = 0x1DA69A2F,
		Soraka = 0x51A4D061,
		Swain = 0x5406B062,
		Syndra = 0x404673AB,
		TahmKench = 0xD93E9107,
		Taliyah = 0x6B3F0A56,
		Talon = 0x9DCDCAFE,
		Taric = 0xA0BC267F,
		Teemo = 0x5474A0F6,
		Thresh = 0x8E5AD89C,
		Tristana = 0x23CDE228,
		Trundle = 0x25D6A766,
		Tryndamere = 0xDE11375B,
		TwistedFate = 0xD04DE692,
		Twitch = 0xEDC9F793,
		Udyr = 0x755493E8,
		Urgot = 0xF637B92F,
		Varus = 0x28A0E785,
		Vayne = 0x2C0C5245,
		Veigar = 0x4D1B41FE,
		Velkoz = 0xDAB5F5B9,
		Vi = 0x761D73,
		Viktor = 0xC04F2C7F,
		Vladimir = 0xBECBD446,
		Volibear = 0x3F2A754A,
		Warwick = 0x8CA82C2,
		Xayah = 0xB3D80D17,
		Xerath = 0x195002D0,
		XinZhao = 0xBBB6179,
		Yasuo = 0xF4E41405,
		Yorick = 0xDA9791F5,
		Zac = 0x3C747BBC,
		Zed = 0x3C787CB9,
		Ziggs = 0xA7103B84,
		Zilean = 0x40169635,
		Zoe = 0x3C827F30,
		Zyra = 0x6846EB6E,
		AzirSunDisc = 0x7EA1F0BF,
		AzirSoldier = 0x71B0E7E0,
		SyndraSphere = 0x95638AD8,
		ZyraSeed = 0xA5BDBA5F,
		SRU_ChaosMinionMelee = 0xB87BB4C7,
		SRU_ChaosMinionRanged = 0xD86EA814,
		SRU_ChaosMinionSiege = 0x6B0C5C0,
		SRU_ChaosMinionSuper = 0x3A6B38CE,
		SRU_OrderMinionMelee = 0xD11193B9,
		SRU_OrderMinionRanged = 0xC44285A2,
		SRU_OrderMinionSiege = 0x1F46A4B2,
		SRU_OrderMinionSuper = 0x530117C0,
		SRU_Plant_Vision = 0x4D61E805,
		SRU_Plant_Satchel = 0xE8814FF5,
		SRU_Plant_Health = 0xC3479CD9,
		SRU_Gromp = 0xD1C19B7E,
		SRU_Krug = 0xA20205E2,
		SRU_Murkwolf = 0x621947DC,
		SRU_Razorbeak = 0x244E9932,
		SRU_Blue = 0xFC505223,
		SRU_Red = 0x9CA35508,
		SRU_Dragon_Water = 0x27F996F4,
		SRU_Dragon_Fire = 0x99A947D9,
		SRU_Dragon_Earth = 0x606DCD87,
		SRU_Dragon_Air = 0x11D34E07,
		SRU_Dragon_Elder = 0x5944E907,
		SRU_RiftHerald = 0xDDAF53D2,
		SRU_Baron = 0x68AC12C9,
		Sru_Crab = 0x2DB77AF9,
		HA_ChaosMinionMelee = 0xD1BBDB04,
		HA_ChaosMinionRanged = 0x35751117,
		HA_ChaosMinionSiege = 0x1FF0EBFD,
		HA_ChaosMinionSuper = 0x53AB5F0B,
		HA_OrderMinionMelee = 0xEA51B9F6,
		HA_OrderMinionRanged = 0x2148EEA5,
		HA_OrderMinionSiege = 0x3886CAEF,
		HA_OrderMinionSuper = 0x6C413DFD,
		TT_NGolem = 0x42D14DDD,
		TT_NWolf = 0x31416281,
		TT_NWraith = 0xF01EA246,
		TT_SpiderBoss = 0x469FC571,
		AnnieTibbers = 0x51DAE4D4,
		EliseSpiderling = 0xD2607B4F,
		HeimerTYellow = 0xEA00055A,
		HeimerTBlue = 0xD473C020,
		IvernMinion = 0x3F0EF4CA,
		MalzaharVoidling = 0x6C78284,
		ShacoBox = 0x4AC2E173,
		YorickGhoulMelee = 0x5783A104,
		YorickBigGhoul = 0x2B12437A,
		ZyraThornPlant = 0x34B09A22,
		ZyraGraspingPlant = 0x71DC5EAE,
		GangplankBarrel = 0x7B575CB5,
		VoidSpawn = 0x91E07467,
		VoidSpawnTracer = 0x7ED0F974,
		JammerDevice = 0x8223B6BA,
		SightWard = 0x7C1BCAD9,
		YellowTrinket = 0x40D7E043,
		BlueTrinket = 0xE20532FD,
	};

	class CharacterData
	{
	public:
		union
		{
			DEFINE_MEMBER_N(const char* SkinName, Offsets::CharacterData::SkinName)
			DEFINE_MEMBER_N(Character SkinHash, Offsets::CharacterData::SkinHash)
			DEFINE_MEMBER_N(CharacterProperties* Properties, Offsets::CharacterData::Properties)
		};
	};
}