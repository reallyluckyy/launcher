#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <iostream>
#include <map>
#include <thread>
#include <algorithm>
#include <optional>
#include <chrono>

#include "util.hpp"
#include "logger.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../include/httplib.h"

#include <windows.h>
#include "shobjidl.h"
#include "shlguid.h"
#include "strsafe.h"