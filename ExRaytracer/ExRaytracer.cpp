#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl/glut.h"
#include "gmath.h"
#include <tuple>

#define MAX_DEPTH 5
#define H 600
#define W 800

// ���� ������
#define n1 1.0
// �� ������
#define n2 1.5

#define kreflect 0.3
#define krefract 0.3

#define PI 3.141592

unsigned char Image[H * W * 3];
std::vector<GSphere> SphereList;
std::vector<GLight> LightList;

// �ݹ� �Լ� ����
void Render();
void Reshape(int w, int h);

// �̹��� �׸���
void Img();
// ����Ʈ���̽�
GVec3 RayTrace(GLine v, int depth);
// 1. �����ϴ� ���� ����� �� ã��
typedef std::tuple<std::shared_ptr<GSphere>, double> findSphere;
findSphere nearest_intersected_surface(GLine ray);
// 2. ������ ã��
GPos3 point_of_intersection(GLine ray, double t);
// 3. �ݻ�
GLine reflection(GVec3 v, GSphere S, GPos3 p);
// 4. ����
GLine refraction(GVec3 v, GSphere S, GPos3 p);
// 5. �� ���̵�
GVec3 phong(GPos3 p, GSphere S, GLine ray);
// 6. ȸ�� �ִϸ��̼�
void Anim(int i);

int main(int argc, char **argv)
{
	// OpenGL �ʱ�ȭ, ������ ũ�� ����, ���÷��� ��� ����
	glutInit(&argc, argv);
	glutInitWindowSize(W, H);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// ������ ���� �� �ݹ� �Լ� ���
	glutCreateWindow("RayTracer");
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);

	// ���� ��ġ
	LightList.push_back(GLight (
		GPos3(-300.0, 200.0, 100.0),
		GVec3(0.2, 0.2, 0.2),
		GVec3(0.7, 0.7, 0.7),
		GVec3(0.8, 0.8, 0.8)
	));

	LightList.push_back(GLight(
		GPos3(100.0, -300.0, -400.0),
		GVec3(0.2, 0.2, 0.2),
		GVec3(0.7, 0.7, 0.7),
		GVec3(0.8, 0.8, 0.8)
	));

	// �� ��ġ

	// �Ķ��� ��
	GSphere Sphere0 (
		GPos3(60 * cos(PI * 2 / 3 * 0), 60 * sin(PI * 2 / 3 * 0), -350.0),
		25.0f
	); 
	Sphere0.Ka.Set(0.2, 0.2, 0.8);
	Sphere0.Kd.Set(0.0, 0.0, 0.7);
	Sphere0.Ks.Set(0.9, 0.9, 0.9);
	Sphere0.ns = 8.0;
	SphereList.push_back(Sphere0);

	// ������ ��
	GSphere Sphere1(
		GPos3(60 * cos(PI * 2 / 3 * 1), 60 * sin(PI * 2 / 3 * 1), -350.0),
		25.0f
	);
	Sphere1.Ka.Set(0.8, 0.2, 0.2);
	Sphere1.Kd.Set(0.7, 0.0, 0.0);
	Sphere1.Ks.Set(0.9, 0.9, 0.9);
	Sphere1.ns = 8.0;
	SphereList.push_back(Sphere1);

	// ����� ��
	GSphere Sphere2(
		GPos3(60 * cos(PI * 2 / 3 * 2), 60 * sin(PI * 2 / 3 * 2), -350.0),
		25.0f
	);
	Sphere2.Ka.Set(0.8, 0.8, 0.2);
	Sphere2.Kd.Set(0.7, 0.7, 0.0);
	Sphere2.Ks.Set(0.9, 0.9, 0.9);
	Sphere2.ns = 8.0;
	SphereList.push_back(Sphere2);

	// �ʷϻ� ��
	GSphere Sphere3(GPos3(0, 0, -350.0), 25.0f);
	Sphere3.Ka.Set(0.2, 0.8, 0.2);
	Sphere3.Kd.Set(0.0, 0.7, 0.0);
	Sphere3.Ks.Set(0.9, 0.9, 0.9);
	Sphere3.ns = 8.0;
	SphereList.push_back(Sphere3);

	// �̹����� ����
	Img();

	// ȸ�� �ִϸ��̼�
	glutTimerFunc(1, Anim, 0);

	// �̺�Ʈ�� ó���� ���� ���� ������ �����Ѵ�.
	glutMainLoop();

	return 0;
}

void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Render()
{
	// Į�� ���ۿ� ���� ���� �����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Į�� ���ۿ� Image �����͸� ���� �׸���.
	glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, Image);

	// Į�� ���� ��ȯ�Ѵ�
	glutSwapBuffers();
}

void Img()
{
	int x0 = -W / 2; // -400
	int y0 = H / 2 - 1; // 299
	double z = -(H / 2) / tan(M_PI * 15 / 180.0);

	for (int i = 0; i < H; ++i)
	{
		for (int j = 0; j < W; ++j)
		{
			double x = x0 + j;
			double y = y0 - i;
			GLine ray(GPos3(0.0, 0.0, 0.0), GPos3(x, y, z));

			GVec3 Color = RayTrace(ray, 0);

			int idx = (H - 1 - i) * W * 3 + j * 3;

			for (int i = 0; i < 3; i++)
			{
				Image[idx + i] = (Color[i] > 1.0) ? 255 : Color[i] * 255;
			}
		}
	}
}

GVec3 RayTrace(GLine v, int depth)
{
	GVec3 C(0.0, 0.0, 0.0);
	depth++;

	// 1. �����ϴ� ���� ����� �� ã��
	auto S = nearest_intersected_surface(v);

	auto &sphere = std::get<0>(S);
	auto &time = std::get<1>(S);

	if (sphere == nullptr || depth == MAX_DEPTH) return C; // default color

	// 2. ������ ã��
	auto p = point_of_intersection(v, time);

	// 3. �ݻ�
	auto R = reflection(v.GetDir(), *sphere, p);

	// 4. ����
	auto T = refraction(v.GetDir(), *sphere, p);

	// 5. �� ���̵�
	C = phong(p, *sphere, v)
		+ kreflect * RayTrace(R, depth);
		+ krefract * RayTrace(T, depth);

	return C;
}

// 1. �����ϴ� ���� ����� �� ã��
findSphere nearest_intersected_surface(GLine ray)
{
	findSphere res = std::make_tuple(nullptr, std::numeric_limits<double>::max());

	for (auto const &sphere : SphereList)
	{
		// u : p0 - q(�� �߽�)
		GVec3 v = ray.GetDir(), u = ray.GetPt() - sphere.Pos;
		double uvDot = u * v, uuDot = u * u, r = sphere.Rad;

		if (SQR(uvDot) - (uuDot - r * r) < 0) continue;

		double q = SQRT(SQR(uvDot) - (uuDot - r * r));
		double t0 = (-1) * uvDot - q, t1 = (-1) * uvDot + q;

		if (t0 > 0 && std::get<1>(res) > t0)
		{
			std::get<0>(res) = std::move(std::make_shared<GSphere>(sphere));
			std::get<1>(res) = t0;
		}
	}

	return res;
}

// 2. ������ ã��
GPos3 point_of_intersection(GLine ray, double t)
{
	return ray.GetPt() + ray.GetDir() * t;
}

// 3. �ݻ�
GLine reflection(GVec3 v, GSphere S, GPos3 p)
{
	auto N = (p - S.Pos).Normalize();

	return GLine(p, (v - 2 * N * v * N).Normalize());
}

// 4. ����
GLine refraction(GVec3 v, GSphere S, GPos3 p)
{
	// 1�� ���� ����
	auto N = (p - S.Pos).Normalize();
	auto cos1 = N * (-v);
	auto cos2 = SQRT(1 - SQR(n1 / n2) * (1 - SQR(cos1)));

	auto T1 = ((n1 / n2) * v - (cos2 - (n1 / n2) * cos1) * N).Normalize();

	// ������ ã��
	v = T1;
	GVec3 u = p - S.Pos;
	double uvDot = u * v, uuDot = u * u, r = S.Rad;

	double q = SQRT(SQR(uvDot) - (uuDot - r * r));
	double t = (-1) * uvDot + q;

	GPos3 p1 = point_of_intersection(GLine(p, v), t);

	// 2�� ���� ����
	N = (S.Pos - p1).Normalize();
	cos1 = N * -v;
	cos2 = SQRT(1 - SQR(n2 / n1) * (1 - SQR(cos1)));

	auto T2 = ((n2 / n1) * v - (cos2 - (n2 / n1) * cos1) * N).Normalize();

	return GLine(p1, T2);
}

// 5. �� ���̵�
GVec3 phong(GPos3 p, GSphere S, GLine ray)
{
	GVec3 C(0, 0, 0);

	auto N = (p- S.Pos).Normalize();
	auto V = (ray.GetPt() - p).Normalize();

	for (auto const &light : LightList)
	{
		auto L = (light.Pos - p).Normalize();
		auto R = reflection((p - light.Pos).Normalize(), S, p);

		for (int i = 0; i < 3; i++)
		{
			C[i] += S.Ka[i] * light.Ia[i]
				+ S.Kd[i] * light.Id[i] * N * L
				+ S.Ks[i] * light.Is[i] * pow(V * R.GetDir(), S.ns);
		}
	}

	return C;
}

// 6. ȸ�� �ִϸ��̼�
void Anim(int i)
{
	float t = i * 0.1f;

	for (int i = 0; i < SphereList.size() - 1; i++)
	{
		auto &p = SphereList[i].Pos;
		p[0] = 60 * cos(t + (PI * 2 / 3 * i));
		p[1] = 60 * sin(t + (PI * 2 / 3 * i));
	}

	Img();
	glutPostRedisplay();
	glutTimerFunc(1, Anim, i + 1);
}


