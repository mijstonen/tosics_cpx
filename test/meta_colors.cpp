#!/usr/bin/env cpx
#|
#(
// test forground ANSI colors
string tu_ansi_colors="RED,YELLOW,GREEN,CYAN,BLUE,MAGENTA,WHITE,HRED,HYELLOW,HGREEN,HCYAN,HBLUE,HMAGENTA,HWHITE";
vector<string> colors;
if( STATEREPORT(append_splitted(&colors,tu_ansi_colors)) ) {
    ThrowBreak("Could not split string with color names.");
}
for( auto color: colors ) {
    //string color_name=STREAM2STR('"'<<color<<'"');
    //INFO("INFO","(",color,",",color_name,")",";");
    cout<< "INFO( "<<color<<", "<<'"'<<color<<'"'<<");\n";
}
#)
