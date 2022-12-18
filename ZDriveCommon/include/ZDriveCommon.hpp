#pragma once

#include "TGLib.hpp"

#include <cmath>
#include <corecrt_math_defines.h> // lazy? makes more sense to me than _USE_MATH_DEFINES, which for some reason wasn't working.

#include <format>
#include <optional>
#include <time.h>
#include <utility>

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "ZDriveCommon/Core.hpp"
#include "ZDriveCommon/Logger.hpp"
#include "ZDriveCommon/Random.hpp"
#include "ZDriveCommon/Enums.hpp"
#include "ZDriveCommon/Structs.hpp"

#include "ZDriveCommon/Lang.hpp"