#ifndef _RouterCalculater_Version_h_
#define _RouterCalculater_Version_h_
#include <Core/Core.h>
namespace Upp { namespace RouterApp {
// Bump for every delivered build. This is application identity, NOT the JSON schema.
constexpr const char* APP_VERSION = "0.3.0";
constexpr const char* APP_BUILD = "2026-10-14.1";
#ifdef _DEBUG
constexpr const char* APP_CONFIGURATION = "Debug";
#else
constexpr const char* APP_CONFIGURATION = "Release";
#endif
inline String AppIdentity() {
    return String("UPP Router Calculator ") + APP_VERSION + " / " + APP_CONFIGURATION + " / build " + APP_BUILD;
}
} }
#endif
