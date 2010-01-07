#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

__BEGIN_DECLS

typedef struct {
    int x;
    int y;
} position_t;

typedef struct {
    int modifiers;
    int keysym;
} hotkey_t;

typedef char varchar[128];

#define TRIGGER_KEY 0x00
#define ENG_KEY 0x01

#define ICBAR_POS 0x02
#define PREEDIT_OPACITY 0x03
#define PREEDIT_COLOR 0x04
#define PREEDIT_FONT 0x05
#define PREEDIT_FONT_COLOR 0x06

#define MAX_KEY 0xff

void  settings_init();
void  settings_destroy();

void  settings_load();
void  settings_save();

void  settings_get(int key, void* data);
void  settings_set(int key, void* data);

__END_DECLS

#endif /* _SETTINGS_H_ */
