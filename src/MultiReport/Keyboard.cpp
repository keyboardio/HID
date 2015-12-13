/*
Copyright (c) 2014-2015 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "Keyboard.h"

static const uint8_t _hidMultiReportDescriptorKeyboard[] PROGMEM = {
    //  NKRO Keyboard
    0x05, 0x01,                      /* USAGE_PAGE (Generic Desktop)	  47 */
    0x09, 0x06,                      /* USAGE (Keyboard) */
    0xa1, 0x01,                      /* COLLECTION (Application) */
    0x85, HID_REPORTID_NKRO_KEYBOARD,	 /*   REPORT_ID */
    0x05, 0x07,                      /*   USAGE_PAGE (Keyboard) */

    /* Keyboard Modifiers (shift, alt, ...) */
    0x19, 0xe0,                      /*   USAGE_MINIMUM (Keyboard LeftControl) */
    0x29, 0xe7,                      /*   USAGE_MAXIMUM (Keyboard Right GUI) */
    0x15, 0x00,                      /*   LOGICAL_MINIMUM (0) */
    0x25, 0x01,                      /*   LOGICAL_MAXIMUM (1) */
    0x75, 0x01,                      /*   REPORT_SIZE (1) */
    0x95, 0x08,                      /*   REPORT_COUNT (8) */
    0x81, 0x02,                      /*   INPUT (Data,Var,Abs) */

    /* 104 Keys as bitmap */
    0x19, 0x00,						/*   Usage Minimum (0) */
    0x29, NKRO_KEY_COUNT - 1,		/*   Usage Maximum (103) */
    0x15, 0x00,						/*   Logical Minimum (0) */
    0x25, 0x01,						/*   Logical Maximum (1) */
    0x75, 0x01,						/*   Report Size (1) */
    0x95, NKRO_KEY_COUNT,			/*   Report Count (104) */
    0x81, 0x02,						/*   Input (Data, Variable, Absolute) */

    /* 1 Custom Keyboard key */
    0x95, 0x01,                      /*   REPORT_COUNT (1) */
    0x75, 0x08,                      /*   REPORT_SIZE (8) */
    0x15, 0x00,                      /*   LOGICAL_MINIMUM (0) */
    0x26, 0xE7, 0x00,                /*   LOGICAL_MAXIMUM (231) */
    0x19, 0x00,                      /*   USAGE_MINIMUM (Reserved (no event indicated)) */
    0x29, 0xE7,                      /*   USAGE_MAXIMUM (Keyboard Right GUI) */
    0x81, 0x00,                      /*   INPUT (Data,Ary,Abs) */

    /* End */
    0xC0						     /*   End Collection */
};

Keyboard_::Keyboard_(void) {
    static HIDSubDescriptor node(_hidMultiReportDescriptorKeyboard, sizeof(_hidMultiReportDescriptorKeyboard));
    HID().AppendDescriptor(&node);
}

void Keyboard_::begin(void) {
    // Force API to send a clean report.
    // This is important for and HID bridge where the receiver stays on,
    // while the sender is resetted.
    releaseAll();
    sendReport();
}


void Keyboard_::end(void) {
    releaseAll();
    sendReport();
}



int Keyboard_::sendReport(void) {
    return HID().SendReport(HID_REPORTID_NKRO_KEYBOARD, &_keyReport, sizeof(_keyReport));
}

size_t Keyboard_::press(uint8_t k) {
    // Press keymap key
    if (k < NKRO_KEY_COUNT) {
        uint8_t bit = 1 << (uint8_t(k) % 8);
        _keyReport.keys[k / 8] |= bit;
        return 1;
    }

    // It's a modifier key
    else if(k >= HID_KEYBOARD_FIRST_MODIFIER && k <= HID_KEYBOARD_LAST_MODIFIER) {
        // Convert key into bitfield (0 - 7)
        k = uint8_t(k) - uint8_t(HID_KEYBOARD_FIRST_MODIFIER);
        _keyReport.modifiers = (1 << k);
        return 1;
    }

    // Its a custom key (outside our keymap)
    else {
        // Add k to the key report only if it's not already present
        // and if there is an empty slot. Remove the first available key.
        auto key = _keyReport.key;

        // Is key already in the list or did we found an empty slot?
        if ((key == uint8_t(k) || key == 0x00)) {
            _keyReport.key = k;
            return 1;
        }
    }

    // No empty/pressed key was found
    return 0;
}

size_t Keyboard_::release(uint8_t k) {
    // Press keymap key
    if (k < NKRO_KEY_COUNT) {
        uint8_t bit = 1 << (uint8_t(k) % 8);
        _keyReport.keys[k / 8] &= ~bit;
        return 1;
    }

    // It's a modifier key
    else if(k >= HID_KEYBOARD_FIRST_MODIFIER && k <= HID_KEYBOARD_LAST_MODIFIER) {
        // Convert key into bitfield (0 - 7)
        k = k - HID_KEYBOARD_FIRST_MODIFIER;
        _keyReport.modifiers &= ~(1 << k);
        return 1;
    }

    // Its a custom key (outside our keymap)
    else {
        // Add k to the key report only if it's not already present
        // and if there is an empty slot. Remove the first available key.
        auto key = _keyReport.key;

        // Test the key report to see if k is present. Clear it if it exists.
        if (key == k) {
            _keyReport.key = 0x00;
            return 1;
        }
    }

    // No empty/pressed key was found
    return 0;
}

// TODO: replace this with a mmap interface
size_t Keyboard_::releaseAll(void) {
    // Release all keys
    size_t ret = 0;
    for (uint8_t i = 0; i < sizeof(_keyReport.allkeys); i++) {
        // Is a key in the list or did we found an empty slot?
        auto bits = _keyReport.allkeys[i];
        do {
            if(bits & 0x01) {
                ret++;
            }
            bits >>=1;
        } while(bits);
        _keyReport.allkeys[i] = 0x00;
    }
    return ret;
}



size_t Keyboard_::write(uint8_t k) {
    if(k >= sizeof(_asciimap)) // Ignore invalid input
        return 0;

    // Read key from ascii lookup table
    k = pgm_read_byte(_asciimap + k);

    if(k & SHIFT)
        press(HID_KEYBOARD_LEFT_SHIFT);
    press(k & ~SHIFT);
    sendReport();
    releaseAll();
    sendReport();
}



Keyboard_ Keyboard;
