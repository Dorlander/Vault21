#include "CallBackIntStates.h"
#include "stdafx.h"

#include "skin_changer.hpp"
#include "skin_database.hpp"

#include "json.hpp"
#include "ExampleAppLog.h"
#include "LeagueHooks.h"
#include "HeavensGateHook.h"
#include "Orbwalker.h"
#include "Globals.h"
#include <StaticLists.h>

CObjectManager* ObjManager;
CFunctions Functions;

ExampleAppLog AppLog;

std::list<CObject*> heroList = {};
std::list<CObject*> minionList = {};

namespace DX11
{
	using json = nlohmann::json;

	int32_t config::current_combo_skin_index = 0;
	int32_t config::current_combo_ward_index = 0;
	int32_t config::current_combo_minion_index = 0;

	int32_t config::current_ward_skin_index = -1;
	int32_t config::current_minion_skin_index = -1;

	std::map<uint32_t, int32_t> config::current_combo_jungle_mob_skin_index;

	std::map<uint32_t, int32_t> config::current_combo_ally_skin_index;
	std::map<uint32_t, int32_t> config::current_combo_enemy_skin_index;
	auto config_json = json();

	void config::save()
	{
		auto player = me;
		if (player)
			config_json[std::string(player->get_character_data_stack()->base_skin.model.str) +
			".current_combo_skin_index"] = current_combo_skin_index;

		config_json["current_combo_ward_index"] = current_combo_ward_index;
		config_json["current_combo_minion_index"] = current_combo_minion_index;
		config_json["current_ward_skin_index"] = current_ward_skin_index;
		config_json["current_minion_skin_index"] = current_minion_skin_index;

		for (auto& it : current_combo_ally_skin_index)
			config_json["current_combo_ally_skin_index"][std::to_string(it.first)] = it.second;

		for (auto& it : current_combo_enemy_skin_index)
			config_json["current_combo_enemy_skin_index"][std::to_string(it.first)] = it.second;

		for (auto& it : current_combo_jungle_mob_skin_index)
			config_json["current_combo_jungle_mob_skin_index"][std::to_string(it.first)] = it.second;

		auto out = std::ofstream(L"league_changer.json");
		out << config_json.dump();
		out.close();
	}

	void config::load()
	{
		auto out = std::ifstream(L"league_changer.json");
		if (!out.good())
			return;

		config_json = json::parse(out);

		auto player = me;
		if (player)
			current_combo_skin_index = config_json.value(
				std::string(player->get_character_data_stack()->base_skin.model.str) + ".current_combo_skin_index", 0);

		current_combo_ward_index = config_json.value("current_combo_ward_index", 0);
		current_combo_minion_index = config_json.value("current_combo_minion_index", 0);
		current_ward_skin_index = config_json.value("current_ward_skin_index", -1);
		current_minion_skin_index = config_json.value("current_minion_skin_index", -1);

		auto ally_skins = config_json.find("current_combo_ally_skin_index");
		if (ally_skins != config_json.end())
			for (auto& it : ally_skins.value().items())
				current_combo_ally_skin_index[std::stoul(it.key())] = it.value().get<int32_t>();

		auto enemy_skins = config_json.find("current_combo_enemy_skin_index");
		if (enemy_skins != config_json.end())
			for (auto& it : enemy_skins.value().items())
				current_combo_enemy_skin_index[std::stoul(it.key())] = it.value().get<int32_t>();

		auto jungle_mobs_skins = config_json.find("current_combo_jungle_mob_skin_index");
		if (jungle_mobs_skins != config_json.end())
			for (auto& it : jungle_mobs_skins.value().items())
				current_combo_jungle_mob_skin_index[std::stoul(it.key())] = it.value().get<int32_t>();

		out.close();
	}

	void config::reset()
	{
		current_combo_skin_index = 0;
		current_combo_ward_index = 0;
		current_combo_minion_index = 0;

		current_ward_skin_index = -1;
		current_minion_skin_index = -1;

		current_combo_ally_skin_index.clear();
		current_combo_enemy_skin_index.clear();
		current_combo_jungle_mob_skin_index.clear();
	}
}

bool doneHookTimerAllowances = false;


namespace DX11
{
	bool Menu::bIsOpen = true;
	static bool onInit = false;


	static void HelpMarker(const char* desc)
	{
		TextDisabled("(?)");
		if (IsItemHovered())
		{
			BeginTooltip();
			PushTextWrapPos(GetFontSize() * 35.0f);
			TextUnformatted(desc);
			PopTextWrapPos();
			EndTooltip();
		}
	}

	char str_buffer[256];

	void Menu::Content()
	{
		if (isMainThreadAllow && !onInit)
		{
			config::load();
			skin_database::load();
			zoomValue = *Engine::GetMaximumZoomAmount();
			HiddenBase = FindHiddenModule();
			hiddenBase_debug = hexify<DWORD>(HiddenBase);

			Base = baseAddr;
			base_debug = hexify<DWORD>(Base);
			AppLog.AddLog("Init Success\n");
			onInit = true;
		}

		if (bIsOpen)
		{
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;

			SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(800, 800));
			Begin("SH3N##1", &Menu::bIsOpen, window_flags);

			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (BeginTabBar("Tabs", tab_bar_flags))
			{
				if (BeginTabItem("Awareness"))
				{
					TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Visuals");
					Separator();
					Columns(2, "visualscolumns", false); // 2-ways, with border
					Checkbox("Champ Name", &g_champ_name);
					SameLine();
					HelpMarker("Display champion name of all hero.");
					Checkbox("Self Range", &g_draw_lp_range);
					SameLine();
					HelpMarker("Display circle of your hero range.");
					Checkbox("Ally Range", &g_draw_ally_range);
					SameLine();
					HelpMarker("Display circle of all ally heroes range.");
					Checkbox("Enemy Range", &g_draw_enemy_range);
					SameLine();
					HelpMarker("Display circle of all enemy heroes range.");
					Checkbox("Enemy Turret Range", &g_enemy_turret);
					SameLine();
					HelpMarker("Display circle of all enemy turret range.");
					NextColumn();
					Checkbox("Enemy Wards", &g_draw_wards);
					SameLine();
					HelpMarker("Display circle of all enemy wards range.");
					Checkbox("Enemy Line", &g_draw_line);
					SameLine();
					HelpMarker("Display line of all enemy heroes position to your position.");
					Checkbox("Enemy Missles", &g_drawEnemyMissles);
					SameLine();
					HelpMarker("Display all enemy spell missles.");
					Checkbox("Spell Textures", &g_display_spell_texture);
					SameLine();
					HelpMarker("Display all enemy spell textures.");
					Columns(1);
					Separator();
					Spacing();

					TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Timers");
					Separator();
					Columns(2, "timerscolumns", false); // 2-ways, with border
					Checkbox("Spells Timer", &g_draw_cd_tracker);
					SameLine();
					HelpMarker("Display spells timer of all heroes.");
					Checkbox("Inhi Respawn Timer", &g_inhi_respawn);
					SameLine();
					HelpMarker("Display respawn timer of inhibitors.");
					NextColumn();
					Checkbox("Jg Camp Time", &g_campTimer);
					SameLine();
					HelpMarker("Display respawn time of all major jungle objectives.");
					Columns(1);
					const char* opt_flashTimer[] = { "Off", "1 Liner", "Multi-Liner", "Full", "Full /all" };
					Combo("Spells Message", &opt_flashTimer_c, opt_flashTimer, IM_ARRAYSIZE(opt_flashTimer));
					SameLine();
					HelpMarker(
						"Print (X) or Chat (J) the enemy spells. 1 Liner - Short timestamp. Multi-liner - Semi-complete details. Full - Complete details.");
					Columns(1);
					Separator();
					Spacing();

					TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Functions");
					Separator();
					Checkbox("Auto Smite", &g_autoSmite);
					SameLine();
					HelpMarker("Auto smite the major jungle objectives.");
					const char* opt_autoCleanse[] = {
						"Off", "100%", "90%", "80%", "70%", "60%", "50%", "40%", "30%", "20%", "10%"
					};
					Combo("Auto Cleanse", &opt_autoCleanse_c, opt_autoCleanse, IM_ARRAYSIZE(opt_autoCleanse));
					SameLine();
					HelpMarker("Auto cleanse when your hero health is below the specified option.");

					SliderInt("Camera Max Zoom", &zoomValue, 1001, 4499, "%d", 0);

					EndTabItem();
				}

				if (BeginTabItem("Orbwalker"))
				{
					TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Orbwalker");
					SliderFloat("extra Windup", &g_orbwalker_windup, 0.f, 120.f, "%.1f");
					EndTabItem();
				}

				if (isMainThreadAllow)
				{
					if (BeginTabItem("Hooks"))
					{
						if (doneHookTimerAllowances)
						{
							TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OnProcessSpell");
							Separator();
							Checkbox("Enable OnProcessSpell", &g_onprocessspell);
							SameLine();
							HelpMarker("Hook OnProcessSpell.");
							Separator();
							if (g_onprocessspell)
							{
								Separator();
								Spacing();
							}

							TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OnCreateObject");
							Separator();
							Checkbox("Enable OnCreateObject", &g_oncreateobject);
							SameLine();
							HelpMarker("Hook OnCreateObject.");
							Separator();
							if (g_oncreateobject)
							{
								Separator();
								Spacing();
							}

							TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OnDeleteObject");
							Separator();
							Checkbox("Enable OnDeleteObject", &g_ondeleteobject);
							SameLine();
							HelpMarker("Hook OnDeleteObject.");
							Separator();
							if (g_ondeleteobject)
							{
								Separator();
								Spacing();
							}

							TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "OnNewPath");
							Separator();
							Checkbox("Enable OnNewPath", &g_onnewpath);
							SameLine();
							HelpMarker("Hook OnNewPath.");
							Separator();
							if (g_onnewpath)
							{
								Separator();
								Spacing();
							}
						}
						else
						{
							Text("Hooks not yet ready. Check logs.");
						}

						EndTabItem();
					}

					if (BeginTabItem("Skin Changer"))
					{
						static auto vector_getter_skin = [](void* vec, int idx, const char** out_text)
						{
							auto& vector = *static_cast<std::vector<skin_database::skin_info>*>(vec);
							if (idx < 0 || idx > static_cast<int>(vector.size())) { return false; }
							*out_text = idx == 0 ? "Don't change" : vector.at(idx - 1).skin_name.c_str();
							return true;
						};

						static auto vector_getter_ward_skin = [](void* vec, int idx, const char** out_text)
						{
							auto& vector = *static_cast<std::vector<std::pair<int32_t, std::string>>*>(vec);
							if (idx < 0 || idx > static_cast<int>(vector.size())) { return false; }
							*out_text = idx == 0 ? "Don't change" : vector.at(idx - 1).second.c_str();
							return true;
						};

						static auto vector_getter_default = [](void* vec, int idx, const char** out_text)
						{
							auto& vector = *static_cast<std::vector<std::string>*>(vec);
							if (idx < 0 || idx > static_cast<int>(vector.size())) { return false; }
							*out_text = idx == 0 ? "Don't change" : vector.at(idx - 1).c_str();
							return true;
						};

						auto player = me;
						if (player)
						{
							auto& values = skin_database::champions_skins[fnv::hash_runtime(
								me->get_character_data_stack()->base_skin.model.str)];

							TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Localplayer skins settings:");
							Separator();
							if (Combo("Current skin", &config::current_combo_skin_index, vector_getter_skin,
								static_cast<void*>(&values), values.size() + 1))
								if (config::current_combo_skin_index > 0)
								{
									me->change_skin(values[config::current_combo_skin_index - 1].model_name.c_str(),
										values[config::current_combo_skin_index - 1].skin_id);
								}

							if (Combo("Current ward skin", &config::current_combo_ward_index, vector_getter_ward_skin,
								static_cast<void*>(&skin_database::wards_skins),
								skin_database::wards_skins.size() + 1))
								config::current_ward_skin_index = config::current_combo_ward_index == 0
								? -1
								: skin_database::wards_skins.at(
									config::current_combo_ward_index - 1).first;

							Separator();
							Spacing();
						}

						TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Global skins settings:");
						Separator();
						if (Combo("Current minion skin", &config::current_combo_minion_index, vector_getter_default,
							static_cast<void*>(&skin_database::minions_skins),
							skin_database::minions_skins.size() + 1))
							config::current_minion_skin_index = config::current_combo_minion_index - 1;
						Separator();
						Spacing();

						TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Jungle mobs skins settings:");
						Separator();
						for (auto& it : skin_database::jungle_mobs_skins)
						{
							snprintf(str_buffer, 256, "Current %s skin", it.name.c_str());
							auto config_entry = config::current_combo_jungle_mob_skin_index.insert({
								it.name_hashes.front(), 0
								});

							if (Combo(str_buffer, &config_entry.first->second, vector_getter_default,
								static_cast<void*>(&it.skins), it.skins.size() + 1))
							{
								for (auto& hash : it.name_hashes)
									config::current_combo_jungle_mob_skin_index[hash] = config_entry.first->second;
							}
						}

						Separator();
						Spacing();

						auto my_team = player ? player->GetTeam() : 100;

						TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Other champions skins settings:");
						Separator();
						auto last_team = static_cast<int32_t>(0);

						for (CObject* a : heroList)
						{
							auto hero = a;
							if (hero == player)
								continue;

							auto hero_team = hero->GetTeam();
							auto is_enemy = hero_team != my_team;

							if (last_team == 0 || hero_team != last_team)
							{
								if (last_team != 0)
									Separator();
								if (!is_enemy)
									Text("Ally champions");
								else
									Text("Enemy champions");
								last_team = hero_team;
							}

							auto& config_array = is_enemy
								? config::current_combo_enemy_skin_index
								: config::current_combo_ally_skin_index;

							auto champion_name_hash = fnv::hash_runtime(
								hero->get_character_data_stack()->base_skin.model.str);
							auto config_entry = config_array.insert({ champion_name_hash, 0 });

							std::string charName = std::string(hero->GetName());
							snprintf(str_buffer, 256, "Current skin (%s)##%X", charName.c_str(),
								reinterpret_cast<uintptr_t>(hero));

							auto& values = skin_database::champions_skins[champion_name_hash];
							if (Combo(str_buffer, &config_entry.first->second, vector_getter_skin,
								static_cast<void*>(&values), values.size() + 1))
								if (config_entry.first->second > 0)
								{
									hero->change_skin(values[config_entry.first->second - 1].model_name.c_str(),
										values[config_entry.first->second - 1].skin_id);
								}
						}

						EndTabItem();
					}
				}
				if (BeginTabItem("Information"))
				{
					TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Information");
					Separator();
					Text("LoL Ver. %s", TARGET_GAMEVERSION);

					if (isMainThreadAllow)
					{
						std::string gameVer = "";
						char* gameVerChar = Engine::GetGameVersion();
						std::string _gameVer(gameVerChar);
						gameVer = _gameVer;

						std::string targetGameVer = "";
						std::string _targetGameVer(TARGET_GAMEVERSION);
						targetGameVer = _targetGameVer;

						int compare = gameVer.compare(targetGameVer);

						if (compare != 0)
						{
							Text("H4X Ver. %s", gameVer);
							Text("VERSION DOES NOT MATCH!!! PLEASE UPDATE THE H4X!!!");
						}
					}

					Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / GetIO().Framerate,
						GetIO().Framerate);
					Separator();
					Spacing();

					TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Credits");
					BulletText("pakeke80");
					BulletText("B3akers - for LeagueSkinChanger");
					BulletText("and EVERYONE in UC forum");
					BulletText("Project of pakeke80, Dencelle, HACKUZAN, MyNameIsEarl, Jiingz");
					EndTabItem();
				}
				if (BeginTabItem("Debug"))
				{
					TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "===DEBUG===");
					Separator();
					Text("Hidden Module %s", &hiddenBase_debug);
					Text("Module %s", &base_debug);

					Separator();

					Checkbox("Show Obj Address", &g_debug_address);
					SameLine();
					HelpMarker("Display object's address.");
					Checkbox("Show Obj Name", &g_debug_name);
					SameLine();
					HelpMarker("Display object's name.");
					Checkbox("Show Obj Location", &g_debug_location);
					SameLine();
					HelpMarker("Display object's location.");
					Separator();
					Text("IsChatBoxOpen %s", &IsChatOpen_debug);
					Text("Health %s%", &me_healthPercentage_debug);
					Text("CastSpell Ctr. %s", &castSpellCtr_debug);
					Text("IsImmobile %s", &me_isImmobile_debug);
					Text("D %s", &d_spellName_debug);
					Text("D CD %s", &d_spellCD_debug);
					Text("D IsDone %s", &d_spellIsDone_debug);
					Text("D Chrg. %s", &d_spellChrg_debug);
					Text("D Dmg. %s", &d_spellDmg_debug);
					Text("D CDAbs %s", &d_spellCDAbs_debug);
					Text("D IsDoneAbs %s", &d_spellIsDoneAbs_debug);
					Text("F %s", &f_spellName_debug);
					Text("F CD %s", &f_spellCD_debug);
					Text("F IsDone %s", &f_spellIsDone_debug);
					Text("F Chrg. %s", &f_spellChrg_debug);
					Text("D Dmg. %s", &f_spellDmg_debug);
					Text("F CDAbs %s", &f_spellCDAbs_debug);
					Text("F IsDoneAbs %s", &f_spellIsDoneAbs_debug);

					EndTabItem();
				}
				if (BeginTabItem("Console Log"))
				{
					AppLog.Draw();

					EndTabItem();
				}
				EndTabBar();
			}

			End();
		}


		if (isMainThreadAllow)
		{
			if (Engine::IsChatBoxOpen())
			{
				IsChatBoxOpen = true;
				IsChatOpen_debug = "true";
			}
			else
			{
				IsChatBoxOpen = false;
				IsChatOpen_debug = "false";
			}

			if (zoomValue > 1000 && zoomValue < 4500 && zoomValueLast != zoomValue)
			{
				zoomValueLast = zoomValue;
				Engine::ChangeMaximumZoom(zoomValue);
			}

			auto me_index = me->GetIndex();

			auto me_pos = me->GetPos();
			auto me_IsOnScreen = me->IsOnScreen();

			if (!finishedOnNewPathInit)
			{
				if (me_IsOnScreen)
				{
					if (me_lastPos.X != 0 ||
						me_lastPos.Y != 0 ||
						me_lastPos.Z != 0)
					{
						if (me_pos.X != me_lastPos.X ||
							me_pos.Y != me_lastPos.Y ||
							me_pos.Z != me_lastPos.Z)
						{
							finishedOnNewPathInit = true;
						}
					}

					me_lastPos = me_pos;
				}
			}

			auto me_IsAlive = me->IsAlive();

			auto gameTime = Engine::GetGameTime();

			auto me_d_spellSlot = me->GetSpellSlotByID(4);
			auto me_f_spellSlot = me->GetSpellSlotByID(5);

			auto me_d_spellName = me_d_spellSlot->GetSpellInfo()->GetSpellData()->GetSpellName2();
			auto me_f_spellName = me_f_spellSlot->GetSpellInfo()->GetSpellData()->GetSpellName2();

			std::string me_d_spellName_str(me_d_spellName);
			transform(me_d_spellName_str.begin(), me_d_spellName_str.end(), me_d_spellName_str.begin(), ::tolower);

			std::string me_f_spellName_str(me_f_spellName);
			std::transform(me_f_spellName_str.begin(), me_f_spellName_str.end(), me_f_spellName_str.begin(), ::tolower);

			bool me_SmiteFound = false;
			bool me_isDoneCDSmite = false;
			int me_smiteSpellSlot = 0;
			float me_smiteDamage = 0;


			d_spellName_debug = me_d_spellName_str;
			f_spellName_debug = me_f_spellName_str;

			d_spellCD_debug = std::to_string(me_d_spellSlot->GetRemainingCD(gameTime));
			d_spellIsDone_debug = (me_d_spellSlot->IsDoneCD(gameTime) ? "True" : "False");

			f_spellCD_debug = std::to_string(me_f_spellSlot->GetRemainingCD(gameTime));
			f_spellIsDone_debug = (me_f_spellSlot->IsDoneCD(gameTime) ? "True" : "False");


			if (me_d_spellName_str.find("smite") != std::string::npos)
			{
				auto me_d_doneCD = me_d_spellSlot->IsDoneAbsoluteCD(gameTime);

				me_SmiteFound = true;
				me_isDoneCDSmite = me_d_doneCD;
				me_smiteSpellSlot = 4;
				me_smiteDamage = me_d_spellSlot->GetDamage();

				d_spellChrg_debug = std::to_string(me_d_spellSlot->GetRemainingCharge());

				d_spellDmg_debug = std::to_string(me_d_spellSlot->GetDamage());

				d_spellCDAbs_debug = std::to_string(me_d_spellSlot->GetAbsoluteCoolDown(gameTime));
				d_spellIsDoneAbs_debug = (me_d_spellSlot->IsDoneAbsoluteCD(gameTime) ? "True" : "False");
			}
			else if (me_f_spellName_str.find("smite") != std::string::npos)
			{
				auto me_f_doneCD = me_f_spellSlot->IsDoneAbsoluteCD(gameTime);

				me_SmiteFound = true;
				me_isDoneCDSmite = me_f_doneCD;
				me_smiteSpellSlot = 5;
				me_smiteDamage = me_f_spellSlot->GetDamage();

				f_spellChrg_debug = std::to_string(me_f_spellSlot->GetRemainingCharge());

				f_spellDmg_debug = std::to_string(me_f_spellSlot->GetDamage());

				f_spellCDAbs_debug = std::to_string(me_f_spellSlot->GetAbsoluteCoolDown(gameTime));
				f_spellIsDoneAbs_debug = (me_f_spellSlot->IsDoneAbsoluteCD(gameTime) ? "True" : "False");
			}

			bool me_CleanseFound = false;
			bool me_isDoneCDCleanse = false;
			int me_cleanseSpellSlot = 0;

			if (me_d_spellName_str.find("boost") != std::string::npos)
			{
				auto me_d_doneCD = me_d_spellSlot->IsDoneCD(gameTime);

				me_CleanseFound = true;
				me_isDoneCDCleanse = me_d_doneCD;
				me_cleanseSpellSlot = 4;

				d_spellChrg_debug = std::to_string(me_d_spellSlot->GetRemainingCharge());
				d_spellCDAbs_debug = std::to_string(me_d_spellSlot->GetAbsoluteCoolDown(gameTime));
				d_spellIsDoneAbs_debug = (me_d_spellSlot->IsDoneAbsoluteCD(gameTime) ? "True" : "False");
			}
			else if (me_f_spellName_str.find("boost") != std::string::npos)
			{
				auto me_f_doneCD = me_f_spellSlot->IsDoneCD(gameTime);

				me_CleanseFound = true;
				me_isDoneCDCleanse = me_f_doneCD;
				me_cleanseSpellSlot = 5;

				f_spellChrg_debug = std::to_string(me_f_spellSlot->GetRemainingCharge());
				f_spellCDAbs_debug = std::to_string(me_f_spellSlot->GetAbsoluteCoolDown(gameTime));
				f_spellIsDoneAbs_debug = (me_f_spellSlot->IsDoneAbsoluteCD(gameTime) ? "True" : "False");
			}

			if (g_draw_lp_range)
			{
				if (me && me->IsAlive())
				{
					auto me_attackRange = me->GetAttackRange();
					auto me_boundingRadius = me->GetBoundingRadius();

					if (me_IsAlive)
					{
						auto color = createRGB(0, 255, 0); // green
						Engine::DrawCircle(&me_pos, me_attackRange + me_boundingRadius, &color, 0, 0.0f, 0, 0.5f);
						//render.draw_circle(me_pos, me_attackRange + me_boundingRadius, color, c_renderer::circle_3d, 50, 0.5f);
					}
				}
			}

			if (opt_autoCleanse_c != 0)
			{
				if (me)
				{
					if (me->GetBuffMgr()->IsImmobile2())
					{
						me_isImmobile_debug = "True";
					}
					else
					{
						me_isImmobile_debug = "False";
					}

					if (me_IsAlive && me_CleanseFound && me_isDoneCDCleanse)
					{
						auto me_currHealth = me->GetHealth();
						auto me_maxHealth = me->GetMaxHealth();

						auto me_healthPercentage = (me_currHealth / me_maxHealth) * 100.0f;
						me_healthPercentage_debug = std::to_string(me_healthPercentage);

						auto me_IsImmobile = me->GetBuffMgr()->IsImmobile2();

						if (me_IsImmobile && (
							(me_healthPercentage <= 100.0f && opt_autoCleanse_c == 1) ||
							(me_healthPercentage <= 90.0f && opt_autoCleanse_c == 2) ||
							(me_healthPercentage <= 80.0f && opt_autoCleanse_c == 3) ||
							(me_healthPercentage <= 70.0f && opt_autoCleanse_c == 4) ||
							(me_healthPercentage <= 60.0f && opt_autoCleanse_c == 5) ||
							(me_healthPercentage <= 50.0f && opt_autoCleanse_c == 6) ||
							(me_healthPercentage <= 40.0f && opt_autoCleanse_c == 7) ||
							(me_healthPercentage <= 30.0f && opt_autoCleanse_c == 8) ||
							(me_healthPercentage <= 20.0f && opt_autoCleanse_c == 9) ||
							(me_healthPercentage <= 10.0f && opt_autoCleanse_c == 10)
							)
							)
						{
							Engine::CastSpellSelf(me_cleanseSpellSlot);
							CastSpellCtr++;
							castSpellCtr_debug = std::to_string(CastSpellCtr);
						}
					}
				}
			}


			CObject holzer;
			auto obj = holzer.GetFirstObject();

			std::string msgString = "";

			if (opt_flashTimer_c != 0)
			{
				if ((GetAsyncKeyState(0x4A) & 0x8000) && (j_key_flag == 0))
				{
					j_key_flag = 1;
					is_j_key_ready = true;
					//isOneTime = true;
				}
				else if (GetAsyncKeyState(0x4A) == 0)
				{
					j_key_flag = 0; //reset the flag
				}

				if ((GetAsyncKeyState(0x58) & 0x8000) && (x_key_flag == 0))
				{
					x_key_flag = 1;
					is_x_key_ready = true;
				}
				else if (GetAsyncKeyState(0x58) == 0)
				{
					x_key_flag = 0; //reset the flag
				}
			}
			if (g_campTimer)
			{
				if ((GetAsyncKeyState(0x55) & 0x8000) && (u_key_flag == 0))
				{
					u_key_flag = 1;
					is_u_key_ready = true;
				}
				else if (GetAsyncKeyState(0x55) == 0)
				{
					u_key_flag = 0; //reset the flag
				}
			}

			if (IsChatBoxOpen)
			{
				is_j_key_ready = false;
				is_x_key_ready = false;
				is_u_key_ready = false;
			}

			std::list<CObject*> _heroList = {};
			std::list<CObject*> _minionList = {};

			int objCount = 0;

			while (obj)
			{
				objCount++;

				auto IsHero = obj->IsHero();
				auto IsMinion = obj->IsMinion();
				auto IsInhibitor = obj->IsInhibitor();
				auto IsTurret = obj->IsTurret();
				auto Index = obj->GetIndex();
				auto IsEnemyToLocalPlayer = obj->IsEnemyTo(me);
				auto Pos = obj->GetPos();
				auto Name = obj->GetName();
				auto IsOnScreen = obj->IsOnScreen();
				auto IsTeammate = obj->IsTeammateTo(me);
				auto IsMissle = obj->IsMissile();

				auto Parent = obj->GetParent(heroList);

				std::string Name_str(Name);
				transform(Name_str.begin(), Name_str.end(), Name_str.begin(), ::tolower);

				Vector objpos_w2s;
				Functions.WorldToScreen(&Pos, &objpos_w2s);

				ImColor _normal = ImColor(255, 102, 102, 174);
				ImColor _onCD = ImColor(102, 255, 102, 174);

				if (g_debug_address)
				{
					if (obj->IsHero())
						render.draw_text(objpos_w2s.X, objpos_w2s.Y,
							("Addr: " + (hexify<DWORD>(obj->GetThis()))).c_str(), true, _onCD);
				}
				if (g_debug_name)
				{
					render.draw_text(objpos_w2s.X, objpos_w2s.Y + 15, ("Name: " + Name_str).c_str(), true, _onCD);
				}
				if (g_debug_location)
				{
					render.draw_text(objpos_w2s.X, objpos_w2s.Y + 30,
						("X: " + std::to_string(Pos.X) + ", Y: " + std::to_string(Pos.Y) + ", Z: " +
							std::to_string(Pos.Z)).c_str(), true, _onCD);
				}


				if (g_draw_wards)
				{
					//if (IsTeammate) { // perks cant be determined if enemy side or ally side :(
					auto color = createRGB(204, 106, 255); // violet
					if (Name_str.find("perks_ghostporo_idle") != std::string::npos)
					{
						// ghost poro
						Engine::DrawCircle(&Pos, 450.0f, &color, 0, 0.0f, 0, 0.5f);
						//render.draw_circle(Pos, 450.0f, color, c_renderer::circle_3d, 50, 0.5f);
					}

					if (Name_str.find("perks_corruptedward_idle") != std::string::npos)
					{
						// zombieward
						Engine::DrawCircle(&Pos, 900.0f, &color, 0, 0.0f, 0, 0.5f);
						//render.draw_circle(Pos, 900.0f, color, c_renderer::circle_3d, 50, 0.5f);
					}
					//}
				}

				if (g_campTimer)
				{
					if ((Name_str.compare("monstercamp_1") == 0) ||
						(Name_str.compare("monstercamp_7") == 0) ||
						(Name_str.compare("monstercamp_4") == 0) ||
						(Name_str.compare("monstercamp_10") == 0) ||
						(Name_str.compare("monstercamp_6") == 0) ||
						(Name_str.compare("monstercamp_17") == 0) ||
						(Name_str.compare("monstercamp_12") == 0)
						)
					{
						float jungleSpawnOn = Engine::GetTimerExpiry(obj);
						float jungleSpawn = jungleSpawnOn - gameTime;

						std::string spawnTimer_str = "--:--";

						auto _color = _onCD;

						std::string monsterName = Name_str;
						if ((Name_str.compare("monstercamp_1") == 0) ||
							(Name_str.compare("monstercamp_7") == 0)
							)
						{
							monsterName = "Blue Sentinel";
						}
						else if ((Name_str.compare("monstercamp_4") == 0) ||
							(Name_str.compare("monstercamp_10") == 0)
							)
						{
							monsterName = "Red Brambleback";
						}
						else if ((Name_str.compare("monstercamp_6") == 0))
						{
							monsterName = "Drake";
						}
						else if ((Name_str.compare("monstercamp_17") == 0))
						{
							monsterName = "Rift Herald";
						}
						else if ((Name_str.compare("monstercamp_12") == 0))
						{
							monsterName = "Baron";
						}

						if (jungleSpawn > 0.0f && jungleSpawn < 86400.0f)
						{
							//86400 is one day
							{
								spawnTimer_str = Engine::SecondsToClock(static_cast<int>(jungleSpawn));
								_color = _normal;
							}
						}

						if (u_key_flag == 1 && is_u_key_ready)
						{
							if (me->GetTeam() == 100 && (Name_str.compare("monstercamp_7") == 0))
							{
								std::string msgString_camp = "Enemy Blue Sentinel >> " + (
									jungleSpawn > 0.0f && jungleSpawn < 86400.0f
									? std::to_string(static_cast<int>(jungleSpawn)) + "s"
									: spawnTimer_str);
								Engine::SendChat(msgString_camp.c_str());
							}
							if (me->GetTeam() == 100 && (Name_str.compare("monstercamp_10") == 0))
							{
								std::string msgString_camp = "Enemy Red Brambleback >> " + (
									jungleSpawn > 0.0f && jungleSpawn < 86400.0f
									? std::to_string(static_cast<int>(jungleSpawn)) + "s"
									: spawnTimer_str);
								Engine::SendChat(msgString_camp.c_str());
							}
							if (me->GetTeam() == 200 && (Name_str.compare("monstercamp_1") == 0))
							{
								std::string msgString_camp = "Enemy Blue Sentinel >> " + (
									jungleSpawn > 0.0f && jungleSpawn < 86400.0f
									? std::to_string(static_cast<int>(jungleSpawn)) + "s"
									: spawnTimer_str);
								Engine::SendChat(msgString_camp.c_str());
							}
							if (me->GetTeam() == 200 && (Name_str.compare("monstercamp_4") == 0))
							{
								std::string msgString_camp = "Enemy Red Brambleback >> " + (
									jungleSpawn > 0.0f && jungleSpawn < 86400.0f
									? std::to_string(static_cast<int>(jungleSpawn)) + "s"
									: spawnTimer_str);
								Engine::SendChat(msgString_camp.c_str());
							}
						}

						render.draw_text(objpos_w2s.X, objpos_w2s.Y, spawnTimer_str.c_str(), true, _color);
						render.draw_text(objpos_w2s.X, objpos_w2s.Y - 15, monsterName.c_str(), true, _color);
					}
				}

				//===================Missles Start===============================
				if (IsMissle)
				{
					if (g_drawEnemyMissles)
					{
						if ((me_pos.DistTo(Pos) >= 0.0f) && (me_pos.DistTo(Pos) <= 4000.0f))
						{
							if (Parent != nullptr)
							{
								if (Parent->IsHero() && Parent->IsEnemyTo(me))
								{
									auto spellStartPos = obj->GetSpellStartPos();
									auto spellEndPos = obj->GetSpellEndPos();

									Vector objspellstartpos_w2s;
									Functions.WorldToScreen(&spellStartPos, &objspellstartpos_w2s);

									Vector objspellendpos_w2s;
									Functions.WorldToScreen(&spellEndPos, &objspellendpos_w2s);

									auto spellWidth = obj->GetSpellCastInfo()->GetSpellData()->GetSpellWidth();

									ImColor _skillsShots = ImColor(255, 102, 102, 79);
									render.draw_line(objspellstartpos_w2s.X, objspellstartpos_w2s.Y,
										objspellendpos_w2s.X, objspellendpos_w2s.Y, _skillsShots,
										spellWidth);

									auto spellEffectRange = obj->GetSpellCastInfo()->GetSpellData()->
										GetSpellEffectRange();
									auto color = createRGB(220, 20, 60); // crimson
									Engine::DrawCircle(&Pos, spellEffectRange, &color, 0, 0.0f, 0, 0.5f);
									//render.draw_circle(Pos, spellEffectRange, color, c_renderer::circle_3d, 50, 0.5f);
								}
							}
						}
					}
				}
				//===================Missles End===============================

				//===================Turret Start===============================
				if (IsTurret)
				{
					if (g_enemy_turret)
					{
						if (IsOnScreen)
						{
							auto IsAlive = obj->IsAlive();
							if (IsAlive)
							{
								if (IsEnemyToLocalPlayer)
								{
									auto boundingRadius = obj->GetBoundingRadius();
									auto color = createRGB(220, 20, 60); // crimson
									Engine::DrawCircle(&Pos, 800.0f + boundingRadius, &color, 0, 0.0f, 0, 0.5f);
									//render.draw_circle(Pos, 800.0f + boundingRadius, color, c_renderer::circle_3d, 50, 0.5f);
								}
							}
						}
					}
				}
				//===================Turret End===============================

				//===================Inhib Start===============================
				if (IsInhibitor)
				{
					if (g_inhi_respawn)
					{
						if (IsOnScreen)
						{
							auto inhiRespawnTime = obj->GetInhiRemainingRespawnTime();
							if (inhiRespawnTime > 0)
							{
								std::string str_respawnTime = Engine::SecondsToClock(
									static_cast<int>(inhiRespawnTime));
								render.draw_text(objpos_w2s.X, objpos_w2s.Y, str_respawnTime.c_str(), true, _onCD);
							}
						}
					}
				}
				//===================Inhib End===============================

				//===================Minion Start===============================
				if (IsMinion)
				{
					_minionList.push_back(obj);
					if (IsEnemyToLocalPlayer)
					{
						if (g_draw_wards)
						{
							if (IsOnScreen)
							{
								Vector w2s;
								Functions.WorldToScreen(&obj->GetPos(), &w2s);

								auto color = createRGB(220, 20, 60); // crimson	
								if ( WardList.find(Name_str) != WardList.end())
								{
									Engine::DrawCircle(&Pos, 50.f, &color, 0, 0.0f, 0, 1.0f);
									Engine::DrawCircle(&Pos, WardList.at(Name_str), &color, 0, 0.0f, 0, 0.5f);
									//render.draw_circle(Pos, 900.0f, color, c_renderer::circle_3d, 50, 0.5f);
									render.draw_text(w2s.X, w2s.Y, obj->GetName(), false, ImColor(255, 255, 255, 255));
								}

							}
						}
					}

					if (g_autoSmite)
					{
						auto Health = obj->GetHealth();
						if (
							(Name_str.find("sru_dragon_fire") != std::string::npos) ||
							(Name_str.find("sru_dragon_earth") != std::string::npos) ||
							(Name_str.find("sru_dragon_water") != std::string::npos) ||
							(Name_str.find("sru_dragon_air") != std::string::npos) ||
							(Name_str.find("sru_dragon_elder") != std::string::npos) ||
							(Name_str.find("sru_baron") != std::string::npos) ||
							(Name_str.find("sru_riftherald") != std::string::npos)
							)
						{
							if (Health > 0)
							{
								if (me_IsAlive && (me_pos.DistTo(Pos) >= 0) && (me_pos.DistTo(Pos) <= 500))
								{
									if (me_SmiteFound && me_isDoneCDSmite)
									{
										if (obj->GetHealth() <= me_smiteDamage)
										{
											Engine::CastSpellTargetted(me_smiteSpellSlot, obj);
											CastSpellCtr++;
											castSpellCtr_debug = std::to_string(CastSpellCtr);
										}
									}
								}
							}
						}
					}
				}
				//===================Minion End===============================

				//===================Hero Start===============================
				if (IsHero)
				{
					_heroList.push_back(obj);

					auto r_spellSlot = obj->GetSpellSlotByID(3);
					auto r_RemainingCD = r_spellSlot->GetRemainingCD(gameTime);

					auto d_spellSlot = obj->GetSpellSlotByID(4);
					auto d_doneCD = d_spellSlot->IsDoneCD(gameTime);
					auto d_RemainingCD = d_spellSlot->GetRemainingCD(gameTime);

					auto f_spellSlot = obj->GetSpellSlotByID(5);
					auto f_doneCD = f_spellSlot->IsDoneCD(gameTime);
					auto f_RemainingCD = f_spellSlot->GetRemainingCD(gameTime);

					ImColor colorBlue = ImColor(102, 102, 255, 174);

					auto champName = obj->GetChampionName();

					std::string champName_str(champName);

					auto d_spellName = d_spellSlot->GetSpellInfo()->GetSpellData()->GetSpellName2();
					auto f_spellName = f_spellSlot->GetSpellInfo()->GetSpellData()->GetSpellName2();

					std::string d_spellName_str(d_spellName);
					transform(d_spellName_str.begin(), d_spellName_str.end(), d_spellName_str.begin(), ::tolower);

					std::string f_spellName_str(f_spellName);
					transform(f_spellName_str.begin(), f_spellName_str.end(), f_spellName_str.begin(), ::tolower);

					if (d_spellName_str.find("smite") != std::string::npos)
					{
						d_doneCD = d_spellSlot->IsDoneAbsoluteCD(gameTime);
						d_RemainingCD = d_spellSlot->GetAbsoluteCoolDown(gameTime);
					}
					else if (f_spellName_str.find("smite") != std::string::npos)
					{
						f_doneCD = f_spellSlot->IsDoneAbsoluteCD(gameTime);
						f_RemainingCD = f_spellSlot->GetAbsoluteCoolDown(gameTime);
					}

					if (g_champ_name)
					{
						if (IsOnScreen)
						{
							render.draw_text(objpos_w2s.X, objpos_w2s.Y, champName, true, colorBlue);
						}
					}

					if (g_draw_cd_tracker)
					{
						if (IsOnScreen && obj->IsAlive() && obj->IsVisible() && obj->IsEnemyTo(me))
						{
							auto q_spellSlot = obj->GetSpellSlotByID(0);
							auto q_RemainingCD = q_spellSlot->GetRemainingCD(gameTime);
							auto q_doneCD = q_spellSlot->IsDoneCD(gameTime);
							auto q_level = q_spellSlot->GetLevel();

							auto w_spellSlot = obj->GetSpellSlotByID(1);
							auto w_RemainingCD = w_spellSlot->GetRemainingCD(gameTime);
							auto w_doneCD = w_spellSlot->IsDoneCD(gameTime);
							auto w_level = w_spellSlot->GetLevel();

							auto e_spellSlot = obj->GetSpellSlotByID(2);
							auto e_RemainingCD = e_spellSlot->GetRemainingCD(gameTime);
							auto e_doneCD = e_spellSlot->IsDoneCD(gameTime);
							auto e_level = e_spellSlot->GetLevel();

							auto r_doneCD = r_spellSlot->IsDoneCD(gameTime);
							auto r_level = r_spellSlot->GetLevel();

							if (q_doneCD)
							{
								render.draw_text(objpos_w2s.X - 30, objpos_w2s.Y + 15, "Q", true, _normal);
							}
							else if (q_level >= 1)
							{
								char skill_q[10];
								snprintf(skill_q, sizeof(skill_q), "%d", static_cast<int>(q_RemainingCD));
								render.draw_text(objpos_w2s.X - 30, objpos_w2s.Y + 15, skill_q, true, _onCD);
							}

							if (w_doneCD)
							{
								render.draw_text(objpos_w2s.X - 10, objpos_w2s.Y + 15, "W", true, _normal);
							}
							else if (w_level >= 1)
							{
								char skill_w[10];
								snprintf(skill_w, sizeof(skill_w), "%d", static_cast<int>(w_RemainingCD));
								render.draw_text(objpos_w2s.X - 10, objpos_w2s.Y + 15, skill_w, true, _onCD);
							}

							if (e_doneCD)
							{
								render.draw_text(objpos_w2s.X + 10, objpos_w2s.Y + 15, "E", true, _normal);
							}
							else if (e_level >= 1)
							{
								char skill_e[10];
								snprintf(skill_e, sizeof(skill_e), "%d", static_cast<int>(e_RemainingCD));
								render.draw_text(objpos_w2s.X + 10, objpos_w2s.Y + 15, skill_e, true, _onCD);
							}

							if (r_doneCD)
							{
								render.draw_text(objpos_w2s.X + 30, objpos_w2s.Y + 15, "R", true, _normal);
							}
							else if (r_level >= 1)
							{
								char skill_r[10];
								snprintf(skill_r, sizeof(skill_r), "%d", static_cast<int>(r_RemainingCD));
								render.draw_text(objpos_w2s.X + 30, objpos_w2s.Y + 15, skill_r, true, _onCD);
							}

							if (d_doneCD)
							{
								render.draw_text(objpos_w2s.X - 15, objpos_w2s.Y + 30, "D", true, _normal);
							}
							else
							{
								char skill_d[10];
								snprintf(skill_d, sizeof(skill_d), "%d", static_cast<int>(d_RemainingCD));
								render.draw_text(objpos_w2s.X - 15, objpos_w2s.Y + 30, skill_d, true, _onCD);
							}

							if (f_doneCD)
							{
								render.draw_text(objpos_w2s.X + 15, objpos_w2s.Y + 30, "F", true, _normal);
							}
							else
							{
								char skill_f[10];
								snprintf(skill_f, sizeof(skill_f), "%d", static_cast<int>(f_RemainingCD));
								render.draw_text(objpos_w2s.X + 15, objpos_w2s.Y + 30, skill_f, true, _onCD);
							}
						}
					}

					if (g_display_spell_texture)
					{
						if (IsOnScreen)
						{
							auto healthBarPos = obj->GetHpBarPosition();

							std::string d_SpellFileName = getSpellImgByName(d_spellName_str, d_doneCD);
							std::string f_SpellFileName = getSpellImgByName(f_spellName_str, f_doneCD);

							if (render.draw_image(d_SpellFileName, 25, 25,
								Vector(healthBarPos.X, healthBarPos.Y, 0)))
							{
								if (d_doneCD)
								{
									std::string d_humanSpellName = champName_str + "'s " + getHumanSpellByName(
										d_spellName_str) + " : " + "READY";
									Engine::SendChat(d_humanSpellName.c_str());
								}
								else
								{
									std::string d_humanSpellName = champName_str + "'s " +
										getHumanSpellByName(d_spellName_str) + " : " + std::to_string(
											static_cast<int>(d_RemainingCD)) + "s";
									Engine::SendChat(d_humanSpellName.c_str());
								}
							}

							if (render.draw_image(f_SpellFileName, 25, 25,
								Vector(healthBarPos.X + 30, healthBarPos.Y, 0)))
							{
								if (f_doneCD)
								{
									std::string f_SpellFileName = champName_str + "'s " + getHumanSpellByName(
										f_spellName_str) + " : " + "READY";
									Engine::SendChat(f_SpellFileName.c_str());
								}
								else
								{
									std::string f_SpellFileName = champName_str + "'s " +
										getHumanSpellByName(f_spellName_str) + " : " + std::to_string(
											static_cast<int>(f_RemainingCD)) + "s";
									Engine::SendChat(f_SpellFileName.c_str());
								}
							}

							if (!d_doneCD)
							{
								char skill_d[10]; ///
								snprintf(skill_d, sizeof(skill_d), "%d", static_cast<int>(d_RemainingCD));
								render.draw_text(healthBarPos.X, healthBarPos.Y, skill_d, true, _onCD);
							}

							if (!f_doneCD)
							{
								char skill_f[10]; ///
								snprintf(skill_f, sizeof(skill_f), "%d", static_cast<int>(f_RemainingCD));
								render.draw_text(healthBarPos.X + 30, healthBarPos.Y, skill_f, true, _onCD);
							}
						}
					}

					if (Index != me_index)
					{
						auto AttackRange = obj->GetAttackRange();
						auto boundingRadius = obj->GetBoundingRadius();
						if (!IsEnemyToLocalPlayer)
						{

							if (g_draw_ally_range)
							{
								if (IsOnScreen && obj->IsAlive())
								{
									auto color = createRGB(124, 252, 0); // lawngreen
									Engine::DrawCircle(&Pos, AttackRange + boundingRadius, &color, 0, 0.0f, 0, 0.5f);
									//render.draw_circle(Pos, AttackRange, color, c_renderer::circle_3d, 50, 0.5f);
								}
							}
						}
						else
						{
							if (g_draw_enemy_range)
							{
								if (IsOnScreen)
								{
									auto color = createRGB(220, 20, 60); // crimson
									Engine::DrawCircle(&Pos, AttackRange + boundingRadius, &color, 0, 0.0f, 0, 0.5f);
									//render.draw_circle(Pos, AttackRange, color, c_renderer::circle_3d, 50, 0.5f);
								}
							}

							if (g_draw_line)
							{
								if (obj->IsVisible() && obj->IsAlive())
								{
									Vector mepos_w2s;
									Functions.WorldToScreen(&me_pos, &mepos_w2s);

									if ((mepos_w2s.X) < 3000.0f || (mepos_w2s.Y) < 3000.0f || (mepos_w2s.X) > -3000.0f
										|| (mepos_w2s.Y) > -3000.0f)
									{
										ImColor _gold = ImColor(255, 215, 0, 127);
										ImColor _orange = ImColor(255, 165, 0, 127);
										ImColor _orangered = ImColor(255, 69, 0, 127);
										ImColor _red = ImColor(255, 0, 0, 127);

										if ((me_pos.DistTo(Pos) >= 3000.0f) && (me_pos.DistTo(Pos) <= 4000.0f))
										{
											render.draw_line(mepos_w2s.X, mepos_w2s.Y, objpos_w2s.X, objpos_w2s.Y,
												_gold, 1); // gold
										}
										else if ((me_pos.DistTo(Pos) >= 2000.0f) && (me_pos.DistTo(Pos) <= 2999.0f))
										{
											render.draw_line(mepos_w2s.X, mepos_w2s.Y, objpos_w2s.X, objpos_w2s.Y,
												_orange, 1); // orange
										}
										else if ((me_pos.DistTo(Pos) >= 1000.0f) && (me_pos.DistTo(Pos) <= 1999.0f))
										{
											render.draw_line(mepos_w2s.X, mepos_w2s.Y, objpos_w2s.X, objpos_w2s.Y,
												_orangered, 1); // orangered
										}
										else if ((me_pos.DistTo(Pos) >= 300.0f) && (me_pos.DistTo(Pos) <= 999.0f))
										{
											render.draw_line(mepos_w2s.X, mepos_w2s.Y, objpos_w2s.X, objpos_w2s.Y, _red,
												1); // red
										}
									}

								}
							}

							if (opt_flashTimer_c != 0)
							{
								if ((j_key_flag == 1 && is_j_key_ready) || (x_key_flag == 1 && is_x_key_ready))
								{
									auto d_CDTime = d_spellSlot->GetTime();

									auto f_CDTime = f_spellSlot->GetTime();

									float flashCoolDownTime = 0;
									float flashCoolDown = 0;
									float smiteCoolDown = 0;
									bool summonerFlashFound = false;
									bool summonerSmiteFound = false;

									if (strcmp(d_spellName, "SummonerFlash") == 0)
									{
										if (!d_doneCD)
										{
											summonerFlashFound = true;
											flashCoolDownTime = d_CDTime;
											flashCoolDown = d_RemainingCD;
										}
									}
									else if (strcmp(f_spellName, "SummonerFlash") == 0)
									{
										if (!f_doneCD)
										{
											summonerFlashFound = true;
											flashCoolDownTime = f_CDTime;
											flashCoolDown = f_RemainingCD;
										}
									}

									if (d_spellName_str.find("smite") != std::string::npos)
									{
										summonerSmiteFound = true;
										smiteCoolDown = d_RemainingCD;
									}
									else if (f_spellName_str.find("smite") != std::string::npos)
									{
										summonerSmiteFound = true;
										smiteCoolDown = f_RemainingCD;
									}

									char str_RemainingCDSmite[MAXIMUM_TEXT_SIZE];
									snprintf(str_RemainingCDSmite, MAXIMUM_TEXT_SIZE, "%d",
										static_cast<int>(smiteCoolDown));
									std::string str_RemainingCDSmite_String(str_RemainingCDSmite);

									char str_RemainingCD[MAXIMUM_TEXT_SIZE];
									snprintf(str_RemainingCD, MAXIMUM_TEXT_SIZE, "%d",
										static_cast<int>(flashCoolDown));
									std::string str_RemainingCD_String(str_RemainingCD);

									char str_RemainingCDUlt[MAXIMUM_TEXT_SIZE];
									snprintf(str_RemainingCDUlt, MAXIMUM_TEXT_SIZE, "%d",
										static_cast<int>(r_RemainingCD));
									std::string str_RemainingCDUlt_string(str_RemainingCDUlt);

									std::string champNameString(champName);

									std::string msgSmite = "  >>  smite  " + (smiteCoolDown > 0
										? str_RemainingCDSmite_String + "s"
										: "READY");

									if (opt_flashTimer_c == 3 || opt_flashTimer_c == 4)
									{
										// full
										msgString = (opt_flashTimer_c == 4 ? "/all " : "") + champNameString +
											"  >>  f  " + (
												flashCoolDown > 0 ? str_RemainingCD_String + "s" : "READY") +
											"  >>  ult  " + (r_RemainingCD > 0
												? str_RemainingCDUlt_string + "s"
												: "READY") + (summonerSmiteFound ? msgSmite : "");

										if (j_key_flag == 1 && is_j_key_ready)
										{
											Engine::SendChat(msgString.c_str());
										}
										if (x_key_flag == 1 && is_x_key_ready)
										{
											Engine::PrintChat(msgString.c_str());
										}
									}
									else
									{
										if (summonerFlashFound)
										{
											std::string _time = Engine::SecondsToClock(
												static_cast<int>(flashCoolDownTime));

											if (opt_flashTimer_c == 1)
											{
												msgString += champNameString + " " + _time + " ";

											}
											else if (opt_flashTimer_c == 2)
											{
												msgString = champNameString + "  >>  " + _time + "  >>  " +
													str_RemainingCD_String + "s";
												if (j_key_flag == 1 && is_j_key_ready)
												{
													Engine::SendChat(msgString.c_str());
												}
												if (x_key_flag == 1 && is_x_key_ready)
												{
													Engine::PrintChat(msgString.c_str());
												}
											}
										}
									}
								}
							}
						}
					}
				}
				//===================Hero End===============================

				obj = holzer.GetNextObject(obj);
			}
			if (!finishedOnCreateObjectInit || !finishedOnDeleteObjectInit)
			{
				if (lastObjCount != 0)
				{
					if (objCount > lastObjCount)
						finishedOnCreateObjectInit = true;
					else
						finishedOnDeleteObjectInit = true;
				}
				lastObjCount = objCount;
			}

			heroList = _heroList;
			minionList = _minionList;

			isOneTime = false;

			if (opt_flashTimer_c == 1)
			{
				if (msgString != "")
				{
					if (j_key_flag == 1 && is_j_key_ready)
					{
						Engine::SendChat(msgString.c_str());
					}
					if (x_key_flag == 1 && is_x_key_ready)
					{
						Engine::PrintChat(msgString.c_str());
					}
				}
			}

			is_j_key_ready = false;
			is_x_key_ready = false;
			is_u_key_ready = false;

			skin_changer::update(heroList, minionList);
		}
	}

	void Menu::Render11()
	{
		Content();
	}

	void Menu::Render9()
	{
		Content();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FuncHook GetThreadContextHook;
LeagueHooksVEH _LeagueHooksVEH;
LeagueHooksHWBP _LeagueHooksHWBP; //supports only 4 hwbp (0,1,2,3)

DWORD oOnProcessSpell_addr, oOnCreateObject_addr, oOnDeleteObject_addr, oOnNewPath_addr;

///////////////////////////////////////LEAGUE HOOKS//////////////////////////////////////////////////
int __fastcall hk_OnProcessSpell(void* spellBook, void* edx, SpellInfo* spellInfo)
{
	SpellInfo* derefSpellInfo = (SpellInfo*)*(DWORD*)(spellInfo);
	short casterIndex = *(short*)((DWORD)spellBook + oSpellBookOwner);
	CObject* caster = Engine::FindObjectByIndex(heroList, casterIndex);

	if (caster)
	{
		if (caster->IsValidHero(heroList))
		{
			if (caster->IsHero())
			{
				AppLog.AddLog(
					(std::string(caster->GetChampionName()) + " : " + std::string(
						derefSpellInfo->GetSpellData()->GetSpellName2()) + " \n").c_str());
			}
		}
	}

	return Functions.OnProcessSpell_h(spellBook, spellInfo);
}

int __fastcall hk_OnCreateObject(CObject* obj, void* edx, unsigned id)
{
	if (obj == nullptr)
		return 0;

	if (obj->IsMissile() || obj->IsHero() || obj->IsMinion() || obj->IsTurret() || obj->IsInhibitor())
	{
		AppLog.AddLog(("OnCreateObject: " + std::string(obj->GetName()) + " \n").c_str());
	}

	return Functions.OnCreateObject_h(obj, id);
}

int __fastcall hk_OnDeleteObject(void* thisPtr, void* edx, CObject* obj)
{
	if (obj == nullptr)
		return 0;

	if (obj->IsMissile() || obj->IsHero() || obj->IsMinion() || obj->IsTurret() || obj->IsInhibitor())
	{
		AppLog.AddLog(("OnDeleteObject: " + std::string(obj->GetName()) + " \n").c_str());
	}

	return Functions.OnDeleteObject_h(thisPtr, obj);
}

int __cdecl hk_OnNewPath(CObject* obj, Vector* start, Vector* end, Vector* tail, int unk1, float* dashSpeed,
	unsigned dash, int unk3, char unk4, int unk5, int unk6, int unk7)
{
	if (obj == nullptr)
		return 0;

	if (obj->IsHero())
	{
		auto isDash = dash != 1;
		auto speed = *dashSpeed != 0.0f ? *dashSpeed : obj->GetObjMoveSpeed();
		auto isDash_str = (isDash ? "true" : "false");

		AppLog.AddLog(
			("OnNewPath: " + std::string(obj->GetName()) + ", Speed " + std::to_string(speed) + " \n").c_str());
		AppLog.AddLog(("\t\t\tisDash: " + std::string(isDash_str) + " \n").c_str());
		AppLog.AddLog(
			("\t\t\tVectorstart X: " + std::to_string(start->X) + " Y: " + std::to_string(start->Y) + " Z: " +
				std::to_string(start->Z) + " \n").c_str());

	}
	return Functions.OnNewPath_h(obj, start, end, tail, unk1, dashSpeed, dash, unk3, unk4, unk5, unk6, unk7);
}

////////////////////////////////////Etc. Hooks//////////////////////////////////////

void SetupGameHooks()
{
	oOnProcessSpell_addr = baseAddr + oOnprocessSpell;
	oOnCreateObject_addr = baseAddr + OnCreateObject;
	oOnDeleteObject_addr = baseAddr + oOnDeleteObject;
	oOnNewPath_addr = baseAddr + oOnNewPath;

	std::vector<PVECTORED_EXCEPTION_HANDLER> PVECTORED_EXCEPTION_HANDLER_list = Process::GetAllHandlers();
	Process::RemoveAllHandlers();
	DWORD leoAddr = LeagueHooks::init();
	Process::ReAddAllHandlers(PVECTORED_EXCEPTION_HANDLER_list);

	/*
	// STILL WORKING ON THIS. HWBP SHOULD NOT BE A PROBLEM WHEN I IMPLEMENTED THIS CORRECTLY
	LeagueDecryptData ldd = LeagueDecrypt::decrypt(nullptr, PVECTORED_EXCEPTION_HANDLER_list);
	AppLog.AddLog(("totalFailedDecrypted: " + hexify<int>((int)ldd.totalFailedDecrypted) + "\n").c_str());
	AppLog.AddLog(("totalSuccessDecrypted: " + hexify<int>((int)ldd.totalSuccessDecrypted) + "\n").c_str());
	AppLog.AddLog(("totalSuccess_EXCEPTION_CONTINUE_EXECUTION: " + hexify<int>((int)ldd.totalSuccess_EXCEPTION_CONTINUE_EXECUTION) + "\n").c_str());
	AppLog.AddLog(("totalSuccess_PAGE_NOACCESS: " + hexify<int>((int)ldd.totalSuccess_PAGE_NOACCESS) + "\n").c_str());
	*/

	for (int i = 10; i > 0; i--)
	{
		// give some time to finish loading client, then recall.
		Sleep(1000);
		AppLog.AddLog(("Hooks ready in: " + std::to_string(i) + "\n").c_str());
	}

	// Only need this if onprocessspell is hwbp
	AppLog.AddLog("Recalling to decrypt OnProcessSpell\n");
	Engine::CastSpellSelf(13); // recall to trigger onprocessspell 1 time before hooking

	AppLog.AddLog("Processing the recall\n");
	Sleep(1000); // process the recall
	
	while (!finishedOnCreateObjectInit || !finishedOnDeleteObjectInit || !finishedOnNewPathInit)
	{
		// these functions must be called atleast once before hooking so we'll test and wait for these functions. see above references.
		AppLog.AddLog("Waiting for functions to be called once...\n");
		Sleep(1000);
	}

	AppLog.AddLog("Giving some extra time...\n");
	Sleep(1000); // just extra allowance

	AppLog.AddLog("Hooks are now ready!\n");
	doneHookTimerAllowances = true;

	EnableHeavensGateHook(); // WEAPONIZING THE HEAVEN'S GATE
}

void MainLoop()
{
	/*if (g_onprocessspell != g_onprocessspell_last)
	{
		if (g_onprocessspell)
		{
			if (_LeagueHooksVEH.addHook(oOnProcessSpell_addr, (DWORD)hk_OnProcessSpell))
			{
				AppLog.AddLog("OnProcessSpellHook Success\n");
				g_onprocessspell_last = g_onprocessspell;
			}
			else
			{
				AppLog.AddLog("OnProcessSpellHook Failed\n");
			}
		}
		else
		{
			if (_LeagueHooksVEH.removeHook(oOnProcessSpell_addr))
			{
				AppLog.AddLog("OnProcessSpellHook remove Success\n");
				g_onprocessspell_last = g_onprocessspell;
			}
			else
			{
				AppLog.AddLog("OnProcessSpellHook remove Failed\n");
			}
		}
	}*/

	if (g_onprocessspell != g_onprocessspell_last) { // onprocessspell hwbp
		if (g_onprocessspell) {
			if (_LeagueHooksHWBP.addHook(oOnProcessSpell_addr, (DWORD)hk_OnProcessSpell, 3)) {
				AppLog.AddLog("OnProcessSpellHook Success\n");
				g_onprocessspell_last = g_onprocessspell;
			}
			else {
				AppLog.AddLog("OnProcessSpellHook Failed\n");
			}
		}
		else {
			if (_LeagueHooksHWBP.removeHook(3)) {
				AppLog.AddLog("OnProcessSpellHook remove Success\n");
				g_onprocessspell_last = g_onprocessspell;
			}
			else {
				AppLog.AddLog("OnProcessSpellHook remove Failed\n");
			}
		}
	}

	if (g_oncreateobject != g_oncreateobject_last)
	{
		if (g_oncreateobject)
		{
			if (_LeagueHooksHWBP.addHook(oOnCreateObject_addr, (DWORD)hk_OnCreateObject, 0))
			{
				AppLog.AddLog("OnCreateObjectHook Success\n");
				g_oncreateobject_last = g_oncreateobject;
			}
			else
			{
				AppLog.AddLog("OnCreateObjectHook Failed\n");
			}
		}
		else
		{
			if (_LeagueHooksHWBP.removeHook(0))
			{
				AppLog.AddLog("OnCreateObjectHook remove Success\n");
				g_oncreateobject_last = g_oncreateobject;
			}
			else
			{
				AppLog.AddLog("OnCreateObjectHook remove Failed\n");
			}
		}
	}

	if (g_ondeleteobject != g_ondeleteobject_last)
	{
		if (g_ondeleteobject)
		{
			if (_LeagueHooksHWBP.addHook(oOnDeleteObject_addr, (DWORD)hk_OnDeleteObject, 1))
			{
				AppLog.AddLog("OnDeleteObjectHook Success\n");
				g_ondeleteobject_last = g_ondeleteobject;
			}
			else
			{
				AppLog.AddLog("OnDeleteObjectHook Failed\n");
			}
		}
		else
		{
			if (_LeagueHooksHWBP.removeHook(1))
			{
				AppLog.AddLog("OnDeleteObjectHook remove Success\n");
				g_ondeleteobject_last = g_ondeleteobject;
			}
			else
			{
				AppLog.AddLog("OnDeleteObjectHook remove Failed\n");
			}
		}
	}

	if (g_onnewpath != g_onnewpath_last)
	{
		if (g_onnewpath)
		{
			if (_LeagueHooksHWBP.addHook(oOnNewPath_addr, (DWORD)hk_OnNewPath, 2))
			{
				AppLog.AddLog("OnNewPathHook Success\n");
				g_onnewpath_last = g_onnewpath;
			}
			else
			{
				AppLog.AddLog("OnNewPathHook Failed\n");
			}
		}
		else
		{
			if (_LeagueHooksHWBP.removeHook(2))
			{
				AppLog.AddLog("OnNewPathHook remove Success\n");
				g_onnewpath_last = g_onnewpath;
			}
			else
			{
				AppLog.AddLog("OnNewPathHook remove Failed\n");
			}
		}
	}
}

void RemoveGameHooks()
{
	LeagueHooks::deinit();
}
