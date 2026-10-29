/* stb_vorbis in its own unit: its single-letter macros (L, R, C) clash with
 * Lua's `lua_State *L` if compiled inside audio.c. */
#include "stb_vorbis.c"
