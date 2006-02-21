/*****************************************************************************
 * Free42 -- a free HP-42S calculator clone
 * Copyright (C) 2004-2006  Thomas Okken
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef WINDOWS
#define SKINS_CC "skins.cpp"
#define SKIN2CC_CONF "skin2cpp.conf"
#define SKIN2CC "skin2cpp"
#else
#define SKINS_CC "skins.cc"
#define SKIN2CC_CONF "skin2cc.conf"
#define SKIN2CC "skin2cc"
#endif

FILE *out;

void write_bytes(FILE *file) {
    int pos;
    int first = 1;
    int c;
    while ((c = fgetc(file)) != EOF) {
	int width = c < 10 ? 1 : c < 100 ? 2 : 3;
	if (first) {
	    first = 0;
	    fprintf(out, "    ");
	    pos = 4;
	} else if (pos + width > 74) {
	    fprintf(out, ",\n    ");
	    pos = 4;
	} else {
	    fprintf(out, ", ");
	    pos += 2;
	}
	fprintf(out, "%d", c);
	pos += width;
    }
}

int main(int argc, char *argv[]) {
    FILE *conf;
    FILE *inp;
    char line[256];
    char skinname[100][256];
    char skinfile[100][256];
    int nskins = 0, i;

    out = fopen(SKINS_CC, "w");
    if (out == NULL) {
	fprintf(stderr, "%s: can't open output file \"%s\"\n", SKIN2CC, SKINS_CC);
	return 1;
    }

    fprintf(out,
	"/* %s\n"
	" * Contains the built-in skins for Free42 (Unix & Windows).\n"
	" * This file is generated by the %s program,\n"
	" * under control of the %s file, which is\n"
	" * a list of skin descriptions and their user-visible names;\n"
	" * each entry consists of the skin name on one line, followed\n"
	" * by the user-visible name on the next line.\n"
	" * The skins consist of two files, <name>.layout (the layout \n"
	" * description), and <name>.gif (the skin bitmap); these files \n"
	" * are looked for in ../skins/.\n"
	" * NOTE: this is a generated file; do not edit!\n"
	" */\n\n", SKINS_CC, SKIN2CC, SKIN2CC_CONF);

	fprintf(out, "#if defined(WINDOWS) && !defined(__GNUC__)\n");
	fprintf(out, "/* Disable warning 'initializing' : truncation from 'const int ' to 'const char ' */\n");
	fprintf(out, "#pragma warning(disable: 4305)\n");
	fprintf(out, "/* Disable warning 'initializing' : truncation of constant value */\n");
	fprintf(out, "#pragma warning(disable: 4309)\n");
	fprintf(out, "#endif\n\n");

    conf = fopen(SKIN2CC_CONF, "r");
    if (conf == NULL) {
	int err = errno;
	fprintf(stderr, "Can't open \"%s\": %s (%d).\n",
			SKIN2CC_CONF, strerror(err), err);
	fclose(out);
	remove(SKINS_CC);
	return 1;
    }

    while (1) {
	int len;

	if (fgets(line, 256, conf) == NULL)
	    break;
	len = strlen(line);
	if (len > 0 && line[len - 1] == '\n')
	    line[--len] = 0;
	strcpy(skinfile[nskins], line);

	if (fgets(line, 256, conf) == NULL)
	    break;
	len = strlen(line);
	if (len > 0 && line[len - 1] == '\n')
	    line[--len] = 0;
	strcpy(skinname[nskins], line);

	nskins++;
    }
    fclose(conf);


    fprintf(out, "/****************************************/\n");
    fprintf(out, "/* Number of skins defined in this file */\n");
    fprintf(out, "/****************************************/\n\n");

    fprintf(out, "int skin_count = %d;\n\n\n", nskins);


    fprintf(out, "/**************/\n");
    fprintf(out, "/* Skin names */\n");
    fprintf(out, "/**************/\n\n");

    fprintf(out, "const char *skin_name[] = {\n");
    for (i = 0; i < nskins; i++)
	fprintf(out, "    \"%s\"%s\n", skinname[i], i < nskins - 1 ? "," : "");
    fprintf(out, "};\n\n\n");


    fprintf(out, "/*************************************/\n");
    fprintf(out, "/* Sizes of skin layout descriptions */\n");
    fprintf(out, "/*************************************/\n\n");
    
    // TODO: If I put 'const' here, the symbol is not exported. Why?
    fprintf(out, "/*const*/ long skin_layout_size[] = {\n");
    for (i = 0; i < nskins; i++) {
	char fname[1024];
	strcpy(fname, "../skins/");
	strcat(fname, skinfile[i]);
	strcat(fname, ".layout");
	inp = fopen(fname, "rb");
	if (inp == NULL) {
	    int err = errno;
	    fprintf(stderr, "Can't open \"%s\": %s (%d)\n",
			fname, strerror(err), err);
	    fclose(out);
	    remove(SKINS_CC);
	    return 1;
	}
	fseek(inp, 0, SEEK_END);
	fprintf(out, "    %d%s\n", ftell(inp), i < nskins - 1 ? "," : "");
	fclose(inp);
    }
    fprintf(out, "};\n\n\n");


    fprintf(out, "/****************************/\n");
    fprintf(out, "/* Skin layout descriptions */\n");
    fprintf(out, "/****************************/\n\n");

    for (i = 0; i < nskins; i++) {
	char fname[1024];
	strcpy(fname, "../skins/");
	strcat(fname, skinfile[i]);
	strcat(fname, ".layout");
	inp = fopen(fname, "rb");
	if (inp == NULL) {
	    int err = errno;
	    fprintf(stderr, "Can't open \"%s\": %s (%d)\n",
			fname, strerror(err), err);
	    fclose(out);
	    remove(SKINS_CC);
	    return 1;
	}
	fprintf(out, "static const unsigned char skin%d_layout_data[] = {\n", i);
	write_bytes(inp);
	fprintf(out, "\n};\n\n");
	fclose(inp);
    }
    fprintf(out, "/*const*/ unsigned char *skin_layout_data[] = {\n");
    for (i = 0; i < nskins; i++)
	fprintf(out, "    (unsigned char *) skin%d_layout_data%s\n", i, i < nskins - 1 ? "," : "");
    fprintf(out, "};\n\n\n");


    fprintf(out, "/*************************/\n");
    fprintf(out, "/* Sizes of skin bitmaps */\n");
    fprintf(out, "/*************************/\n\n");

    // TODO: If I put 'const' here, the symbol is not exported. Why?
    fprintf(out, "/*const*/ long skin_bitmap_size[] = {\n");
    for (i = 0; i < nskins; i++) {
	char fname[1024];
	strcpy(fname, "../skins/");
	strcat(fname, skinfile[i]);
	strcat(fname, ".gif");
	inp = fopen(fname, "rb");
	if (inp == NULL) {
	    int err = errno;
	    fprintf(stderr, "Can't open \"%s\": %s (%d)\n",
			fname, strerror(err), err);
	    fclose(out);
	    remove(SKINS_CC);
	    return 1;
	}
	fseek(inp, 0, SEEK_END);
	fprintf(out, "    %d%s\n", ftell(inp), i < nskins - 1 ? "," : "");
	fclose(inp);
    }
    fprintf(out, "};\n\n\n");


    fprintf(out, "/****************/\n");
    fprintf(out, "/* Skin bitmaps */\n");
    fprintf(out, "/****************/\n\n");

    for (i = 0; i < nskins; i++) {
	char fname[1024];
	strcpy(fname, "../skins/");
	strcat(fname, skinfile[i]);
	strcat(fname, ".gif");
	inp = fopen(fname, "rb");
	if (inp == NULL) {
	    int err = errno;
	    fprintf(stderr, "Can't open \"%s\": %s (%d)\n",
			fname, strerror(err), err);
	    fclose(out);
	    remove(SKINS_CC);
	    return 1;
	}
	fprintf(out, "static const unsigned char skin%d_bitmap_data[] = {\n", i);
	write_bytes(inp);
	fprintf(out, "\n};\n\n");
	fclose(inp);
    }
    fprintf(out, "/*const*/ unsigned char *skin_bitmap_data[] = {\n");
    for (i = 0; i < nskins; i++)
	fprintf(out, "    (unsigned char *) skin%d_bitmap_data%s\n", i, i < nskins - 1 ? "," : "");
    fprintf(out, "};\n\n\n");


    fprintf(out, "/***********/\n");
    fprintf(out, "/* The End */\n");
    fprintf(out, "/***********/\n");
    fclose(out);
    return 0;
}
