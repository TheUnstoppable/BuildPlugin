/*
   Unstoppable's Build Plugin
   Copyright 2020 Unstoppable

   This plugin is free software: you can redistribute it and /or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
*/

#include "da_gamefeature.h"
#include "da_event.h"
#define PI 3.141592653589
#define Obj(Player) Player->Get_GameObj()
#define Iterate(Vector, Value) for(int Value = 0; Value < Vector.Count(); ++Value)

struct BuildablePresetObject { //Allows us to spawn multiple objects with presets.
	StringClass Tag; //A short tag about the object;
	StringClass Preset; //Our preset
	StringClass Model; //Custom model we wanna use.
	Vector3 OriginOffset; //Our offset from the point object created.
	float FacingOffset; //Initial object facing + this.
};
struct BuildablePreset {
	StringClass Tag; //A short tag about the preset;
	DynamicVectorClass<StringClass> Abbreviations; //To use after command !build.
	DynamicVectorClass<BuildablePresetObject*> Objects; //List of objects we will build.
	float PlayerOffset; //Our offset to player.
};
inline bool operator== (const BuildablePresetObject& l, const BuildablePresetObject& r) { return strcmp(l.Tag, r.Tag) == 0; }
inline bool operator!= (const BuildablePresetObject& l, const BuildablePresetObject& r) { return strcmp(l.Tag, r.Tag) != 0; }
inline bool operator== (const BuildablePreset& l, const BuildablePreset& r) { return !strcmp(l.Tag, r.Tag); }
inline bool operator!= (const BuildablePreset& l, const BuildablePreset& r) { return strcmp(l.Tag, r.Tag) != 0; }
class UP_Build_Plugin : public DAEventClass, public DAGameFeatureClass {
public:
	virtual void Init();
	virtual void Settings_Loaded_Event();
	~UP_Build_Plugin();
	bool Build_Chat_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);
	bool Build_List_Chat_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);
private:
	DynamicVectorClass<BuildablePreset*> Presets;
	int AccessLevel;
protected:
	BuildablePreset* Find_Preset_By_Name(StringClass Name) {
		Iterate(Presets, i) {
			BuildablePreset* Preset = Presets[i];
			Iterate(Preset->Abbreviations, a) {
				if (!strcmp(Preset->Abbreviations[a], Name)) {
					return Preset;
				}
			}
		}
		return 0;
	}
	void Reset_List() {
		for (int i = 0; i < Presets.Count(); ++i) {
			//for (int a = 0; i < Presets[i]->Objects.Count(); ++a) {
			//	delete Presets[i]->Objects[a];
			//}
			delete Presets[i];
		}
		Presets.Delete_All();
	}
};