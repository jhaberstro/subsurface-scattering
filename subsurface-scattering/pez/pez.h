// Pez was developed by Philip Rideout and released under the MIT License.

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// Pez is a platform abstraction layer for OpenGL; it's an extra-tiny alternative to GLUT.
// To create a cross-platform app, simply link against the Pez source and implement these four functions.
//
const char* PezInitialize(int width, int height); // receive window size and return window title
void PezRender();                                 // draw scene (Pez swaps the backbuffer for you)
void PezUpdate(unsigned int milliseconds);        // receive elapsed time (e.g., update physics)
void PezHandleMouse(int x, int y, int action);    // handle mouse action: PEZ_DOWN, PEZ_UP, or PEZ_MOVE
void PezHandleKeyDown(int key);                   // handle key down actions
void PezHandleKeyUp(int key);                     // handle key up actions

// Here's the Pez configuration section.  Modify these constants to your liking!
//
#define PEZ_VIEWPORT_WIDTH 900
#define PEZ_VIEWPORT_HEIGHT 750
#define PEZ_ENABLE_MULTISAMPLING 1
#define PEZ_VERTICAL_SYNC 1

#if defined(__APPLE__)
#define PEZ_GL_VERSION_TOKEN "GL2"
#define PEZ_FORWARD_COMPATIBLE_GL 0
#else
#define PEZ_GL_VERSION_TOKEN "GL3"
#define PEZ_FORWARD_COMPATIBLE_GL 1
#endif

// Pez also defines a small handful of fixed constants and macros:
//
enum
{
    KEY_BACKSPACE,
    KEY_DELETE,
    KEY_TAB,
    KEY_RETURN,
    KEY_ESCAPE,
    KEY_SPACE,
    KEY_ZERO,
    KEY_ONE,
    KEY_TWO,
    KEY_THREE,
    KEY_FOUR,
    KEY_FIVE,
    KEY_SIX,
    KEY_SEVEN,
    KEY_EIGHT,
    KEY_NINE,
    KEY_MINUS,
    KEY_EQUALS,
    KEY_UPARROW,
    KEY_DOWNARROW,
    KEY_RIGHTARROW,
    KEY_LEFTARROW,
    KEY_LEFTBRACKET,
    KEY_RIGHTBRACKET,
    KEY_BACKSLASH,
    KEY_SEMICOLON,
    KEY_SINGLEQUOTE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFTSHIFT,
    KEY_RIGHTSHIFT,
    KEY_LEFTCONTROL,
    KEY_RIGHTCONTROL,
    KEY_LEFTALT,
    KEY_RIGHTALT,
    KEY_LEFTAPPLE,
    KEY_RIGHTAPPLE,
    KEY_LEFTMOUSE,
    KEY_MIDDLEMOUSE,
    KEY_RIGHTMOUSE,
    KEY_KEYCODECOUNT        
};
enum {PEZ_DOWN, PEZ_UP, PEZ_MOVE};
#define TwoPi (6.28318531f)
#define Pi (3.14159265f)
#define countof(A) (sizeof(A) / sizeof(A[0]))

// Additionally, Pez provides the following utility functions as alternatives to printf, exceptions, and asserts.
// On Windows, strings get sent to the debugger, so use the VS debugger window or dbgview to see them.
//
void PezDebugString(const char* pStr, ...);
void PezDebugStringW(const wchar_t* pStr, ...);
void PezFatalError(const char* pStr, ...);
void PezFatalErrorW(const wchar_t* pStr, ...);
#define PezCheckCondition(A, B) if (!(A)) { PezFatalError(B); }
#define PezCheckConditionW(A, B) if (!(A)) { PezFatalErrorW(B); }

#ifdef __cplusplus
}
#endif
