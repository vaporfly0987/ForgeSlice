#include "app/Application.hpp"
#include "slicer/Mesh.hpp"
#include "slicer/Slicer.hpp"
#include <iostream>
#include <string>

namespace { void usage(){
    std::cout<<"ForgeSlice 0.3\nUsage: ForgeSlice model.stl [options]\n\nOptions:\n"
    <<"  -o <file>                 Output G-code path\n"
    <<"  --layer-height <mm>       Layer height\n  --first-layer <mm>        First layer height\n"
    <<"  --walls <n>               Wall count\n  --infill <percent>        Infill density 0-100\n"
    <<"  --pattern <name>          Infill pattern\n  --bottom-layers <n>       Solid bottom layers\n"
    <<"  --top-layers <n>          Solid top layers\n  --retraction <mm>         Retraction setting\n"
    <<"  --speed <mm/s>            Print speed\n  --travel <mm/s>           Travel speed\n"
    <<"  --list-patterns           List all 26 built-in patterns\n  --help                    Show this help\n";
}}

int main(int argc,char**argv){
    if(argc<2)return forgeslice::app::Application{}.run();
    std::string input,output="ForgeSlice.gcode";forgeslice::slicer::SliceSettings settings;
    try{
        for(int i=1;i<argc;++i){std::string a=argv[i];
            if(a=="-o"&&i+1<argc)output=argv[++i];
            else if(a=="--layer-height"&&i+1<argc)settings.layer_height_mm=std::stod(argv[++i]);
            else if(a=="--first-layer"&&i+1<argc)settings.first_layer_height_mm=std::stod(argv[++i]);
            else if(a=="--walls"&&i+1<argc)settings.walls=std::stoi(argv[++i]);
            else if(a=="--infill"&&i+1<argc)settings.infill_percent=std::stod(argv[++i]);
            else if(a=="--pattern"&&i+1<argc)settings.infill_pattern=forgeslice::slicer::infillPatternFromString(argv[++i]);
            else if(a=="--bottom-layers"&&i+1<argc)settings.bottom_layers=std::stoi(argv[++i]);
            else if(a=="--top-layers"&&i+1<argc)settings.top_layers=std::stoi(argv[++i]);
            else if(a=="--retraction"&&i+1<argc)settings.retraction_mm=std::stod(argv[++i]);
            else if(a=="--speed"&&i+1<argc)settings.print_speed_mm_s=std::stod(argv[++i]);
            else if(a=="--travel"&&i+1<argc)settings.travel_speed_mm_s=std::stod(argv[++i]);
            else if(a=="--list-patterns"){for(const auto&p:forgeslice::slicer::availableInfillPatterns())std::cout<<p<<"\n";return 0;}
            else if(a=="--help"){usage();return 0;}
            else if(a.rfind("-",0)!=0)input=a;else{std::cerr<<"Unknown option: "<<a<<"\n";usage();return 2;}
        }
        if(input.empty()){usage();return 2;}if(settings.walls<1||settings.bottom_layers<0||settings.top_layers<0)throw std::invalid_argument("Invalid wall/solid layer count");
        const auto mesh=forgeslice::slicer::Mesh::loadAsciiStl(input);forgeslice::slicer::Slicer slicer(settings);const auto layers=slicer.slice(mesh);slicer.writeGcode(mesh,output);
        std::cout<<"Sliced "<<input<<" -> "<<output<<" ("<<layers.size()<<" layers, "<<forgeslice::slicer::infillPatternName(settings.infill_pattern)<<")\n";
    }catch(const std::exception&e){std::cerr<<"ForgeSlice error: "<<e.what()<<"\n";return 1;}return 0;
}
