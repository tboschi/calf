/* Calf DSP Library Utility Application - calfjackhost
 * Standalone application module wrapper example.
 * Copyright (C) 2007 Krzysztof Foltman
 *
 * Note: This module uses phat graphics library, so it's 
 * licensed under GPL license, not LGPL.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */
#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <config.h>
#include <sys/stat.h>
#include <glade/glade.h>
#include <jack/jack.h>
#include <calf/giface.h>
#include <calf/jackhost.h>
#include <calf/modules.h>
#include <calf/modules_dev.h>
#include <calf/gui.h>
#include <calf/preset.h>
#include <calf/preset_gui.h>

using namespace synth;
using namespace std;

// I don't need anyone to tell me this is stupid. I already know that :)
plugin_gui_window *gui_win;

const char *effect_name = "flanger";
const char *client_name = "calfhost";
const char *input_name = "input";
const char *output_name = "output";
const char *midi_name = "midi";

void destroy(GtkWindow *window, gpointer data)
{
    gtk_main_quit();
}

static struct option long_options[] = {
    {"help", 0, 0, 'h'},
    {"version", 0, 0, 'v'},
    {"client", 1, 0, 'c'},
    {"effect", 1, 0, 'e'},
    {"plugin", 1, 0, 'p'},
    {"input", 1, 0, 'i'},
    {"output", 1, 0, 'o'},
    {0,0,0,0},
};

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    glade_init();
    while(1) {
        int option_index;
        int c = getopt_long(argc, argv, "c:e:i:o:m:p:hv", long_options, &option_index);
        if (c == -1)
            break;
        switch(c) {
            case 'h':
            case '?':
                printf("JACK host for Calf effects\n"
                    "Syntax: %s [--plugin reverb|flanger|filter|monosynth] [--client <name>] [--input <name>]"
                    "       [--output <name>] [--midi <name>] [--help] [--version]\n", 
                    argv[0]);
                return 0;
            case 'v':
                printf("%s\n", PACKAGE_STRING);
                return 0;
            case 'e':
            case 'p':
                effect_name = optarg;
                break;
            case 'c':
                client_name = optarg;
                break;
            case 'i':
                input_name = optarg;
                break;
            case 'o':
                output_name = optarg;
                break;
            case 'm':
                midi_name = optarg;
                break;
        }
    }
    try {
        struct stat st;
        if (!stat(get_preset_filename().c_str(), &st))
            load_presets(get_preset_filename().c_str());
        else if (!stat(PKGLIBDIR "/presets.xml", &st))
            load_presets(PKGLIBDIR "/presets.xml");
        jack_host_base *jh = NULL;
        if (!strcmp(effect_name, "reverb"))
            jh = new jack_host<reverb_audio_module>();
        else if (!strcmp(effect_name, "flanger"))
            jh = new jack_host<flanger_audio_module>();
        else if (!strcmp(effect_name, "filter"))
            jh = new jack_host<filter_audio_module>();
        else if (!strcmp(effect_name, "monosynth"))
            jh = new jack_host<monosynth_audio_module>();
#ifdef ENABLE_EXPERIMENTAL
        else if (!strcmp(effect_name, "organ"))
            jh = new jack_host<organ_audio_module>();
#endif
        else {
#ifdef ENABLE_EXPERIMENTAL
            fprintf(stderr, "Unknown plugin name; allowed are: reverb, flanger, filter, monosynth, organ\n");
#else
            fprintf(stderr, "Unknown plugin name; allowed are: reverb, flanger, filter, monosynth\n");
#endif
            return 1;
        }
        jh->open(client_name, input_name, output_name, midi_name);
        gui_win = new plugin_gui_window;
        gui_win->create(jh, client_name, effect_name);
        gtk_signal_connect(GTK_OBJECT(gui_win->toplevel), "destroy", G_CALLBACK(destroy), NULL);
        gtk_main();
        delete gui_win;
        jh->close();
        save_presets(get_preset_filename().c_str());
        delete jh;
    }
    catch(std::exception &e)
    {
        fprintf(stderr, "%s\n", e.what());
        exit(1);
    }
    return 0;
}
