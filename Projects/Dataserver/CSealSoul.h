#pragma once

#include "pugixml.hpp"
#include <SharedStructs.h>

bool ReadSealInfo(int id, STRUCT_SEALFILE& info);
bool WriteSealInfo(int id, const STRUCT_SEALFILE& info);