#include "stdafx.h"
#include "PluginTemplate.h"
#include "NavGrid.h"
#include "Draw.h"
#include "Geometry.h"

namespace HACKUZAN {
	namespace Plugins {

		

		using namespace HACKUZAN::SDK;
		using namespace HACKUZAN::SDK::Orbwalker;
	
		namespace ChampionConfig {

			namespace ChampionConfig {
				CheckBox* UseQ;
				CheckBox* UseW;
				Slider* WmaNa;
				CheckBox* UseE;
				Slider* EmaNa;
				CheckBox* UseR;
				Slider* enemiesInRange;
			}

			namespace ChampionFarm {
				CheckBox* UseQ;
				CheckBox* UseE;
				Slider* QmaNa;
				Slider* EmaNa;
			}

			namespace ChampionMisc {
				CheckBox* AutoE;
				CheckBox* AutoCatch;
				CheckBox* switchCatch;
				CheckBox* UseQforW;
				CheckBox* CatchUnderTurret;
				CheckBox* DrawAxe;
				Slider* DravenAxePickRange;
			}
		}

		void HACKUZAN::Plugins::ChampionName::Initialize()
		{
			auto menu = Menu::CreateMenu("Draven", "Draven");

			auto combo = menu->AddMenu("Combo", "Combo Settings");
			
			auto farm = menu->AddMenu("farm", "Farm Settings");
			

			auto misc = menu->AddMenu("misc", "Misc");


			EventManager::AddEventHandler(LeagueEvents::OnIssueOrder, OnIssueOrder);
			EventManager::AddEventHandler(LeagueEvents::OnPresent, OnGameUpdate);
			EventManager::AddEventHandler(LeagueEvents::OnCreateObject, OnCreateObject);
			EventManager::AddEventHandler(LeagueEvents::OnDeleteObject, OnDeleteObject);
			EventManager::AddEventHandler(LeagueEvents::OnProcessSpell, OnProcessSpell);
			EventManager::AddEventHandler(LeagueEvents::OnPresent, OnDraw);

			GameClient::PrintChat("Draven Script Loaded~!", IM_COL32(255, 69, 255, 255));
		}

		void ChampionName::Dispose()
		{
			EventManager::RemoveEventHandler(LeagueEvents::OnIssueOrder, OnIssueOrder);
			EventManager::RemoveEventHandler(LeagueEvents::OnPresent, OnGameUpdate);
			EventManager::RemoveEventHandler(LeagueEvents::OnCreateObject, OnCreateObject);
			EventManager::RemoveEventHandler(LeagueEvents::OnDeleteObject, OnDeleteObject);
			EventManager::RemoveEventHandler(LeagueEvents::OnProcessSpell, OnProcessSpell);
			EventManager::RemoveEventHandler(LeagueEvents::OnPresent, OnDraw);
		}

		bool ChampionName::OnIssueOrder(GameObject* unit, GameObjectOrder order, Vector3 position) {
			if (unit == ObjectManager::Player)
			{
				if (Orbwalker::DisableNextMove && order == GameObjectOrder::MoveTo) {

					return false;
				}
			}
			return  true;
		}

		void ChampionName::OnCreateObject(GameObject* unit)
		{

		}

		void ChampionName::OnDeleteObject(GameObject* unit)
		{
			if (unit == nullptr)
				return;


		}

		void ChampionName::OnDraw()
		{

		}

		void ChampionName::OnGameUpdate()
		{
			auto target = GetTarget();

			if (!Orbwalker::OrbwalkerEvading) {

				if (ActiveMode != OrbwalkerMode_None)
				{

				}
			}

			if (ActiveMode & OrbwalkerMode_Combo) {

				if (target && Orbwalker::CanCastAfterAttack()) {


				}
			}

			if (ActiveMode != OrbwalkerMode_None) {

				if (Orbwalker::CanCastAfterAttack()) {

				}
			}
		}


		GameObject* ChampionName::GetTarget()
		{
			std::vector<GameObject*> heroes;
			auto hero_list = HACKUZAN::GameObject::GetHeroes();
			for (size_t i = 0; i < hero_list->size; i++)
			{
				auto hero = hero_list->entities[i];

				if (hero && hero->IsEnemy() && hero->IsValidTarget(1000)) {
					heroes.push_back(hero);
				}
			}
			return TargetSelector::GetTarget(heroes, DamageType_Physical);
		}

	}
}