#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl/glut.h"
#include "gmath.h"

// ���� ����
#define MAX_DEPTH 3
//���� ppt��� ����
#define H 600
#define W 800
unsigned char Image[H * W * 3];
std::vector<GSphere> SphereList;
std::vector<GLight> LightList;

// �ݹ� �Լ� ����
void Render();
void Reshape(int w, int h);

// ���� ���� �Լ�
int GetIdx(int i, int j);
void CreateImage();
GVec3 RayTrace(GLine ray, int depth);
GVec3 Phong(GPos3 P, GVec3 N, GSphere Obj);
bool intersect_line_sphere(GLine ray, int &sidx, double &t);

GVec3 cal_norvec(GPos3 p1, GPos3 p2);// �븻 ���� ���ϴ� �Լ�
GVec3 cal_refract(GPos3 P, GVec3 N, GVec3 V);// ���� ������ ���͸� ���ϴ� �Լ�
//ȭ���� 64����� �Ͽ� ���� ������ ������ �ִ� ������ ������ �����ϴ� �Լ�
void CreatepartImage(int a, int b);
void DoTimer(int value);//Ÿ�̸�

int timerlimit = 0;// Ÿ�̸Ӹ� ���߱� ���� ����

bool part = false; // �ش� ������ ���� ������ ������ �ִ��� ���θ� �Ǵ�
int num = 0; // ������ �����ϴ� �ȼ��� ��(�ִ밡 800 * 600 = 480000)

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

	// ���� ����
	GLight Light0;
	Light0.Pos.Set(-300.0, 200.0, 100.0);
	Light0.Ia.Set(0.2, 0.2, 0.2);
	Light0.Id.Set(0.7, 0.7, 0.7);
	Light0.Is.Set(0.8, 0.8, 0.8);
	LightList.push_back(Light0);

	GLight Light1;
	Light1.Pos.Set(200.0, -300.0, -580.0);
	Light1.Ia.Set(0.2, 0.2, 0.2);
	Light1.Id.Set(0.7, 0.7, 0.7);
	Light1.Is.Set(0.8, 0.8, 0.8);
	LightList.push_back(Light1);

	// ��鿡 ���� ��ġ�Ѵ�.
	GSphere Sphere0; // �Ķ��� ��
	Sphere0.Pos.Set(-50, -20, -350.0);
	Sphere0.Rad = 35.0;
	Sphere0.Ka.Set(0.2, 0.2, 0.8);
	Sphere0.Kd.Set(0.0, 0.0, 0.7);
	Sphere0.Ks.Set(0.9, 0.9, 0.9);
	Sphere0.ns = 8.0;
	SphereList.push_back(Sphere0);

	GSphere Sphere1; // �ʷϻ� ��
	Sphere1.Pos.Set(50, -45, -380.0);
	Sphere1.Rad = 35.0;
	Sphere1.Ka.Set(0.8, 0.2, 0.2);
	Sphere1.Kd.Set(0.7, 0.0, 0.0);
	Sphere1.Ks.Set(0.9, 0.9, 0.9);
	Sphere1.ns = 8.0;
	SphereList.push_back(Sphere1);

	GSphere Sphere2; // ����� ��
	Sphere2.Pos.Set(50, 50, -440.0);
	Sphere2.Rad = 35.0;
	Sphere2.Ka.Set(0.8, 0.8, 0.2);
	Sphere2.Kd.Set(0.7, 0.7, 0.0);
	Sphere2.Ks.Set(0.9, 0.9, 0.9);
	Sphere2.ns = 8.0;
	SphereList.push_back(Sphere2);

	// �̹����� ����
	CreateImage();

	printf("%d", num); // ������ ��

	// Ÿ�̸� : 30�и� �ʸ��� ����� ���� ������(z��) 5 �̵�(70 �̵��ϸ� �����.)
	glutTimerFunc(30, DoTimer, 1);

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

void CreateImage()
{
	int x0 = -W / 2; // -400
	int y0 = H / 2 - 1; // 299
	double z = -(H / 2) / tan(M_PI * 15 / 180.0);

	// ȭ���� 64�����Ͽ� ���� �ִ��� ���θ� Ȯ�� �ִ� �κи� ���� ĳ����
	// �̺κ��� �ּ��� Ǯ�� �ؿ� �κ��� �ּ��� �ϰ� �ϸ� �ȴ�.
	int sidx;
	double t;
	int dx = W / 8;
	int dy = H / 8;

	// 64����� �Ѵ�.
	for (int k = 0; k < 8; k++)
	{
		for (int l = 0; l < 8; l++)
		{
			double ax = -W / 2 + (l * dx);
			double ay = -H / 2 + (k * dy);

			// 64����� �κ��� �׵θ�(4��)�� ���� �������� �˻��Ѵ�.
			for (int m = 0; m < dy; m++)
			{
				//���� �Ʒ��� �׵θ� ��
				double bx = ax + m;
				double by = ay + dy;

				// ���� �׵θ� �� �ȼ����� ���� ������ part���� true�� �ٲ۴�.
				GLine aray(GPos3(0.0, 0.0, 0.0), GPos3(bx, ay, z));
				if (intersect_line_sphere(aray, sidx, t))
				{
					part = true;
					break;
				}

				// ���� �׵θ� �� �ȼ����� ���� ������ part���� true�� �ٲ۴�.
				GLine bray(GPos3(0.0, 0.0, 0.0), GPos3(bx, by, z));
				if (intersect_line_sphere(bray, sidx, t))
				{
					part = true;
					break;
				}
			}

			for (int n = 0; n < dx; n++)
			{
				// �¿��� �׵θ���
				double bx = ax + dx;
				double by = ay - n;

				// ���� �׵θ� �� �ȼ����� ���� ������ part���� true�� �ٲ۴�.
				GLine aray(GPos3(0.0, 0.0, 0.0), GPos3(ax, by, z));
				if (intersect_line_sphere(aray, sidx, t))
				{
					part = true;
					break;
				}

				// ���� �׵θ� �� �ȼ����� ���� ������ part���� true�� �ٲ۴�.
				GLine bray(GPos3(0.0, 0.0, 0.0), GPos3(bx, by, z));
				if (intersect_line_sphere(bray, sidx, t))
				{
					part = true;
					break;
				}
			}

			// �˻簡 ���� �Ŀ� part���� true�̸� �� �κ��� ���� �������� �� ������ ��� �ȼ��� ����ĳ�����Ѵ�.
			if (part == true)
			{
				part = false;
				CreatepartImage(ax, ay);
			}
		}
	}

	// �������� �ʰ� ��� �ȼ��� ���� �����ϴ� ���
	/*for (int i = 0; i < H; ++i)
	{
		for (int j = 0; j < W; ++j)
		{
			double x = x0 + j;
			double y = y0 - i;
			GLine ray(GPos3(0.0, 0.0, 0.0), GPos3(x, y, z));

			GVec3 Color = RayTrace(ray, 0);

			num++;

			int idx = (H - 1 - i) * W * 3 + j * 3;
			unsigned char r = (Color[0] > 1.0) ? 255 : Color[0] * 255;
			unsigned char g = (Color[1] > 1.0) ? 255 : Color[1] * 255;
			unsigned char b = (Color[2] > 1.0) ? 255 : Color[2] * 255;
			Image[idx] = r;
			Image[idx + 1] = g;
			Image[idx + 2] = b;
		}
	}*/
}

// ���� ������� ���� ������ ���� ������ ��� �ȼ��� ���� ĳ�����ϴ� �Լ�
// ������ �����ϸ� �������� �ʰ� ���� �����ϴ� �κа� �����ϴ�.
void CreatepartImage(int a, int b)
{
	double z = -(H / 2) / tan(M_PI * 15 / 180.0);

	int dx = W / 8;
	int dy = H / 8;

	for (int i = 0; i < dy; ++i)
	{
		for (int j = 0; j < dx; ++j)
		{

			double x = a + j;
			double y = b + i;
			GLine ray(GPos3(0.0, 0.0, 0.0), GPos3(x, y, z));

			GVec3 Color = RayTrace(ray, 0);

			num++;

			int imagex = x + W / 2;
			int imagey = y + H / 2;

			// ���ϰ� �ٲ�����Ƿ� �ٽ� ���ϸ� �ٲ۴�.
			imagey = (imagey - H);
			if (imagey < 0)
			{
				imagey = imagey * (-1);
			}

			int idx = (H + 1 - imagey) * W * 3 + imagex * 3;
			unsigned char r = (Color[0] > 1.0) ? 255 : Color[0] * 255;
			unsigned char g = (Color[1] > 1.0) ? 255 : Color[1] * 255;
			unsigned char b = (Color[2] > 1.0) ? 255 : Color[2] * 255;
			Image[idx] = r;
			Image[idx + 1] = g;
			Image[idx + 2] = b;
		}
	}
}

GVec3 RayTrace(GLine ray, int depth)
{
	GVec3 C;
	if (depth++ > MAX_DEPTH)
	{
		return C;
	}

	double k_reflect = 0.3;
	double k_refract = 0.3;

	int sidx;	// ������ �����ϴ� ���� ��� ���� �ε���
	double t;	// ���������� ������ �Ķ���� t
	if (intersect_line_sphere(ray, sidx, t))
	{
		// ����...
		GPos3 P;
		GVec3 N, V, R;
		GLine ray_reflect, ray_refract;

		//������ �� P�� ���Ѵ�.
		P[0] = ray.p[0] + t * ray.v[0];
		P[1] = ray.p[1] + t * ray.v[1];
		P[2] = ray.p[2] + t * ray.v[2];

		// N��V�� ����
		N = cal_norvec(SphereList[sidx].Pos, P);
		V = cal_norvec(GPos3(0.0, 0.0, 0.0), P);
		double nvdot = (N[0] * V[0]) + (N[1] * V[1]) + (N[2] * V[2]);

		// �� ���̵��� �Ѵ�.
		C = Phong(P, N, SphereList[sidx]);

		// �ݻ� ������ ���Ѵ�.
		R = V - 2 * (nvdot * N);
		ray_reflect.p = P;
		ray_reflect.v = R;

		// �ݻ� ������ �����Ͽ� ���� ���� ���Ѵ�.
		C += k_reflect * RayTrace(ray_reflect, depth);

		// ���� ������ ���ϴ� �κ�. �������� ���Ͽ���. �ƽ���.
		// P0�� ���� �߽��� �Ÿ��� ���Ͽ� �̸� ������ ���� �������� �� ���� ���� ���������� �����Ϸ� �ߴ�.
		// ���� �� ���� ���� ������ ���� �ݻ� ������ ������ �Ϸ��� �Ͽ���.
		/*double pcircledist = sqrt(((ray.p[0] - SphereList[sidx].Pos[0]) * (ray.p[0] - SphereList[sidx].Pos[0])) + ((ray.p[1] - SphereList[sidx].Pos[1]) * (ray.p[1] - SphereList[sidx].Pos[1])) + ((ray.p[2] - SphereList[sidx].Pos[2]) * (ray.p[2] - SphereList[sidx].Pos[2])));

		// p0�� ���� �߽ɻ����� �Ÿ��� ���� ����� ���� ���������� �� ���� �� ���� ���� �����̴�
		// �ݻ籤���� �߻�
		if (pcircledist > SphereList[sidx].Rad)
		{
			N = cal_norvec(SphereList[sidx].Pos, P);
			V = cal_norvec(GPos3(0.0, 0.0, 0.0), P);

			double nvdot = (N[0] * V[0]) + (N[1] * V[1]) + (N[2] * V[2]);

			C = Phong(P, N, SphereList[sidx]);

			R = V - 2 * (nvdot * N);

			ray_reflect.p = P;
			ray_reflect.v = R;

			C += k_reflect * RayTrace(ray_reflect, depth);

			GVec3 revect = cal_refract(P, N, V);

			ray_refract.p = P;
			ray_refract.v = revect;

			C += k_refract * RayTrace(ray_refract, depth);
		}
		else
		{
		// �ݻ籤���� �� �߻�
			N = cal_norvec(P, SphereList[sidx].Pos);
			V = cal_norvec(GPos3(0.0, 0.0, 0.0), P);

			C = Phong(P, N, SphereList[sidx]);

			GVec3 revect = cal_refract(P, N, V);

			ray_refract.p = P;
			ray_refract.v = revect;

			C += k_refract * RayTrace(ray_refract, depth);
		}*/
	}
	return C;
}

//�� ���̵�
GVec3 Phong(GPos3 P, GVec3 N, GSphere Obj)
{
	GVec3 C;

	GVec3 L, V, R;

	C[0] = 0;
	C[1] = 0;
	C[2] = 0;

	int lightsize = LightList.size(); // ������ ����

	// �� �������� �߻��� ������ ���Ѵ�
	for (int i = 0; i < lightsize; i++)
	{
		V = cal_norvec(P, GPos3(0.0, 0.0, 0.0));
		L = cal_norvec(P, LightList[i].Pos);

		// N�� V�� ����
		GVec3 sv = cal_norvec(LightList[i].Pos, P);
		double nsvdot = (N[0] * sv[0]) + (N[1] * sv[1]) + (N[2] * sv[2]);

		R = sv - 2 * (nsvdot * N);

		// N�� L�� ����
		double nldot = (N[0] * L[0]) + (N[1] * L[1]) + (N[2] * L[2]);
		// N�� R�� ����
		double vrdot = (V[0] * R[0]) + (V[1] * R[1]) + (V[2] * R[2]);

		C[0] += (Obj.Ka[0] * LightList[i].Ia[0]) + (Obj.Kd[0] * LightList[i].Id[0] * nldot) + (Obj.Kd[0] * LightList[i].Id[0] * pow(vrdot, Obj.ns));
		C[1] += (Obj.Ka[1] * LightList[i].Ia[1]) + (Obj.Kd[1] * LightList[i].Id[1] * nldot) + (Obj.Kd[1] * LightList[i].Id[1] * pow(vrdot, Obj.ns));
		C[2] += (Obj.Ka[2] * LightList[i].Ia[2]) + (Obj.Kd[2] * LightList[i].Id[2] * nldot) + (Obj.Kd[2] * LightList[i].Id[2] * pow(vrdot, Obj.ns));
	}
	return C;
}

// ���� �������� ���θ� �˻�
bool intersect_line_sphere(GLine ray, int &sidx, double &t)
{
	double finalt = -1;
	int finalsidx = -1;

	int spheresize = SphereList.size();

	for (int i = 0; i < spheresize; i++)
	{
		GVec3 u = ray.p - SphereList[i].Pos; // ���� ���� �� - ���� �߽�

		// U�� V�� ����
		double uvdot = u[0] * ray.v[0] + u[1] * ray.v[1] + u[2] * ray.v[2];
		//U�� ũ��
		double u_distance = (u[0] * u[0]) + (u[1] * u[1]) + (u[2] * u[2]);

		// (U�� V�� ����)���� - (U�� ũ���� ���� - ���� ������ ����)
		// �̰� 0���� ũ�ų� ������ ���� ������ �������� �����.
		double t_distinct = (uvdot * uvdot) - (u_distance - (SphereList[i].Rad * SphereList[i].Rad));
		// ���� ���� ������ �������� �ð� t�� ���Ѵ�.
		double pret = ((-1) * uvdot) - sqrt(t_distinct);

		// �������� ������ �Ͻ� ���� ����� �������� ����
		if (t_distinct >= 0 && pret > 0)
		{
			if (finalt < 0)
			{
				finalt = pret;
				finalsidx = i;
			}
			else
			{
				if (finalt > pret)
				{
					finalt = pret;
					finalsidx = i;
				}
			}
		}
	}

	if (finalt > 0 && finalsidx >= 0)
	{
		sidx = finalsidx;
		t = finalt;

		return true;
	}
	else
	{
		return false;
	}

}

// �븻 ���͸� ���ϴ� �Լ� �����Ѵ�.
GVec3 cal_norvec(GPos3 p1, GPos3 p2)
{
	GVec3 r;
	double dist;

	r[0] = p2[0] - p1[0];
	r[1] = p2[1] - p1[1];
	r[2] = p2[2] - p1[2];

	dist = sqrt(((p2[0] - p1[0]) * (p2[0] - p1[0])) + ((p2[1] - p1[1]) * (p2[1] - p1[1])) + ((p2[2] - p1[2]) * (p2[2] - p1[2])));

	r[0] = r[0] / dist;
	r[1] = r[1] / dist;
	r[2] = r[2] / dist;

	return r;
}

// Ÿ�̸� 30�и��ʸ��� ����� ���� ���� �������� 5�� �̵��Ѵ�(�̵� �Ÿ��� 70�Ͻ� �����.)
void DoTimer(int value)
{
	timerlimit += 5;

	if (timerlimit < 90)
	{
		SphereList[2].Pos[2] += 10;
	}

	CreateImage();

	glutPostRedisplay();

	glutTimerFunc(30, DoTimer, 1);
}

// ���������� ���͸� ���ϴ� �Լ�. ����� ���� �� ������ �����Ѵ� �ƽ���.
GVec3 cal_refract(GPos3 P, GVec3 N, GVec3 V)
{
	GVec3 vect_refract;

	double index_refract = 1.33;

	//�ڻ����� ���Ѵ�.
	double cos1 = (N[0] * (-1) * V[0]) + (N[1] * (-1) * V[1]) + (N[2] * (-1) * V[2]);
	double cos2 = sqrt(1 - ((index_refract * index_refract) * (1 - (cos1 * cos1))));

	// aV + bN
	double a = index_refract;
	double b = cos2 - (a * cos1);

	vect_refract[0] = (a * V[0]) + (b * N[0]);
	vect_refract[1] = (a * V[1]) + (b * N[1]);
	vect_refract[2] = (a * V[2]) + (b * N[2]);

	return vect_refract;
}
