#pragma once
#include "graph.h"
#include "linear.h"
#include "model.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ranges>
#include <tuple>
#include <algorithm>

float edgeFunction(const Pixel2D& a, const Pixel2D& b, Pixel2D& p) {
    return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

float perspectiveCorrectedDepth(float z0, float z1, float z2, 
                              float w0_weight, float w1_weight, float w2_weight,
                              float bary0, float bary1, float bary2) {
    float corrected_w0 = bary0 * w0_weight;
    float corrected_w1 = bary1 * w1_weight;
    float corrected_w2 = bary2 * w2_weight;
    
    float numerator = corrected_w0 * z0 + corrected_w1 * z1 + corrected_w2 * z2;
    float denominator = corrected_w0 + corrected_w1 + corrected_w2;
    
    if (std::abs(denominator) < 1e-6f) {
        return (z0 + z1 + z2) / 3.0f; 
    }
    
    return numerator / denominator;
}

Point2D perspectiveCorrectedUV(Point2D uv0, Point2D uv1, Point2D uv2,
                           float invW0, float invW1, float invW2,
                           float bary0, float bary1, float bary2) {
    
    // 透视校正UV插值
    float denominator = bary0 * invW0 + bary1 * invW1 + bary2 * invW2;
    
    if (std::abs(denominator) < 1e-6f) {
        // 退化情况，返回简单平均
        return (uv0 + uv1 + uv2) / 3.0f;
    }
    
    float u = (bary0 * uv0.x * invW0 + bary1 * uv1.x * invW1 + bary2 * uv2.x * invW2) / denominator;
    float v = (bary0 * uv0.y * invW0 + bary1 * uv1.y * invW1 + bary2 * uv2.y * invW2) / denominator;
    
    return Point2D(u, v);
}

Pixel2D ndcToScreen(const vec3& ndc, int width, int height){
    int x = static_cast<int>((ndc.x + 1.0f) * 0.5f * width);
    int y = static_cast<int>((1.0f - (ndc.y + 1.0f) * 0.5f) * height); // 注意y轴翻转
    return Pixel2D(x, y);
}

vec3 homoToNdc(const vec4& homo){
    return homo.homogenizate();
   
}

bool shouldCullTriangle(const Pixel2D p1, const Pixel2D& p2, const Pixel2D& p3, 
                       int screen_width, int screen_height) {
    auto [minX, maxX] = std::minmax({p1.x, p2.x, p3.x});
    auto [minY, maxY] = std::minmax({p1.y, p2.y, p3.y});
    
    if (maxX < 0 || minX >= screen_width || maxY < 0 || minY >= screen_height) {
        return true;
    }
    
    return false;
}

bool isTriangleInNDC(const vec3& a, const vec3& b, const vec3& c) {
    constexpr float NEAR_PLANE = -1.0f;
    constexpr float FAR_PLANE = 1.0f;
    
    auto inRange = [](float z) { 
        return z >= NEAR_PLANE && z <= FAR_PLANE; 
    };
    
    int vertices_in_frustum = (inRange(a.z) ? 1 : 0) + 
                             (inRange(b.z) ? 1 : 0) + 
                             (inRange(c.z) ? 1 : 0);
    
    return vertices_in_frustum > 0; // 至少有一个顶点在视锥内
}

bool render(const model::Model& render_model, const texture::Texture& render_texture, Picture& image, Matrix& depthBuffer){
    depthBuffer.fill(1.f);
    image.fill(0);

    vec3 lightDir = vec3(1, 2, 3).normalize();

    struct TriangleBounds {
        int minX, maxX, minY, maxY;
        bool valid;
    };
    std::vector<TriangleBounds> triangleBounds(render_model.verticle_idx.size());

    std::vector<float> w_weights(render_model.transfromed_vertices.size());
    std::vector<vec3> ndc_points(render_model.transfromed_vertices.size());
    for (size_t idx = 0; idx < render_model.transfromed_vertices.size(); ++idx) {
        float w = render_model.transfromed_vertices[idx].w;
        if (std::abs(w) < 1e-6f || std::isnan(w) || std::isinf(w)) {
            w_weights[idx] = 1.0f / 1e-6f;
            ndc_points[idx] = vec3(0,0,1);
        } else {
            w_weights[idx] = 1.0f / w;
            ndc_points[idx] = homoToNdc(render_model.transfromed_vertices[idx]);
        }
    }

    for (size_t i = 0; i < render_model.verticle_idx.size(); ++i) {
        auto posIdx = render_model.verticle_idx[i];
        
        auto pa_ndc = ndc_points[posIdx.v0];
        auto pb_ndc = ndc_points[posIdx.v1];
        auto pc_ndc = ndc_points[posIdx.v2];

        if (!isTriangleInNDC(pa_ndc, pb_ndc, pc_ndc)) {
            triangleBounds[i] = {-1, -1, -1, -1, false};
            continue;
        }
        
        auto pa_screen = ndcToScreen(pa_ndc, image.width(), image.height());
        auto pb_screen = ndcToScreen(pb_ndc, image.width(), image.height());
        auto pc_screen = ndcToScreen(pc_ndc, image.width(), image.height());
        
        if (shouldCullTriangle(pa_screen, pb_screen, pc_screen, 
                              image.width(), image.height())) {
            triangleBounds[i] = {-1, -1, -1, -1, false};
            continue;
        }

        auto [minX, maxX] = std::minmax({pa_screen.x,pb_screen.x,pc_screen.x});
        auto [minY, maxY] = std::minmax({pa_screen.y,pb_screen.y,pc_screen.y});

        triangleBounds[i] = {
            max(static_cast<int>(minX), 0),
            min(static_cast<int>(maxX), image.width() - 1),
            max(static_cast<int>(minY), 0),
            min(static_cast<int>(maxY), image.height() - 1),
            true
        };
    }
    for (size_t i = 0; i < render_model.verticle_idx.size(); ++i) {
         if (!triangleBounds[i].valid) continue;

        auto posIdx = render_model.verticle_idx[i];
        auto bounds = triangleBounds[i];

        std::optional<model::Triangle> texIdx;
        std::optional<model::Triangle> normIdx;

        if (i < render_model.texture_idx.size())
            texIdx = render_model.texture_idx[i];
        if (i < render_model.normal_idx.size())
            normIdx = render_model.normal_idx[i];

        auto pa_ndc = ndc_points[posIdx.v0];
        auto pb_ndc = ndc_points[posIdx.v1];
        auto pc_ndc = ndc_points[posIdx.v2];

        if (!isTriangleInNDC(pa_ndc, pb_ndc, pc_ndc)) {
            continue;
        }
        
        auto pa_screen = ndcToScreen(pa_ndc, image.width(), image.height());
        auto pb_screen = ndcToScreen(pb_ndc, image.width(), image.height());
        auto pc_screen = ndcToScreen(pc_ndc, image.width(), image.height());
        

        float area_screen = edgeFunction(pa_screen, pb_screen, pc_screen);
        const  float inv_area = 1.0f / area_screen;
        if (std::abs(area_screen) < 1e-6f) continue;
 
        for(int y = bounds.minY; y <= bounds.maxY; ++y){
            //auto img_row = image.row_ptr(y);s
            for(int x = bounds.minX; x <= bounds.maxX; ++x){  
                Pixel2D pixel(x,y);
                float w0 = edgeFunction(pb_screen, pc_screen, pixel) * inv_area;
                float w1 = edgeFunction(pc_screen, pa_screen, pixel) * inv_area; 
                float w2 = edgeFunction(pa_screen, pb_screen, pixel) * inv_area;
                
                if (w0 < 0 || w1 < 0 || w2 < 0) continue;
                
                auto interpolated_depth = perspectiveCorrectedDepth(pa_ndc.z, pb_ndc.z, pc_ndc.z, 
                        w_weights[posIdx.v0], w_weights[posIdx.v1], w_weights[posIdx.v2],
                        w0, w1, w2);
                
                int buf_x = x, buf_y = y;
                    float buffered_depth = depthBuffer(buf_y, buf_x);
                    
                if (interpolated_depth >= buffered_depth) {
                    continue;
                }
            
                vec3 render_color(255,255,255);
                if (texIdx.has_value() && render_texture.isLoaded) {
                    auto [t0, t1, t2] = texIdx.value();
                    auto texCoordA = render_model.texcoords[t0];
                    auto texCoordB = render_model.texcoords[t1];
                    auto texCoordC = render_model.texcoords[t2];
                    auto correct_uv = perspectiveCorrectedUV(
                    texCoordA, texCoordB, texCoordC,
                    w_weights[posIdx.v0], w_weights[posIdx.v1], w_weights[posIdx.v2],
                    w0, w1, w2
                    );
                    render_color = render_texture.getColor(correct_uv.x, correct_uv.y);
                }

                if (normIdx.has_value()) {
                    auto [n0, n1, n2] = normIdx.value();
                    vec3 na = render_model.vertex_norm[n0];
                    vec3 nb = render_model.vertex_norm[n1];
                    vec3 nc = render_model.vertex_norm[n2];

                    vec3 faceNormal = ((na + nb + nc) / 3.0f).normalize();
                    float brightness = std::clamp(faceNormal.dot(lightDir),0.2f,1.f);
                    render_color = render_color * brightness;
                }
                
                depthBuffer(buf_y, buf_x) = interpolated_depth;
                // img_row[x] = static_cast<uint8_t>(render_color.x);
                // img_row[x + 1] = static_cast<uint8_t>(render_color.y);
                // img_row[x + 2] = static_cast<uint8_t>(render_color.z);
                image.at(x,y,0) = static_cast<uint8_t>(render_color.x);
                image.at(x,y,1) = static_cast<uint8_t>(render_color.y);
                image.at(x,y,2) = static_cast<uint8_t>(render_color.z);

            }
        }
    }
    return true;
}
