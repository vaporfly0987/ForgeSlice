#include "slicer/Mesh.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace forgeslice::slicer {
namespace {

std::uint32_t readU32(const unsigned char* p) {
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}

float readF32(const unsigned char* p) {
    float value{};
    std::memcpy(&value, p, sizeof(value));
    return value;
}

bool plausibleBinaryStl(const std::string& path, std::uint32_t& count) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) return false;
    const auto size = in.tellg();
    if (size < 84) return false;
    in.seekg(80);
    unsigned char header[4]{};
    in.read(reinterpret_cast<char*>(header), 4);
    if (!in) return false;
    count = readU32(header);
    const std::uint64_t expected = 84ull + 50ull * count;
    return expected == static_cast<std::uint64_t>(size);
}

Mesh loadBinaryStl(const std::string& path, std::uint32_t count) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Could not open STL: " + path);
    std::array<unsigned char, 84> header{};
    in.read(reinterpret_cast<char*>(header.data()), header.size());
    if (!in) throw std::runtime_error("Invalid binary STL header: " + path);
    Mesh mesh;
    mesh.triangles.reserve(count);
    std::array<unsigned char, 50> record{};
    for (std::uint32_t i=0;i<count;++i) {
        in.read(reinterpret_cast<char*>(record.data()), record.size());
        if (!in) throw std::runtime_error("Unexpected end of binary STL: " + path);
        const auto vec=[&](std::size_t o){return Vec3{readF32(record.data()+o),readF32(record.data()+o+4),readF32(record.data()+o+8)};};
        mesh.triangles.push_back({vec(12),vec(24),vec(36)});
    }
    mesh.recalculateBounds();
    return mesh;
}
}

Mesh Mesh::loadAsciiStl(const std::string& path) {
    std::uint32_t count=0;
    if (plausibleBinaryStl(path,count)) return loadBinaryStl(path,count);
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Could not open STL: "+path);
    Mesh mesh; std::string line; std::vector<Vec3> vertices;
    while(std::getline(in,line)){
        std::istringstream ss(line); std::string word; ss>>word;
        if(word!="vertex")continue;
        Vec3 v;if(ss>>v.x>>v.y>>v.z)vertices.push_back(v);
        if(vertices.size()==3){mesh.triangles.push_back({vertices[0],vertices[1],vertices[2]});vertices.clear();}
    }
    mesh.recalculateBounds();
    if(mesh.empty())throw std::runtime_error("No STL triangles found: "+path);
    return mesh;
}

void Mesh::recalculateBounds(){
    if(triangles.empty()){min=max={};return;}
    const double inf=std::numeric_limits<double>::infinity();min={inf,inf,inf};max={-inf,-inf,-inf};
    for(const auto&t:triangles)for(const auto&v:{t.a,t.b,t.c}){min.x=std::min(min.x,v.x);min.y=std::min(min.y,v.y);min.z=std::min(min.z,v.z);max.x=std::max(max.x,v.x);max.y=std::max(max.y,v.y);max.z=std::max(max.z,v.z);}
}
}
