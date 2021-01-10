#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl/glut.h"
#include "gmath.h"
#include <tuple>

#define MAX_DEPTH 3
#define H 600
#define W 800

unsigned char Image[H * W * 3];
std::vector<GSphere> SphereList;
std::vector<GLight> LightList;

// 콜백 함수 선언
void Render();
void Reshape(int w, int h);

void Img();
float Vec3Dot(GVec3 a, GVec3 b);
GVec3 RayTrace(GLine v, int depth);
typedef std::tuple<std::shared_ptr<GSphere>, float> findSphere;
findSphere nearest_intersected_surface(GLine ray);
GPos3 point_of_intersection(GLine ray, float t);
// 3. 반사
GVec3 reflection(GLine ray, GSphere S, GPos3 p);

int main(int argc, char **argv)
{
	// OpenGL 초기화, 윈도우 크기 설정, 디스플레이 모드 설정
	glutInit(&argc, argv);
	glutInitWindowSize(W, H);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// 윈도우 생성 및 콜백 함수 등록
	glutCreateWindow("RayTracer");
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);

	// 조명 배치
	LightList.push_back(GLight (
		GPos3(-300.0, 200.0, 100.0),
		GVec3(-300.0, 200.0, 100.0),
		GVec3(-300.0, 200.0, 100.0),
		GVec3(-300.0, 200.0, 100.0)
	));

	LightList.push_back(GLight(
		GPos3(200.0, -300.0, -580.0),
		GVec3(0.2, 0.2, 0.2),
		GVec3(0.7, 0.7, 0.7),
		GVec3(0.8, 0.8, 0.8)
	));

	// 구 배치

	// 파란색 구
	GSphere Sphere0 (GPos3(-50, -20, -350.0), 35.0f); 
	Sphere0.Ka.Set(0.2, 0.2, 0.8);
	Sphere0.Kd.Set(0.0, 0.0, 0.7);
	Sphere0.Ks.Set(0.9, 0.9, 0.9);
	Sphere0.ns = 8.0;
	SphereList.push_back(Sphere0);

	// 초록색 구
	GSphere Sphere1(GPos3(50, -45, -380.0), 35.0f);
	Sphere1.Ka.Set(0.8, 0.2, 0.2);
	Sphere1.Kd.Set(0.7, 0.0, 0.0);
	Sphere1.Ks.Set(0.9, 0.9, 0.9);
	Sphere1.ns = 8.0;
	SphereList.push_back(Sphere1);

	// 노란색 구
	GSphere Sphere2(GPos3(50, 50, -440.0), 35.0f);
	Sphere2.Ka.Set(0.8, 0.8, 0.2);
	Sphere2.Kd.Set(0.7, 0.7, 0.0);
	Sphere2.Ks.Set(0.9, 0.9, 0.9);
	Sphere2.ns = 8.0;
	SphereList.push_back(Sphere2);

	// 이미지를 생성
	Img();

	// 타이머 : 30밀리 초마다 노란색 구를 앞으로(z축) 5 이동(70 이동하면 멈춘다.)
	

	// 이벤트를 처리를 위한 무한 루프로 진입한다.
	//glutMainLoop();

	return 0;
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Render()
{
	// 칼라 버퍼와 깊이 버퍼 지우기
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 칼라 버퍼에 Image 데이터를 직접 그린다.
	glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, Image);

	// 칼라 버퍼 교환한다
	glutSwapBuffers();
}

void Img()
{
	int x0 = -W / 2; // -400
	int y0 = H / 2 - 1; // 299
	double z = -(H / 2) / tan(M_PI * 15 / 180.0);

	// 1번 테스트
	{
		GPos3 s0 = SphereList.at(0).Pos, s1 = SphereList.at(1).Pos, s2 = SphereList.at(2).Pos;
		GVec3 Color = RayTrace(GLine(GPos3(0.0, 0.0, 0.0), s0), 0); // YES
		Color = RayTrace(GLine(GPos3(0.0, 0.0, 0.0), s1), 0); // YES
		Color = RayTrace(GLine(GPos3(0.0, 0.0, 0.0), s2), 0); // YES

		GPos3 w0(5, 5, -400), w1(100, 100, -200);

		Color = RayTrace(GLine(GPos3(0.0, 0.0, 0.0), w0), 0); // NO
		Color = RayTrace(GLine(GPos3(0.0, 0.0, 0.0), w1), 0); // NO

		s0.V[1] += 10.0f;
		Color = RayTrace(GLine(GPos3(0.0, 0.0, 0.0), s0), 0); // YES
	}
	

	//for (int i = 0; i < H; ++i)
	//{
	//	for (int j = 0; j < W; ++j)
	//	{
	//		double x = x0 + j;
	//		double y = y0 - i;
	//		GLine ray(GPos3(0.0, 0.0, 0.0), GPos3(x, y, z));

	//		GVec3 Color = RayTrace(ray, 0);

	//		int idx = (H - 1 - i) * W * 3 + j * 3;

	//		for (int i = 0; i < 3; i++)
	//		{
	//			Image[idx + i] = (Color[i] > 1.0) ? 255 : Color[i] * 255;
	//		}
	//	}
	//}
}

GVec3 RayTrace(GLine v, int depth)
{
	GVec3 C(0, 0, 0);
	depth++;

	// 1. 교차하는 가장 가까운 구 찾기
	auto S = nearest_intersected_surface(v);

	auto &sphere = std::get<0>(S);
	auto &time = std::get<1>(S);

	if (sphere == nullptr || depth == MAX_DEPTH)
	{
		std::cout << "No" << std::endl;
		return C; // default color
	}

	// 2. 교차점 찾기
	auto p = point_of_intersection(v, time);

	// 3. 반사
	auto R = reflection(v, *sphere, p);

	//// 4. 굴절
	//T = refraction(v, S, p);

	//// 5. 퐁 쉐이딩
	//C = phong(p, S, v) + kreflect * ray_trace(R, d) + krefract * ray_trace(T, d);

	std::cout << "Yes" << std::endl;
	std::cout << sphere->Pos << std::endl;
	std::cout << time << std::endl;

	return C;
}

// 벡터 내적 
float Vec3Dot(GVec3 a, GVec3 b)
{
	return a.V[0] * b.V[0] + a.V[1] * b.V[1] + a.V[2] * b.V[2];
}
// 1. 교차하는 가장 가까운 구 찾기
findSphere nearest_intersected_surface(GLine ray)
{
	findSphere res = std::make_tuple(nullptr, std::numeric_limits<float>::max());

	for (auto const &sphere : SphereList)
	{
		// u : p0 - q(구 중심)
		GVec3 v = ray.GetDir(), u = ray.GetPt() - sphere.Pos;
		float uvDot = Vec3Dot(u, v), uDist = norm(u), r = sphere.Rad;

		if (uvDot * uvDot - (uDist * uDist - r * r) < 0) continue;

		float t = (-1) * uvDot - SQRT(uvDot * uvDot - (uDist * uDist - r * r));
		if(t < 0) t = (-1) * uvDot + SQRT(uvDot * uvDot - (uDist * uDist - r * r));

		if (t < 0) continue;

		if (std::get<1>(res) > t)
		{
			std::get<0>(res) = std::move(std::make_shared<GSphere>(sphere));
			std::get<1>(res) = t;
		}
	}

	return res;
}

// 2. 교차점 찾기
GPos3 point_of_intersection(GLine ray, float t)
{
	return ray.GetPt() + ray.GetDir() * t;
}

// 3. 반사
GVec3 reflection(GLine ray, GSphere S, GPos3 p)
{
	auto v = ray.GetDir();
	auto N = (S.Pos - p).Normalize();

	return v - 2 * Vec3Dot(N, v) * N;
}


