#include "LaminaHandler.h"
#include "include/wrapper/cef_helpers.h"

using namespace std;

namespace {
   LaminaHandler* g_instance = NULL;
}  // namespace

LaminaHandler::LaminaHandler() {
   DCHECK(!g_instance);
   g_instance = this;
}

LaminaHandler::~LaminaHandler() {
   g_instance = NULL;
}

// static
LaminaHandler* LaminaHandler::GetInstance() {
   return g_instance;
}

