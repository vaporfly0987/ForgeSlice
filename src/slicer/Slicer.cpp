#include "slicer/Slicer.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <stdexcept>

namespace forgeslice::slicer {
namespace {
constexpr double EPS = 1e-7;

std::vector<Segment2> intersect(const Triangle& t, double z) {
    std::vector<Point2> p;
    const auto edge = [&](const Vec3& a, const Vec3& b) {
        const bool crosses = (a.z < z && b.z > z) || (a.z > z && b.z < z);
        if (crosses) {
            const double q = (z - a.z) / (b.z - a.z);
            p.push_back({a.x + q * (b.x - a.x), a.y + q * (b.y - a.y)});
        } else if (std::abs(a.z - z) < EPS && std::abs(b.z - z) > EPS) {
            p.push_back({a.x, a.y});
        }
    };
    edge(t.a,t.b); edge(t.b,t.c); edge(t.c,t.a);
    if (p.size() < 2) return {};
    if (std::hypot(p[0].x-p[1].x,p[0].y-p[1].y) < EPS) return {};
    return {{{p[0],p[1]}}};
}

bool near(const Point2& a,const Point2& b){return std::hypot(a.x-b.x,a.y-b.y)<1e-4;}

double area(const std::vector<Point2>& p){double a=0;for(size_t i=0;i<p.size();++i){auto&q=p[i];auto&r=p[(i+1)%p.size()];a+=q.x*r.y-r.x*q.y;}return a*0.5;}

std::vector<std::vector<Point2>> chain(std::vector<Segment2> segs){
    std::vector<std::vector<Point2>> loops;
    while(!segs.empty()){
        Segment2 s=segs.back();segs.pop_back();std::vector<Point2> loop{s.a,s.b};
        while(!near(loop.front(),loop.back())){
            bool found=false;
            for(size_t i=0;i<segs.size();++i){
                if(near(segs[i].a,loop.back())){loop.push_back(segs[i].b);segs.erase(segs.begin()+i);found=true;break;}
                if(near(segs[i].b,loop.back())){loop.push_back(segs[i].a);segs.erase(segs.begin()+i);found=true;break;}
            }
            if(!found)break;
        }
        if(loop.size()>=3&&near(loop.front(),loop.back()))loop.pop_back();
        if(loop.size()>=3&&std::abs(area(loop))>1e-8)loops.push_back(std::move(loop));
    }
    return loops;
}

bool inside(const Point2&p,const std::vector<Point2>&poly){
    bool c=false;for(size_t i=0,j=poly.size()-1;i<poly.size();j=i++){
        const auto&a=poly[i];const auto&b=poly[j];
        if(((a.y>p.y)!=(b.y>p.y))&&p.x<(b.x-a.x)*(p.y-a.y)/(b.y-a.y+1e-30)+a.x)c=!c;
    }return c;
}

bool insideAny(const Point2&p,const std::vector<std::vector<Point2>>&loops){bool c=false;for(const auto&l:loops)if(inside(p,l))c=!c;return c;}

std::vector<Segment2> infill(const std::vector<std::vector<Point2>>&loops,double spacing,bool diagonal){
    std::vector<Segment2> out;if(spacing<=0)return out;
    double minX=std::numeric_limits<double>::infinity(),maxX=-minX,minY=std::numeric_limits<double>::infinity(),maxY=-minY;
    for(const auto&l:loops)for(const auto&p:l){minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);}
    const double pad=spacing*2;minX-=pad;maxX+=pad;minY-=pad;maxY+=pad;
    const double span=std::hypot(maxX-minX,maxY-minY);
    for(double k=-span;k<=span;k+=std::max(spacing,0.05)){
        std::vector<Point2> points;
        if(!diagonal){const double x=minX+k;for(int n=0;n<800;++n){double y=minY+(maxY-minY)*n/799.0;Point2 p{x,y};if(insideAny(p,loops))points.push_back(p);}}
        else{const double c=minX+minY+k;for(int n=0;n<1000;++n){double x=minX+(maxX-minX)*n/999.0;Point2 p{x,c-x};if(insideAny(p,loops))points.push_back(p);}}
        if(points.size()>=2){size_t begin=0;for(size_t i=1;i<points.size();++i){if(!insideAny({(points[i-1].x+points[i].x)/2,(points[i-1].y+points[i].y)/2},loops)){if(i-1>begin)out.push_back({points[begin],points[i-1]});begin=i;}}if(points.size()-1>begin)out.push_back({points[begin],points.back()});}
    }return out;
}

void move(std::ofstream&out,const Point2&p,double z,double speed,double&e,const Point2*from,bool extrude,double width,double layerHeight,double filament){
    out<<"G1 X"<<std::fixed<<std::setprecision(3)<<p.x<<" Y"<<p.y<<" Z"<<z;
    if(extrude&&from){double len=std::hypot(p.x-from->x,p.y-from->y);double filamentArea=M_PI*std::pow(filament/2.0,2);e+=len*width*layerHeight/filamentArea;out<<" E"<<std::setprecision(5)<<e;}
    out<<" F"<<std::setprecision(0)<<speed*60.0<<"\n";
}
}

std::vector<SliceLayer>Slicer::slice(const Mesh&mesh)const{
    if(mesh.empty())throw std::runtime_error("Cannot slice an empty mesh");
    if(settings_.layer_height_mm<=0||settings_.first_layer_height_mm<=0)throw std::runtime_error("Layer height must be positive");
    std::vector<SliceLayer>result;
    const double start=mesh.min.z+settings_.first_layer_height_mm*0.5;
    for(double z=start;z<=mesh.max.z+EPS;z+=settings_.layer_height_mm){
        std::vector<Segment2>segs;for(const auto&t:mesh.triangles){auto s=intersect(t,z);segs.insert(segs.end(),s.begin(),s.end());}
        auto loops=chain(std::move(segs));if(!loops.empty())result.push_back({z,std::move(loops)});
    }return result;
}

void Slicer::writeGcode(const Mesh&mesh,const std::string&output)const{
    const auto layers=slice(mesh);std::ofstream out(output);if(!out)throw std::runtime_error("Could not create G-code: "+output);
    out<<"; ForgeSlice 0.2 functional MVP\n; Printer: FlashForge Adventurer 5M Pro\n; Layers: "<<layers.size()<<"\nG90\nM82\nG28\nM104 S200\nM140 S60\nM109 S200\nM190 S60\n";
    double e=0;const double density=std::clamp(settings_.infill_percent,0.0,100.0);const double spacing=density>0?settings_.line_width_mm*100.0/density:0;
    for(size_t li=0;li<layers.size();++li){const auto&layer=layers[li];out<<";LAYER:"<<li<<"\n";
        for(const auto&loop:layer.loops){if(loop.size()<3)continue;move(out,loop.front(),layer.z,settings_.travel_speed_mm_s,e,nullptr,false,settings_.line_width_mm,settings_.layer_height_mm,settings_.filament_diameter_mm);Point2 prev=loop.front();for(size_t j=1;j<=loop.size();++j){Point2 p=loop[j%loop.size()];move(out,p,layer.z,settings_.print_speed_mm_s,e,&prev,true,settings_.line_width_mm,settings_.layer_height_mm,settings_.filament_diameter_mm);prev=p;}}
        if(density>0){auto lines=infill(layer.loops,spacing,li%2);bool rev=false;for(const auto&s:lines){Point2 a=rev?s.b:s.a,b=rev?s.a:s.b;move(out,a,layer.z,settings_.travel_speed_mm_s,e,nullptr,false,settings_.line_width_mm,settings_.layer_height_mm,settings_.filament_diameter_mm);move(out,b,layer.z,settings_.print_speed_mm_s,e,&a,true,settings_.line_width_mm,settings_.layer_height_mm,settings_.filament_diameter_mm);rev=!rev;}}
    }out<<"M104 S0\nM140 S0\nG28 X0\nM84\n";
}

} // namespace forgeslice::slicer
