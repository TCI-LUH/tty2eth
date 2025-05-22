#pragma once

#include "storage.h"
#include "memfs.h"

#include <cJSON_Utils.h>

extern cJSON* mainConfig;
extern FileDef* mainConfigFile;

extern FileDef* metricCertFile;
extern FileDef* metricKeyFile;
void initConfigFiles();

bool configIsAutoBoot();
char* configGetMetricIdentifier();