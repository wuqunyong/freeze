#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"

namespace APie {

apie::status::Status initHook();
apie::status::Status startHook();
apie::status::Status readyHook();
apie::status::Status exitHook();

}
