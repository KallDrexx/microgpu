#include "microgpu-common/common.h"
#include "microgpu-common/fonts/fonts.h"
#include "microgpu-common/operations/execution/fonts.h"

char string[256] = {0};

void mgpu_exec_font_draw(Mgpu_TextureManager *textureManager, Mgpu_DrawCharsOperation *operation) {
    // We can't guarantee the consumer sent us a null terminated string, so move it into the
    // string to be sure of it.

    int count = min(sizeof(string), operation->numCharacters);
    int index;
    for (index = 0; index < count; index++) {
        string[index] = operation->characters[index];
    }

    if (index >= 255) {
        string[255] = '\0';
    } else {
        string[index] = '\0';
    }

    mgpu_font_draw(textureManager,
                   operation->fontId,
                   operation->textureId,
                   string,
                   operation->color,
                   operation->startX,
                   operation->startY);
}
