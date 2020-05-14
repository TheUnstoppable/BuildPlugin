// Unstoppable's Build Plugin
// Copyright 2020 Unstoppable
// 
// This plugin is free software: you can redistribute it and /or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include <General.h>
#include "engine.h"
#include "engine_da.h"
#include "da.h"
#include "da_gamefeature.h"
#include "da_event.h"
#include "da_settings.h"
#include "BuildPlugin.h"

void UP_Build_Plugin::Init() {
	Register_Chat_Command((DAECC)& UP_Build_Plugin::Build_Chat_Command, "!build|!bld|!make", 1, DAAccessLevel::SUPERMODERATOR, DAChatType::ALL);
	Register_Chat_Command((DAECC)& UP_Build_Plugin::Build_List_Chat_Command, "!buildlist|!bldlist|!makelist", 0, DAAccessLevel::SUPERMODERATOR, DAChatType::ALL);
	Register_Event(DAEvent::SETTINGSLOADED);
}

void UP_Build_Plugin::Settings_Loaded_Event() {
	DynamicVectorClass<StringClass> BuildsToLook;
	auto Buildcfg = DASettingsManager::Get_Settings("build.ini");
	do {
		DASettingsManager::Add_Settings("Build.ini");
		Buildcfg = DASettingsManager::Get_Settings("build.ini");
	}
	while (!Buildcfg);
	Reset_List();
	StringClass BuildList;
	BuildList = DASettingsManager::Get_String(BuildList, "Build", "Builds", "");
	DATokenParserClass Parser(BuildList, '|');
	for (char* Text = Parser.Get_String(); Text; Text = Parser.Get_String()) {
		BuildsToLook.Add(Text);
	}
	Iterate(BuildsToLook, a) {
		StringClass Text = BuildsToLook[a];
		StringClass AbbrevsTemp;
		DASettingsManager::Get_String(AbbrevsTemp, StringFormat("%s_BUILDPRESET", Text), "Abbreviations", "");
		DATokenParserClass Abbrev(AbbrevsTemp, '|');
		BuildablePreset* PresetData = new BuildablePreset;
		PresetData->Abbreviations = DynamicVectorClass<StringClass>();
		for (char* Text = Abbrev.Get_String(); Text; Text = Abbrev.Get_String()) {
			PresetData->Abbreviations.Add(Text);
		}
		PresetData->Tag = Text;
		PresetData->PlayerOffset = DASettingsManager::Get_Float(StringFormat("%s_BUILDPRESET", Text), "PlayerOffset", 0.0f);
		auto ObjSection = Buildcfg->Get_Section(StringFormat("%s_BUILDOBJECT", Text));
		for (INIEntry* i = ObjSection->EntryList.First(); i && i->Is_Valid(); i = i->Next()) {
			BuildablePresetObject* ObjectData = new BuildablePresetObject;
			ObjectData->Tag = i->Entry;
			DATokenParserClass ValueParser(i->Value, '|');
			ObjectData->Preset = ValueParser.Get_String();
			ObjectData->Model = ValueParser.Get_String();
			DATokenParserClass VectorParser(ValueParser.Get_String(), ',');
			ObjectData->OriginOffset = Vector3(0, 0, 0);
			bool a = VectorParser.Get_Float(ObjectData->OriginOffset.X);
			bool b = VectorParser.Get_Float(ObjectData->OriginOffset.Y);
			bool c = VectorParser.Get_Float(ObjectData->OriginOffset.Z);
			if (!(a && b && c)) {
				Console_Output("[UPB] Invalid Vector3 value at %s under %s. Object may appear broken.\n", Text, i->Entry);
			}
			bool d = ValueParser.Get_Float(ObjectData->FacingOffset);
			if (!d) {
				Console_Output("[UPB] Invalid Facing value at %s under %s. Object may appear broken.\n", Text, i->Entry);
			}
			PresetData->Objects.Add(ObjectData);
		}
		Presets.Add(PresetData);
	}
	Console_Output("[UPB] Loaded %d builds.\n", Presets.Count());
}

UP_Build_Plugin::~UP_Build_Plugin() {
	Reset_List();
}

bool UP_Build_Plugin::Build_Chat_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType)
{
	if (BuildablePreset* Preset = Find_Preset_By_Name(Text[0])) {
		Vector3 Location = Commands->Get_Position(Obj(Player));
		float Facing = Commands->Get_Facing(Obj(Player));
		Location.Y += Preset->PlayerOffset * sin(Facing * (float)(PI / 180)); //Let's make it spawn where soldier is facing.
		Location.X += Preset->PlayerOffset * cos(Facing * (float)(PI / 180));
		Iterate(Preset->Objects, i) { //Iterate the object and spawn them.
			auto object = Preset->Objects[i];
			Vector3 Loc(Location.X, Location.Y, Location.Z);
			Loc += object->OriginOffset;
			auto obj = Commands->Create_Object(object->Preset, Loc);
			if (obj) { Commands->Set_Model(obj, object->Model); }
			else { DA::Page_Player(Player, "UPB > Invalid preset: %s", object->Preset); }
		}
		DA::Page_Player(Player, "UPB > You've built %s successfully.", Preset->Tag);
		return true;
	}
	else {
		DA::Page_Player(Player, "UPB > Buildable preset not found, please use !buildlist for the list of buildables.");
		return false;
	}
}

bool UP_Build_Plugin::Build_List_Chat_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType)
{
	DA::Page_Player(Player, "UPB > Buildable presets (%d total): ", Presets.Count());
	StringClass List;
	Iterate(Presets, i) {
		auto Preset = Presets[i];
		List += StringFormat("UPB > %s (!build <", Preset->Tag);
		Iterate(Preset->Abbreviations, a) {
			List += StringFormat("%s, ", Preset->Abbreviations[a]);
		}
		List.TruncateRight(2);
		List += ">)";
		DA::Page_Player(Player, List);
		List.Erase(0, List.Get_Length());
	}
	return true;
}

Register_Game_Feature(UP_Build_Plugin, "Building", "EnableBuilding", 0);