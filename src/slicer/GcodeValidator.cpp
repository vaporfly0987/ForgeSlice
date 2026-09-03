#include "slicer/GcodeValidator.hpp"
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

namespace forgeslice::slicer {
namespace {
constexpr double EPS = 1e-6;
void issue(GcodeValidationResult& r,int line,const std::string& msg){r.ok=false;r.issues.push_back({line,msg});}
bool valueAfter(const std::string&s,char key,double&out){auto p=s.find(key);if(p==std::string::npos)return false;const char*b=s.c_str()+p+1;char*e=nullptr;out=std::strtod(b,&e);return e!=b&&std::isfinite(out);}
std::string commandOnly(std::string s){auto p=s.find(';');if(p!=std::string::npos)s.resize(p);while(!s.empty()&&std::isspace((unsigned char)s.back()))s.pop_back();return s;}
}
GcodeValidationResult validateGcode(const std::string&gcode,const core::PrinterProfile&profile){
 GcodeValidationResult r;if(gcode.empty()){issue(r,0,"G-code is empty");return r;}
 double x=0,y=0,z=0,e=0,lastZ=-EPS;bool absXYZ=true,absE=true,motion=false,layer=false,home=false,extrusion=false,end=false;int layers=0,prevLayer=-1;
 std::istringstream in(gcode);std::string raw;int line=0;
 while(std::getline(in,raw)){++line;
  if(raw.rfind(";LAYER:",0)==0){layer=true;++layers;try{int n=std::stoi(raw.substr(7));if(n<=prevLayer)issue(r,line,"Layer markers are not strictly increasing");prevLayer=n;}catch(...){issue(r,line,"Malformed layer marker");}}
  std::string s=commandOnly(raw);if(s.empty())continue;std::istringstream ss(s);std::string cmd;ss>>cmd;std::string rest;std::getline(ss,rest);
  if(cmd=="G90"){absXYZ=true;continue;}if(cmd=="G91"){absXYZ=false;continue;}if(cmd=="M82"){absE=true;continue;}if(cmd=="M83"){absE=false;continue;}if(cmd=="G28"){home=true;continue;}
  if(cmd=="G92"){double v;if(valueAfter(rest,'E',v))e=v;if(valueAfter(rest,'X',v))x=v;if(valueAfter(rest,'Y',v))y=v;if(valueAfter(rest,'Z',v))z=v;continue;}
  if(cmd=="M104"||cmd=="M109"){double t;if(valueAfter(rest,'S',t)&&(t<0||t>profile.max_nozzle_temperature_c+EPS))issue(r,line,"Nozzle temperature exceeds printer profile limits");continue;}
  if(cmd=="M140"||cmd=="M190"){double t;if(valueAfter(rest,'S',t)&&(t<0||t>profile.max_bed_temperature_c+EPS))issue(r,line,"Bed temperature exceeds printer profile limits");continue;}
  if(cmd=="M84"){end=true;continue;}if(cmd=="M999"){issue(r,line,"Firmware reset/recovery command is not allowed");continue;}if(cmd!="G0"&&cmd!="G1")continue;
  motion=true;double nx=x,ny=y,nz=z,ne=e,f=0;bool hx=valueAfter(rest,'X',nx),hy=valueAfter(rest,'Y',ny),hz=valueAfter(rest,'Z',nz),he=valueAfter(rest,'E',ne),hf=valueAfter(rest,'F',f);
  if(!std::isfinite(nx)||!std::isfinite(ny)||!std::isfinite(nz)||!std::isfinite(ne)){issue(r,line,"Non-finite motion value");continue;}
  if(!absXYZ){if(hx)nx+=x;if(hy)ny+=y;if(hz)nz+=z;}if(!absE&&he)ne+=e;
  if(nx<-EPS||ny<-EPS||nx>profile.build_volume.width_mm+EPS||ny>profile.build_volume.depth_mm+EPS||nz<-EPS||nz>profile.build_volume.height_mm+EPS)issue(r,line,"Motion leaves the printer build volume");
  if(hf&&(f<=0||f>profile.max_travel_speed_mm_s*60+EPS))issue(r,line,"Feed rate exceeds printer profile limit or is invalid");
  if(he&&std::abs(ne-e)>1e-7)extrusion=true;if(hz&&nz+EPS<lastZ)issue(r,line,"Layer Z moved backwards");x=nx;y=ny;z=nz;e=ne;if(z>lastZ)lastZ=z;
 }
 if(!home)issue(r,0,"Missing homing command (G28)");if(!motion)issue(r,0,"No motion commands found");if(!layer)issue(r,0,"No layer markers found");if(!extrusion)issue(r,0,"No extrusion moves found");if(layers==0)issue(r,0,"No printable layers found");if(!end)issue(r,0,"Missing end-of-print command (M84)");return r;
}
} // namespace forgeslice::slicer
