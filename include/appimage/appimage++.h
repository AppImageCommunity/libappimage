#pragma once

/**
 * C++ headers aggregation to differentiate from the C only interface.
 */

#include <appimage/config.h>
#include <appimage/core/AppImage.h>

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
#include <appimage/desktop_integration/IntegrationManager.h>
#endif
