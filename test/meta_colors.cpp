#!/usr/bin/env cpx
#|
#(

// test forground ANSI colors, by generating code to print the color name in its color to the console
// using the color names as defined in the util library. See also color.cpp

string tu_ansi_colors="RED,YELLOW,GREEN,CYAN,BLUE,MAGENTA,WHITE,HRED,HYELLOW,HGREEN,HCYAN,HBLUE,HMAGENTA,HWHITE";
vector<string> colors;
if( STATEREPORT(Append_splitted(&colors,tu_ansi_colors)) ) {
    ThrowBreak("Could not split string with color names.");
}

INFO("#pragma once");
for( auto color: colors ) {
    INFO("INFO(",color,",",STREAM2STR('"'<<color<<'"'),");");
}
#)
