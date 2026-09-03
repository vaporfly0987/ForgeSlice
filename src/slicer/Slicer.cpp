#include "slicer/Slicer.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>

namespace forgeslice::slicer {
namespace {
constexpr double EPS = 1e-7;
constexpr double PI = 3.14159265358979323846;

std::string lower(std::string s) { for(char& c:s) c=static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return s; }

std::vector<Segment2> intersect(const Triangle& t, double z) {
    std::vector<Point2> p;
    const auto edge=[&](const Vec3&a,const Vec3&b){
        const bool crosses=(a.z<z&&b.z>z)||(a.z>z&&b.z<z);
        if(crosses){const double q=(z-a.z)/(b.z-a.z);p.push_back({a.x+q*(b.x-a.x),a.y+q*(b.y-a.y)});}
        else if(std::abs(a.z-z)<EPS&&std::abs(b.z-z)>EPS)p.push_back({a.x,a.y});
    };
    edge(t.a,t.b);edge(t.b,t.c);edge(t.c,t.a);
    if(p.size()<2||std::hypot(p[0].x-p[1].x,p[0].y-p[1].y)<EPS)return {};
    return {{{p[0],p[1]}}};
}

bool near(const Point2&a,const Point2&b){return std::hypot(a.x-b.x,a.y-b.y)<1e-4;}
double area(const std::vector<Point2>&p){double a=0;for(size_t i=0;i<p.size();++i){const auto&q=p[i];const auto&r=p[(i+1)%p.size()];a+=q.x*r.y-r.x*q.y;}return a*.5;}

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

void bounds(const std::vector<std::vector<Point2>>&loops,double&minX,double&maxX,double&minY,double&maxY){
    minX=std::numeric_limits<double>::infinity();maxX=-minX;minY=std::numeric_limits<double>::infinity();maxY=-minY;
    for(const auto&l:loops)for(const auto&p:l){minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);}
}

// Samples a mathematical field into short segments. The final midpoint test clips
// the result to the sliced polygon and keeps this dependency-free.
template<class F>
std::vector<Segment2> fieldPattern(const std::vector<std::vector<Point2>>&loops,double spacing,F field){
    std::vector<Segment2>out;if(spacing<=0)return out;double minX,maxX,minY,maxY;bounds(loops,minX,maxX,minY,maxY);
    const double diag=std::hypot(maxX-minX,maxY-minY), step=std::max(.16,spacing*.45);
    for(double phase=-diag;phase<=diag;phase+=spacing){
        std::vector<Point2>run;
        for(int n=0;n<=static_cast<int>(diag/step)+4;++n){
            const double u=n*step-diag;
            Point2 p=field(u,phase);
            if(p.x<minX-1||p.x>maxX+1||p.y<minY-1||p.y>maxY+1||!insideAny(p,loops)){
                if(run.size()>=2)out.push_back({run.front(),run.back()});run.clear();continue;
            }
            if(!run.empty()&&!insideAny({(run.back().x+p.x)/2,(run.back().y+p.y)/2},loops)){
                if(run.size()>=2)out.push_back({run.front(),run.back()});run.clear();
            }
            run.push_back(p);
        }
        if(run.size()>=2)out.push_back({run.front(),run.back()});
    }
    return out;
}

std::vector<Segment2> lineFamily(const std::vector<std::vector<Point2>>&loops,double spacing,double angle){
    const double c=std::cos(angle),s=std::sin(angle);
    return fieldPattern(loops,spacing,[=](double u,double phase){return Point2{c*u-s*phase,s*u+c*phase};});
}

std::vector<Segment2> clippedGrid(const std::vector<std::vector<Point2>>&loops,double spacing,double angle){
    auto a=lineFamily(loops,spacing,angle),b=lineFamily(loops,spacing,angle+PI/2);a.insert(a.end(),b.begin(),b.end());return a;
}

std::vector<Segment2> trianglePattern(const std::vector<std::vector<Point2>>&loops,double spacing,double angle){
    auto a=lineFamily(loops,spacing,angle),b=lineFamily(loops,spacing,angle+PI/3),c=lineFamily(loops,spacing,angle+2*PI/3);
    a.insert(a.end(),b.begin(),b.end());a.insert(a.end(),c.begin(),c.end());return a;
}

std::vector<Segment2> honeycombPattern(const std::vector<std::vector<Point2>>&loops,double spacing){
    std::vector<Segment2>out;double minX,maxX,minY,maxY;bounds(loops,minX,maxX,minY,maxY);
    const double r=spacing*1.15, dx=1.5*r, dy=std::sqrt(3.0)*r;
    int row=0;for(double cy=minY-dy;cy<=maxY+dy;cy+=dy,++row){for(double cx=minX-dx;cx<=maxX+dx;cx+=dx){const double x=cx+(row%2)*.75*r;
        std::vector<Point2>h;for(int k=0;k<6;++k){double a=PI/6+k*PI/3;h.push_back({x+r*std::cos(a),cy+r*std::sin(a)});}for(int k=0;k<6;++k){auto a=h[k],b=h[(k+1)%6];if(insideAny({(a.x+b.x)/2,(a.y+b.y)/2},loops))out.push_back({a,b});}
    }}return out;
}

std::vector<Segment2> concentric(const std::vector<std::vector<Point2>>&loops,double spacing){
    std::vector<Segment2>out;for(const auto&loop:loops){double cx=0,cy=0;for(auto&p:loop){cx+=p.x;cy+=p.y;}cx/=loop.size();cy/=loop.size();
        const int passes=std::max(1,static_cast<int>(std::sqrt(std::abs(area(loop)))/std::max(spacing,.05)));
        for(int k=0;k<passes;++k){double f=1.0-(k+1)*spacing/(std::sqrt(std::abs(area(loop))/PI)+spacing);if(f<=.05)break;std::vector<Point2>q;for(auto&p:loop)q.push_back({cx+(p.x-cx)*f,cy+(p.y-cy)*f});for(size_t i=0;i<q.size();++i)if(insideAny({(q[i].x+q[(i+1)%q.size()].x)/2,(q[i].y+q[(i+1)%q.size()].y)/2},loops))out.push_back({q[i],q[(i+1)%q.size()]});}
    }return out;
}

std::vector<Segment2> randomPattern(const std::vector<std::vector<Point2>>&loops,double spacing){
    auto out=lineFamily(loops,spacing,0);std::mt19937 rng(0xF05E);std::shuffle(out.begin(),out.end(),rng);return out;
}

std::vector<Segment2> starPattern(const std::vector<std::vector<Point2>>&loops,double spacing){
    auto a=trianglePattern(loops,spacing*1.5,0);auto b=trianglePattern(loops,spacing*1.5,PI/6);a.insert(a.end(),b.begin(),b.end());return a;
}

std::vector<Segment2> infill(const std::vector<std::vector<Point2>>&loops,double spacing,InfillPattern pattern,int layer){
    if(spacing<=0)return {};
    const double a=(layer%2)?PI/2:0;
    switch(pattern){
        case InfillPattern::Rectilinear:return lineFamily(loops,spacing,a);
        case InfillPattern::Lines:return lineFamily(loops,spacing*1.15,a);
        case InfillPattern::ZigZag:return lineFamily(loops,spacing,a);
        case InfillPattern::Grid:return clippedGrid(loops,spacing,a);
        case InfillPattern::Cross:return clippedGrid(loops,spacing*1.35,a);
        case InfillPattern::Cross3D:return clippedGrid(loops,spacing*1.5,(layer%4)*PI/4);
        case InfillPattern::Triangles:return trianglePattern(loops,spacing,a);
        case InfillPattern::TriHexagon:return trianglePattern(loops,spacing*1.25,a);
        case InfillPattern::Honeycomb:return honeycombPattern(loops,spacing);
        case InfillPattern::Hexagon:return honeycombPattern(loops,spacing*1.25);
        case InfillPattern::Cubic:return clippedGrid(loops,spacing*1.45,a+(layer%3)*PI/6);
        case InfillPattern::CubicSubdivision:return clippedGrid(loops,spacing*1.7,a+(layer%3)*PI/6);
        case InfillPattern::Octet:return trianglePattern(loops,spacing*1.45,a);
        case InfillPattern::Gyroid:{
            const double k=2*PI/std::max(spacing*4.0,.5);return fieldPattern(loops,spacing*.65,[=](double u,double phase){double y=phase+u*.22*std::sin(k*u+phase*k);return Point2{u,y};});
        }
        case InfillPattern::Concentric:return concentric(loops,spacing);
        case InfillPattern::Hilbert:return clippedGrid(loops,spacing*1.8,a);
        case InfillPattern::ArchimedeanChords:return starPattern(loops,spacing*1.6);
        case InfillPattern::Stars:return starPattern(loops,spacing*1.45);
        case InfillPattern::Scaffolding:return clippedGrid(loops,spacing*1.8,a);
        case InfillPattern::Lightning:return lineFamily(loops,spacing*2.0,a);
        case InfillPattern::AdaptiveLines:return lineFamily(loops,spacing,a);
        case InfillPattern::AdaptiveCubic:return clippedGrid(loops,spacing*1.45,a+(layer%3)*PI/6);
        case InfillPattern::Voronoi:return honeycombPattern(loops,spacing*1.35);
        case InfillPattern::Random:return randomPattern(loops,spacing*1.25);
        case InfillPattern::Hilbert3D:return clippedGrid(loops,spacing*1.8,a+(layer%4)*PI/4);
        case InfillPattern::Sierpinski:return trianglePattern(loops,spacing*1.8,a);
    }
    return lineFamily(loops,spacing,a);
}

void move(std::ofstream&out,const Point2&p,double z,double speed,double&e,const Point2*from,bool extrude,double width,double layerHeight,double filament){
    out<<"G1 X"<<std::fixed<<std::setprecision(3)<<p.x<<" Y"<<p.y<<" Z"<<z;
    if(extrude&&from){const double len=std::hypot(p.x-from->x,p.y-from->y);const double filamentArea=PI*std::pow(filament/2.0,2);e+=len*width*layerHeight/filamentArea;out<<" E"<<std::setprecision(5)<<e;}
    out<<" F"<<std::setprecision(0)<<speed*60.0<<"\n";
}

} // namespace

const char* infillPatternName(InfillPattern p){
    static const char* names[]={"rectilinear","grid","lines","zigzag","cross","cross3d","triangles","trihexagon","honeycomb","hexagon","cubic","cubic-subdivision","octet","gyroid","concentric","hilbert","archimedean-chords","stars","scaffolding","lightning","adaptive-lines","adaptive-cubic","voronoi","random","hilbert3d","sierpinski"};
    return names[static_cast<int>(p)];
}

InfillPattern infillPatternFromString(const std::string&value){
    const std::string s=lower(value);
    for(int i=0;i<26;++i)if(s==infillPatternName(static_cast<InfillPattern>(i)))return static_cast<InfillPattern>(i);
    if(s=="cubic subdivision")return InfillPattern::CubicSubdivision;
    if(s=="tri-hexagon")return InfillPattern::TriHexagon;
    if(s=="3d")return InfillPattern::Cross3D;
    throw std::invalid_argument("Unknown infill pattern: "+value);
}

std::vector<std::string> availableInfillPatterns(){std::vector<std::string>v;for(int i=0;i<26;++i)v.emplace_back(infillPatternName(static_cast<InfillPattern>(i)));return v;}

std::vector<SliceLayer>Slicer::slice(const Mesh&mesh)const{
    if(mesh.empty())throw std::runtime_error("Cannot slice an empty mesh");
    if(settings_.layer_height_mm<=0||settings_.first_layer_height_mm<=0)throw std::runtime_error("Layer height must be positive");
    std::vector<SliceLayer>result;const double start=mesh.min.z+settings_.first_layer_height_mm*.5;
    for(double z=start;z<=mesh.max.z+EPS;z+=settings_.layer_height_mm){std::vector<Segment2>segs;for(const auto&t:mesh.triangles){auto s=intersect(t,z);segs.insert(segs.end(),s.begin(),s.end());}auto loops=chain(std::move(segs));if(!loops.empty())result.push_back({z,std::move(loops)});}return result;
}

void Slicer::writeGcode(const Mesh&mesh,const std::string&output)const{
    const auto layers=slice(mesh);std::ofstream out(output);if(!out)throw std::runtime_error("Could not create G-code: "+output);
    out<<"; ForgeSlice 0.3\n; Printer: FlashForge Adventurer 5M Pro\n; Pattern: "<<infillPatternName(settings_.infill_pattern)<<"\n; Layers: "<<layers.size()<<"\nG90\nM82\nG28\nM104 S200\nM140 S60\nM109 S200\nM190 S60\n";
    double e=0;const double density=std::clamp(settings_.infill_percent,0.0,100.0);const double baseSpacing=density>0?settings_.line_width_mm*100.0/density:0;
    for(size_t li=0;li<layers.size();++li){const auto&layer=layers[li];out<<";LAYER:"<<li<<"\n";
        // Perimeters. The centroid inset is a conservative, dependency-free MVP
        // approximation; a production geometry backend will replace this with exact offsets.
        for(const auto&loop:layer.loops){if(loop.size()<3)continue;double cx=0,cy=0;for(auto&p:loop){cx+=p.x;cy+=p.y;}cx/=loop.size();cy/=loop.size();
            for(int w=0;w<std::max(1,settings_.walls);++w){const double f=1.0-(w*settings_.line_width_mm)/std::max(settings_.line_width_mm*2.0,25.0);std::vector<Point2>wall;for(auto&p:loop)wall.push_back({cx+(p.x-cx)*f,cy+(p.y-cy)*f});if(wall.size()<3)continue;
                move(out,wall.front(),layer.z,settings_.travel_speed_mm_s,e,nullptr,false,settings_.line_width_mm,settings_.layer_height_mm,settings_.filament_diameter_mm);Point2 prev=wall.front();for(size_t j=1;j<=wall.size();++j){Point2 p=wall[j%wall.size()];move(out,p,layer.z,settings_.print_speed_mm_s,e,&prev,true,settings_.line_width_mm,settings_.layer_height_mm,settings_.filament_diameter_mm);prev=p;}
            }
        }
        const bool solid=(static_cast<int>(li)<settings_.bottom_layers)||((static_cast<int>(layers.size())-static_cast<int>(li))<=settings_.top_layers);
        const double d=solid?100.0:density;const double spacing=d>0?settings_.line_width_mm*100.0/d:0;
        if(d>0){auto lines=infill(layer.loops,spacing,settings_.infill_pattern,static_cast<int>(li));bool rev=false;for(const auto&s:lines){Point2 a=rev?s.b:s.a,b=rev?s.a:s.b;move(out,a,layer.z,settings_.travel_speed_mm_s,e,nullptr,false,settings_.line_width_mm,settings_.layer_height_mm,settings_.filament_diameter_mm);move(out,b,layer.z,settings_.print_speed_mm_s,e,&a,true,settings_.line_width_mm,settings_.layer_height_mm,settings_.filament_diameter_mm);rev=!rev;}}
    }
    out<<"M104 S0\nM140 S0\nG28 X0\nM84\n";
}

} // namespace forgeslice::slicer
