#import "macos_keycode.h"

static unsigned osx_modifiers_to_ime_modifiers (unsigned modifiers) 
{
    unsigned ret = 0;

    if (modifiers & OSX_SHIFT_MASK)
        ret |= IM_SHIFT_MASK;
    if (modifiers & OSX_CTRL_MASK)
        ret |= IM_CTRL_MASK;
    if (modifiers & OSX_ALT_MASK)
        ret |= IM_ALT_MASK;

    return ret;
}

static unsigned osx_keycode_to_ime_keycode (unsigned keycode) 
{
    unsigned ret = 0;

    switch (keycode) {
        case OSX_VK_SPACE:
            ret = IM_VK_SPACE;
            break;
        case OSX_VK_MINUS:
            ret = IM_VK_MINUS;
            break;
        case OSX_VK_EQUALS:
            ret = IM_VK_EQUALS;
            break;
        case OSX_VK_COMMA:
            ret = IM_VK_COMMA;
            break;
        case OSX_VK_PERIOD:
            ret = IM_VK_PERIOD;
            break;
        case OSX_VK_OPEN_BRACKET:
            ret = IM_VK_OPEN_BRACKET;
            break;
        case OSX_VK_CLOSE_BRACKET:
            ret = IM_VK_CLOSE_BRACKET;
            break;
        case OSX_VK_BACK_QUOTE:
            ret = IM_VK_BACK_QUOTE;
            break;
        case OSX_VK_ENTER:
            ret = IM_VK_ENTER;
            break;
        case OSX_VK_BACK_SPACE:
            ret = IM_VK_BACK_SPACE;
            break;
        case OSX_VK_ESCAPE:
            ret = IM_VK_ESCAPE;
            break;
        case OSX_VK_PAGE_UP:
            ret = IM_VK_PAGE_UP;
            break;
        case OSX_VK_PAGE_DOWN:
            ret = IM_VK_PAGE_DOWN;
            break;
        case OSX_VK_END:
            ret = IM_VK_END;
            break;
        case OSX_VK_HOME:
            ret = IM_VK_HOME;
            break;
        case OSX_VK_LEFT:
            ret = IM_VK_LEFT;
            break;
        case OSX_VK_UP:
            ret = IM_VK_UP;
            break;
        case OSX_VK_RIGHT:
            ret = IM_VK_RIGHT;
            break;
        case OSX_VK_DOWN:
            ret = IM_VK_DOWN;
            break;
        case OSX_VK_DELETE:
            ret = IM_VK_DELETE;
            break;
        case OSX_VK_CONTROL:
            ret = IM_VK_CONTROL;
            break;
        case OSX_VK_SHIFT:
            ret = IM_VK_SHIFT;
            break;
        case OSX_VK_ALT:
            ret = IM_VK_ALT;
            break;
        default:
            ret = keycode;
            break;
    }
    
    return ret;
}

CKeyEvent osx_keyevent_to_ime_keyevent (unsigned keyCode, unsigned keyChar, unsigned modifiers)
{
    unsigned value = osx_keycode_to_ime_keycode (keyCode);
    unsigned mod = osx_modifiers_to_ime_modifiers (modifiers);
    return CKeyEvent (value, keyChar, mod);
}

