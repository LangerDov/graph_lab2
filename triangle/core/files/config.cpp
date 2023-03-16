#include "config.h"
#include "tinyxml2/tinyxml2.h"
#include "debugging/log.h"
#include "files.h"
#include "globalvars.h"

#define SAVE_LOGS false

using namespace tinyxml2;
XMLDocument main_cfg;

void SetDefaultConfig()
{
	main_cfg.Clear();
	XMLElement* max_tps = main_cfg.NewElement("max_tps");
	XMLElement* prj_name = main_cfg.NewElement("prj_name");
	XMLElement* save_logs = main_cfg.NewElement("save_logs");
	XMLElement* version = main_cfg.NewElement("version");
	max_tps->SetText(gVars->TPS_max);
	prj_name->SetText(gVars->prj_name.Str());
	save_logs->SetText(SAVE_LOGS);
	main_cfg.InsertEndChild(max_tps);
	main_cfg.InsertEndChild(prj_name);
	main_cfg.InsertEndChild(save_logs);
	main_cfg.SaveFile("configs/main.xml");
}

bool Config::Init()
{
	if (!FILES::PathExist(CONFIGS_FOLDER))
	{
		Log(lWarn, "Config dir doesnt exist, creating config dir");
		if (FILES::CreateDir(CONFIGS_FOLDER))
		{
			Log(lInfo, "Config dir created");
		}
		else
		{
			Log(lErr, "Config dir was not created");
		}
	}
	if (main_cfg.LoadFile("configs/main.xml") != XML_SUCCESS)
	{
		Log(lErr, "Main xml config not loaded. Using default settings");
		if (FILES::PathExist("configs/main.xml"))
			Log(lInfo, "Main xml file exist");
		else
		{
			Log(lInfo, "Main xml file not exist, creating main xml file");
			SetDefaultConfig();
		}
	}
	else
		Log(lInfo, "main xml config loaded");
	return false;
}
