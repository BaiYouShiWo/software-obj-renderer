#pragma once 

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <format>
#include "linear.h"

class Model;

namespace model{

    class Triangle{
    public:
        unsigned int v0, v1, v2;
        Triangle(unsigned int a, unsigned int b, unsigned int c) : v0(a),v1(b),v2(c) {}
    };

    struct MeshSoA {
    std::vector<float> x, y, z;
    };

    class Model {
    public:
        Model() = default;
        Model(Model&& other) noexcept = default;
        Model& operator=(Model&& other) noexcept = default;
        std::vector<vec3> vertices;
        std::vector<vec3> vertex_norm;

        std::vector<Point2D> texcoords;

        std::vector<Triangle> verticle_idx;
        std::vector<Triangle> texture_idx;
        std::vector<Triangle> normal_idx;
        bool isLoaded = false;

        std::vector<vec4> transfromed_vertices;
        MeshSoA mesh;
    }; 



    inline MeshSoA to_soa(const std::vector<vec3>& vertices) {
    const size_t n = vertices.size();
    MeshSoA out;
    out.x.reserve(n); out.y.reserve(n); out.z.reserve(n);
    for (const auto& v : vertices) {
        out.x.push_back(v.x);
        out.y.push_back(v.y);
        out.z.push_back(v.z);
    }
    return out;
}
    Model loadModel(const std::string& filename){
        Model model_dst;
        std::ifstream  fin;
        fin.open(filename,std::ios::in);
        if (!fin){ 
            std::cerr << "Fail to open file" << std::endl;
            model_dst.isLoaded = false;
            return model_dst;
        }

        model_dst.vertices.clear();
        model_dst.vertex_norm.clear();
        model_dst.texcoords.clear();
        model_dst.verticle_idx.clear();
        model_dst.texture_idx.clear();
        model_dst.normal_idx.clear();

        std::string line;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string token;
            iss >> token;
            if (token == "v") {
                float x, y, z;
                if (iss >> x >> y >> z) {
                    model_dst.vertices.emplace_back(x, y, z);
                } else {
                    std::cerr << "Invalid verticle: " << line << std::endl;
                }

            }else if (token == "vn") {
                float x, y, z;
                if (iss >> x >> y >> z) {
                    model_dst.vertex_norm.emplace_back(x, y, z);
                } else {
                    std::cerr << "Invalid verticle: " << line << std::endl;
                }

            }else if (token == "vt") {
                float x, y;
                if (iss >> x >> y) {
                    model_dst.texcoords.emplace_back(x, y);
                } else {
                    std::cerr << "Invalid verticle: " << line << std::endl;
                }
            }else if (token == "f") {
                std::string faceToken;

                std::vector<unsigned int> v_idx;
                std::vector<unsigned int> vt_idx;
                std::vector<unsigned int> vn_idx;

                while (iss >> faceToken) {
                    int slashCount = std::count(faceToken.begin(), faceToken.end(), '/');
                    if(slashCount == 0){
                        unsigned int v = std::stoi(faceToken) - 1;
                        v_idx.push_back(v);
                        continue;
                    }else if (slashCount == 1) {
                        std::stringstream ss(faceToken);
                        unsigned int v = 0;
                        unsigned int vn = 0;
                        int fieldIdx = 0;
                        for (std::string line; std::getline(ss, line,'/');){
                            unsigned int idx = std::stoi(line);
                            if (idx == 0) {
                                std::cerr << "Invalid index 0 in OBJ\n";
                            }

                            if (fieldIdx == 0)      v = idx - 1;
                            else if (fieldIdx == 1) vn = idx - 1;
                            fieldIdx++;
                        }
                        v_idx.push_back(v);
                        vn_idx.push_back(vn);
                    }else if(slashCount == 2){
                        std::stringstream ss(faceToken);
                        unsigned int v = 0;
                        std::optional<unsigned int> vn;
                        std::optional<unsigned int> vt;
                        int fieldIdx = 0;
                        for (std::string line; std::getline(ss, line,'/');){
                            if (fieldIdx == 0)      v = std::stoi(line) - 1;
                            else if (fieldIdx == 1 && line != "") vt = std::stoi(line) - 1;
                            else if (fieldIdx == 2 && line != "") vn = std::stoi(line) - 1;
                            fieldIdx++;
                        }
                        v_idx.push_back(v);
                        if(vt.has_value())vt_idx.push_back(vt.value());
                        if(vn.has_value())vn_idx.push_back(vn.value());
                    }
                }


                size_t n = v_idx.size();
                if (n >= 3) {
                    for (size_t i = 1; i + 1 < n; ++i) {
                        model_dst.verticle_idx.emplace_back(v_idx[0], v_idx[i], v_idx[i + 1]);

                        if (vt_idx.size() == n)
                            model_dst.texture_idx.emplace_back(vt_idx[0], vt_idx[i], vt_idx[i + 1]);

                        if (vn_idx.size() == n)
                            model_dst.normal_idx.emplace_back(vn_idx[0], vn_idx[i], vn_idx[i + 1]);
                    }
                }
            }else {
                continue;
            }
        }

        fin.close();
        printf("Veticles count:%d\n", model_dst.vertices.size());
        printf("Normal count:%d\n", model_dst.vertex_norm.size());
        printf("TextureCoord count:%d\n", model_dst.texcoords.size());
        printf("Face count:%d\n", model_dst.verticle_idx.size());
        model_dst.isLoaded = true;
        model_dst.transfromed_vertices.resize(model_dst.vertices.size());
        model_dst.mesh = to_soa(model_dst.vertices);
        return model_dst;
    }
}

#include <png.h>
namespace texture {

struct Texture {
    int width;
    int height;
    std::vector<vec3> data; // RGB format
    int isLoaded = false;

    vec3 getColor(float u, float v) const {
        if(data.empty()) return vec3(255,255,255); // white color as default
        
        int x = std::clamp(static_cast<int>(u * width), 0, width - 1);
        int y = std::clamp(static_cast<int>((1 - v) * height), 0, height - 1);
        
        if (x < 0 || x >= width || y < 0 || y >= height) {
            return vec3(255, 0, 255);
        }
        
        return data[y * width + x];
    }
};

class TextureLoadError : public std::runtime_error {
public:
    TextureLoadError(const std::string& message) : std::runtime_error(message) {}
};

namespace {
    bool isPNG(const uint8_t* data) {
        return png_sig_cmp(const_cast<uint8_t*>(data), 0, 8) == 0;
    }

    std::vector<uint8_t> readFileToMemory(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw TextureLoadError("Cannot open file: " + filename);
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            throw TextureLoadError("Failed to read file: " + filename);
        }
        
        return buffer;
    }

    void readPngFromMemory(png_structp png_ptr, png_bytep data, png_size_t length) {
        uint8_t** buffer_ptr = static_cast<uint8_t**>(png_get_io_ptr(png_ptr));
        std::memcpy(data, *buffer_ptr, length);
        *buffer_ptr += length;
    }
}

Texture loadTexture(const std::string& filename) {
    Texture texture;
    texture.width = 0;
    texture.height = 0;
    texture.data.clear();
    
    try {
        std::vector<uint8_t> file_data = readFileToMemory(filename);
        if (file_data.empty()) {
            throw TextureLoadError("File is empty: " + filename);
        }
        
        if (!isPNG(file_data.data())) {
            throw TextureLoadError("Not a valid PNG file: " + filename);
        }
        
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
                                                   nullptr, nullptr, nullptr);
        if (!png_ptr) {
            throw TextureLoadError("Failed to create PNG read structure");
        }
        
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            throw TextureLoadError("Failed to create PNG info structure");
        }
        
        if (setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            throw TextureLoadError("PNG decoding error: " + filename);
        }

        // skip PNG signature
        uint8_t* data_ptr = file_data.data() + 8; 
        png_set_read_fn(png_ptr, &data_ptr, readPngFromMemory);
        png_set_sig_bytes(png_ptr, 8); 
        
        png_read_info(png_ptr, info_ptr);
        
        texture.width = png_get_image_width(png_ptr, info_ptr);
        texture.height = png_get_image_height(png_ptr, info_ptr);
        png_byte color_type = png_get_color_type(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        
        if (bit_depth == 16) {
            png_set_strip_16(png_ptr);
        }
        
        if (color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png_ptr);
        }
        
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
            png_set_expand_gray_1_2_4_to_8(png_ptr);
        }
        
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png_ptr);
        } else if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY) {

            png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
        }
        

        if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {

        }
        

        png_read_update_info(png_ptr, info_ptr);
        

        int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
        std::vector<png_bytep> row_pointers(texture.height);
        texture.data.resize(texture.width * texture.height);
        

        for (int y = 0; y < texture.height; y++) {
            row_pointers[y] = new png_byte[rowbytes];
        }
        

        png_read_image(png_ptr, row_pointers.data());
        

        for (int y = 0; y < texture.height; y++) {
            png_bytep row = row_pointers[y];
            for (int x = 0; x < texture.width; x++) {

                png_byte* ptr = &(row[x * 4]); 
                
                if (png_get_rowbytes(png_ptr, info_ptr) == texture.width * 3) {
                    // RGB
                    texture.data[y * texture.width + x] = vec3(
                        ptr[0], ptr[1], ptr[2]
                    );
                } else {
                    // RGBA
                    texture.data[y * texture.width + x] = vec3(
                        ptr[0], ptr[1], ptr[2]
                    );
                }
            }
            delete[] row_pointers[y];
        }
        
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        
        std::cout << "Successfully loaded texture: " << filename 
                  << " (" << texture.width << "x" << texture.height << ")" << std::endl;
        texture.isLoaded = true;
    } catch (const TextureLoadError& e) {
        std::cerr << "Texture loading failed: " << e.what() << std::endl;
        // white as default
        texture.width = 1;
        texture.height = 1;
        texture.data.clear();
        texture.data.emplace_back(255, 255, 255);
        texture.isLoaded = false;

    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        texture.width = 1;
        texture.height = 1;
        texture.data.clear();
        texture.data.emplace_back(255, 0, 255); // red means error
        texture.isLoaded = false;

    }
    
    
    return texture;
}

// class RenderTarget{
//     model::Model model_;
//     texture::Texture texture_;
// };

} 