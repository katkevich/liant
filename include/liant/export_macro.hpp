#pragma once

#if defined(LIANT_MODULE) || defined(LIANT_STD_MODULE)
#define LIANT_EXPORT export
#else
#define LIANT_EXPORT
#endif