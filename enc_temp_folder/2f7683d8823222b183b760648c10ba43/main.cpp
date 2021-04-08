#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <vector>


/*
#include <complex>
// Might be faster than sqrt. Need to profile.
float FastSqrt(float val)
{
    __m128 reg = _mm_set1_ps(val);
    return _mm_cvtss_f32(_mm_rcp_ss(_mm_rsqrt_ss(reg)));
}
*/

struct vec3 {
    float x;
    float y;
    float z;

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    void normalize() {
        const auto l = length();
        x = x / l;
        y = y / l;
        z = z / l;
    }

    vec3 operator*(const float& fac) const {
        return vec3{x * fac, y * fac, z * fac};
    }

    vec3 operator+(const vec3& other) const {
        return vec3{x + other.x, y + other.y, z + other.z};
    }

    vec3 operator-(const vec3& other) const {
        return vec3{x - other.x, y - other.y, z - other.z};
    }

};

// Override base class with your custom functionality
class RayMarcher : public olc::PixelGameEngine {
private:
    float fAccumulatedTime = 0.0F;
    std::vector<std::vector<float>> frameBuffer;
    float height{};
    float width{};

public:
    RayMarcher() {
        // Name your application
        sAppName = "PGE Ray Marcher";
    }

    static float sdf(const vec3& pos) {
        const vec3 center = {0.0, 0.0, 0.0};
        return (pos - center).length() - 0.5F;
    }

    float shade(const vec3& pos, const float t) const {
        vec3 L = {
            20.0F * sin(t),
            20.0F,
            20.0F * cos(t)
        };
        L.normalize();

        const auto dt = 1e-6F;
        const auto current_val = sdf(pos);

        const vec3 x = {pos.x + dt, pos.y, pos.z};
        const auto dx = sdf(x) - current_val;

        const vec3 y = {pos.x, pos.y + dt, pos.z};
        const auto dy = sdf(y) - current_val;

        const vec3 z = {pos.x, pos.y, pos.z + dt};
        const auto dz = sdf(z) - current_val;

        vec3 N{
            (dx - pos.x) / dt,
            (dy - pos.y) / dt,
            (dz - pos.z) / dt
        };

        if (N.length() < 1e-9) {
            return 0;
        }

        N.normalize();

        const auto diffuse{L.x * N.x + L.y * N.y + L.z * N.z};
        return diffuse * 0.5F + 0.5F; // Remap from -1..1 to 0..1
    }

    void RayMarch(const float t) {
        for (auto y = 0; y < height; ++y) {
            for (auto x = 0; x < width; ++x) {
                vec3 pos = {0.0, 0.0, -3.0};
                // Map x and y to -0.5 to 0.5 and then correct y for aspect ratio.
                const struct vec3 target = {
                    x / width - 0.5F,
                    (y / height - 0.5F) * (height / width),
                    -1.5F
                };

                auto ray = (target - pos);
                ray.normalize();
                auto pxl = 0.0F;
                const auto max = 3.0F; // Started at 9999. We can dial down because there's not much depth to our scene.
                for (auto i = 0; i < 15000; i++) {
                    if (std::abs(pos.x) > max
                        || std::abs(pos.y) > max
                        || std::abs(pos.z) > max) {
                        break;
                    }

                    const auto dist = sdf(pos);
                    if (dist < 1e-3) {
                        // Started as 1e-6. Sphere gets chopping at 1e-2 and 1e-1
                        pxl = shade(pos, t);
                        break;
                    }

                    pos = pos + ray * dist;
                } // end for (i)
                frameBuffer[x][y] = pxl;
            } // end for(x)
        } // end for(y)
    } 


    bool OnUserCreate() override {
        height = ScreenHeight();
        width = ScreenWidth();

        frameBuffer.resize(width);
        for (auto i = 0; i < width; ++i) {
            frameBuffer[i].resize(height);
        }

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        // Erase previous frame
        Clear(olc::BLACK);
        fAccumulatedTime += fElapsedTime;
        RayMarch(fAccumulatedTime);

        for (auto y = 0; y < height; ++y) {
            for (auto x = 0; x < width; ++x) {
                const auto p = frameBuffer[x][y] * 255;
                Draw(x, y, olc::Pixel(p, 0, 0));
            }
        }
        return true;
    }
};

int main() {
    RayMarcher rayMarcher;
    if (rayMarcher.Construct(480, 360, 1, 1) != 0) {
        rayMarcher.Start();
    }
    return 0;
}
