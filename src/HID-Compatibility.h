#pragma once

#if defined(USBCON) && !defined(CORE_TEENSY)
# define KEYBOARDIO_HAS_PLUGGABLE_USB 1
#endif
