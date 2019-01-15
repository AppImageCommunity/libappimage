#pragma once

/**
 * C++ headers aggregation to differentiate from the C only interface.
 */

#include <appimage/core/AppImage.h>

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION
#include <appimage/desktop_integration/IntegrationManager.h>
#endif
