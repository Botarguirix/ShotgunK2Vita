#ifndef SG_STEAM_INSTALL_H
#define SG_STEAM_INSTALL_H
#include <stdio.h>

/* Installs the game from <root>/data.sgr (the owner's Steam file) into <root>
 * on first boot: decrypts and inflates the container while streaming, writes
 * its resources, converts PNGs to .sgimg and writes the Steam bridge profile.
 * A marker skips the work on later boots while data.sgr stays the same.
 * Returns 1 when the data is ready (installed now, earlier, or prepared on PC),
 * 0 on failure; the reason is written to log. */
typedef void (*SgInstallProgress)(unsigned percent,void *user);
int sg_steam_install(const char *root,FILE *log,SgInstallProgress progress,void *user);

/* Boot loader art written by tools/make_livearea.py ("SGL1", u16 w/h, 256
 * RGBA palette, zlib'd indices). Returns a malloc'd w*h buffer of display
 * pixels, or NULL when the file is missing or another size. */
unsigned *sg_steam_loader_image(const char *path,int w,int h);
/* Copies VERSION="..." from <root>/code/data.lua (the Steam game's version).
 * Returns 0 and leaves out empty before the data is installed. */
int sg_steam_game_version(const char *root,char *out,size_t size);

#endif
