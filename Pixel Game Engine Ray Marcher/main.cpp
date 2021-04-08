
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <vector>

struct vec3 {
	float x;
	float y;
	float z;

	float length() const {
		return sqrt(x * x + y * y + z * z);
	}

	void normalize() {
		float l = length();
		x = x / l; y = y / l; z = z / l;
	}

	struct vec3 operator*(float fac) const {
		struct vec3 r;
		r.x = x * fac; r.y = y * fac; r.z = z * fac;
		return r;
	}

	struct vec3 operator+(struct vec3 other) const {
		struct vec3 r;
		r.x = x + other.x; r.y = y + other.y; r.z = z + other.z;
		return r;
	}
	struct vec3 operator-(struct vec3 other) const {
		struct vec3 r;
		r.x = x - other.x; r.y = y - other.y; r.z = z - other.z;
		return r;
	}

};

// Override base class with your custom functionality
class BreakOut : public olc::PixelGameEngine
{
private:
	float fAccumulatedTime = 0.0f;
	std::vector<std::vector<double>> framebuffer;

public:
	BreakOut()
	{
		// Name your application
		sAppName = "Break Out";
	}

	float sdf(struct vec3 pos) {
		struct vec3 center = { 0.0, 0.0, 0.0 };

		return (pos - center).length() - 0.5;
	}

	float shade(struct vec3 pos, double t) {
		struct vec3 L = {
			20.0 * sin(t),
			20.0,
			20.0 * cos(t)
		};
		L.normalize();

		const float dt = 1e-6;
		const float current_val = sdf(pos);

		const struct vec3 x = { pos.x + dt, pos.y, pos.z };
		const float dx = sdf(x) - current_val;

		const struct vec3 y = { pos.x, pos.y + dt, pos.z };
		const float dy = sdf(y) - current_val;

		const struct vec3 z = { pos.x, pos.y, pos.z + dt };
		const float dz = sdf(z) - current_val;

		struct vec3 N; // N for normal
		N.x = (dx - pos.x) / dt;
		N.y = (dy - pos.y) / dt;
		N.z = (dz - pos.z) / dt;

		if (N.length() < 1e-9) {
			return 0;
		}

		N.normalize();

		const float diffuse = L.x * N.x + L.y * N.y + L.z * N.z;
		return pow(diffuse * 0.5 + 0.5, 0.4545) ; // Remap from -1..1 to 0..1
	}


	void RayMarch(double t) {
		const int height = ScreenHeight();
		const int width = ScreenWidth();

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				struct vec3 pos = { 0.0, 0.0, -3.0 };
				 // Map x and y to -0.5 to 0.5 and then correct y for aspect ratio.
				const struct vec3 target = {
					x / (float)width - 0.5f,
					(y / (float)height - 0.5f) * (height / (float)width) ,
						-1.5f
				};

				struct vec3 ray = (target - pos);
			    ray.normalize();
				double pxl = 0.0;
				const float max = 3.0f; // Started at 9999. We can dial down because there's not much depth to our scene.
				for (int i = 0; i < 15000; i++) {
					if (fabs(pos.x) > max
						|| fabs(pos.y) > max
						|| fabs(pos.z) > max)
						break;

					const float dist = sdf(pos);
					if (dist < 1e-3) { // Started as 1e-6. Shape gets chopping at 1e-2 and 1e-1
						pxl = shade(pos, t);
						break;
					}

					pos = pos + ray * dist;
				} // end for (i)

				framebuffer[x][y]= pxl;
			} // end for(x)
		} // end for(y)
	} // end raymarch()


	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		framebuffer.resize(ScreenWidth());
		for(auto i = 0; i < ScreenWidth(); ++i) {
			framebuffer[i].resize(ScreenHeight());
		}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Erase previous frame
		Clear(olc::BLACK);
		fAccumulatedTime += fElapsedTime;
		RayMarch(fAccumulatedTime);
		const int height = ScreenHeight();
		const int width = ScreenWidth();
        for (auto y = 0; y < height; ++y) {
			
            for (auto x = 0; x < width; ++x) {
				const double p = framebuffer[x][y] * 255;
				Draw(x, y, olc::Pixel(p, 0, 0));
		   }
		}
        return true;
	}
};

int main()
{
	BreakOut demo;
	if (demo.Construct(480, 360, 1, 1))
		demo.Start();
	return 0;
}