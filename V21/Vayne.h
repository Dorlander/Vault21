#pragma once
#include "ObjectManager.h"
#include "TargetSelector.h"
#include "Orbwalker.h"
#include "HudManager.h"
#include "EventManager.h"
#include "ClockFacade.h"
#include "Renderer.h"
#include "Functions.h"
#include "Menu.h"

#define PI 3.14159265f

namespace V21 {
	namespace Plugins {
		namespace Vayne {
			void Initialize();
			void Dispose();
			void OnGameUpdate();
			void OnProcessSpell(SpellInfo* castInfo, SpellDataResource* spellData);
			void OnGapCloserSpell(SpellInfo* castInfo, SpellDataResource* spellData);
			void OnInterruptibleSpell(SpellInfo* castInfo, SpellDataResource* spellData);
			bool IsCondemnable(GameObject* target);

			extern bool LastAttackCastSpell;
		}
	}
}