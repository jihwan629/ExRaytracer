#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl/glut.h"
#include "gmath.h"

// 전역 변수
#define MAX_DEPTH 3
//과제 ppt대로 변경
#define H 600
#define W 800
unsigned char Image[H * W * 3];
std::vector<GSphere> SphereList;
std::vector<GLight> LightList;

// 콜백 함수 선언
void Render();
void Reshape(int w, int h);

// 광선 추적 함수
int GetIdx(int i, int j);
void CreateImage();
GVec3 RayTrace(GLine ray, int depth);
GVec3 Phong(GPos3 P, GVec3 N, GSphere Obj);
bool intersect_line_sphere(GLine ray, int &sidx, double &t);

GVec3 cal_norvec(GPos3 p1, GPos3 p2);// 노말 벡터 구하는 함수
GVec3 cal_refract(GPos3 P, GVec3 N, GVec3 V);// 굴절 광선의 벡터를 구하는 함수
//화면을 64등분을 하여 구와 만나는 광선이 있는 영역만 광선을 생성하는 함수
void CreatepartImage(int a, int b);
void DoTimer(int value);//타이머

int timerlimit = 0;// 타이머를 멈추기 위한 변수

bool part = false; // 해당 구역에 구와 만나는 광선이 있는지 여부를 판단
int num = 0; // 광선을 생성하는 픽셀의 수(최대가 800 * 600 = 480000)

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

	// 조명 설정
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

	// 장면에 구를 배치한다.
	GSphere Sphere0; // 파란색 구
	Sphere0.Pos.Set(-50, -20, -350.0);
	Sphere0.Rad = 35.0;
	Sphere0.Ka.Set(0.2, 0.2, 0.8);
	Sphere0.Kd.Set(0.0, 0.0, 0.7);
	Sphere0.Ks.Set(0.9, 0.9, 0.9);
	Sphere0.ns = 8.0;
	SphereList.push_back(Sphere0);

	GSphere Sphere1; // 초록색 구
	Sphere1.Pos.Set(50, -45, -380.0);
	Sphere1.Rad = 35.0;
	Sphere1.Ka.Set(0.8, 0.2, 0.2);
	Sphere1.Kd.Set(0.7, 0.0, 0.0);
	Sphere1.Ks.Set(0.9, 0.9, 0.9);
	Sphere1.ns = 8.0;
	SphereList.push_back(Sphere1);

	GSphere Sphere2; // 노란색 구
	Sphere2.Pos.Set(50, 50, -440.0);
	Sphere2.Rad = 35.0;
	Sphere2.Ka.Set(0.8, 0.8, 0.2);
	Sphere2.Kd.Set(0.7, 0.7, 0.0);
	Sphere2.Ks.Set(0.9, 0.9, 0.9);
	Sphere2.ns = 8.0;
	SphereList.push_back(Sphere2);

	// 이미지를 생성
	CreateImage();

	printf("%d", num); // 광선의 수

	// 타이머 : 30밀리 초마다 노란색 구를 앞으로(z축) 5 이동(70 이동하면 멈춘다.)
	glutTimerFunc(30, DoTimer, 1);

	// 이벤트를 처리를 위한 무한 루프로 진입한다.
	glutMainLoop();

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

void CreateImage()
{
	int x0 = -W / 2; // -400
	int y0 = H / 2 - 1; // 299
	double z = -(H / 2) / tan(M_PI * 15 / 180.0);

	// 화면을 64분할하여 구가 있는지 여부를 확인 있는 부분만 레이 캐스팅
	// 이부분을 주석을 풀고 밑에 부분을 주석을 하고 하면 된다.
	int sidx;
	double t;
	int dx = W / 8;
	int dy = H / 8;

	// 64등분을 한다.
	for (int k = 0; k < 8; k++)
	{
		for (int l = 0; l < 8; l++)
		{
			double ax = -W / 2 + (l * dx);
			double ay = -H / 2 + (k * dy);

			// 64등분한 부분의 테두리(4개)만 구와 만나는지 검사한다.
			for (int m = 0; m < dy; m++)
			{
				//위와 아래의 테두리 선
				double bx = ax + m;
				double by = ay + dy;

				// 만약 테두리 중 픽셀에서 구와 만나면 part값을 true로 바꾼다.
				GLine aray(GPos3(0.0, 0.0, 0.0), GPos3(bx, ay, z));
				if (intersect_line_sphere(aray, sidx, t))
				{
					part = true;
					break;
				}

				// 만약 테두리 중 픽셀에서 구와 만나면 part값을 true로 바꾼다.
				GLine bray(GPos3(0.0, 0.0, 0.0), GPos3(bx, by, z));
				if (intersect_line_sphere(bray, sidx, t))
				{
					part = true;
					break;
				}
			}

			for (int n = 0; n < dx; n++)
			{
				// 좌우의 테두리선
				double bx = ax + dx;
				double by = ay - n;

				// 만약 테두리 중 픽셀에서 구와 만나면 part값을 true로 바꾼다.
				GLine aray(GPos3(0.0, 0.0, 0.0), GPos3(ax, by, z));
				if (intersect_line_sphere(aray, sidx, t))
				{
					part = true;
					break;
				}

				// 만약 테두리 중 픽셀에서 구와 만나면 part값을 true로 바꾼다.
				GLine bray(GPos3(0.0, 0.0, 0.0), GPos3(bx, by, z));
				if (intersect_line_sphere(bray, sidx, t))
				{
					part = true;
					break;
				}
			}

			// 검사가 끝난 후에 part값이 true이면 그 부분이 구를 지남으로 그 영역의 모든 픽셀을 레이캐스팅한다.
			if (part == true)
			{
				part = false;
				CreatepartImage(ax, ay);
			}
		}
	}

	// 분할하지 않고 모든 픽셀에 광선 추적하는 방법
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

// 분할 방법에서 구를 지나는 선의 구역의 모든 픽셀을 레이 캐스팅하는 함수
// 범위만 제외하면 분할하지 않고 광선 추적하는 부분과 동일하다.
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

			// 상하가 바뀌었으므로 다시 상하를 바꾼다.
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

	int sidx;	// 광선과 교차하는 가장 까가운 구의 인덱스
	double t;	// 교차점에서 광선의 파라미터 t
	if (intersect_line_sphere(ray, sidx, t))
	{
		// 구현...
		GPos3 P;
		GVec3 N, V, R;
		GLine ray_reflect, ray_refract;

		//구위의 점 P를 구한다.
		P[0] = ray.p[0] + t * ray.v[0];
		P[1] = ray.p[1] + t * ray.v[1];
		P[2] = ray.p[2] + t * ray.v[2];

		// N과V의 내적
		N = cal_norvec(SphereList[sidx].Pos, P);
		V = cal_norvec(GPos3(0.0, 0.0, 0.0), P);
		double nvdot = (N[0] * V[0]) + (N[1] * V[1]) + (N[2] * V[2]);

		// 퐁 셰이딩을 한다.
		C = Phong(P, N, SphereList[sidx]);

		// 반사 광선을 구한다.
		R = V - 2 * (nvdot * N);
		ray_reflect.p = P;
		ray_reflect.v = R;

		// 반사 광선을 추적하여 색을 구해 더한다.
		C += k_reflect * RayTrace(ray_reflect, depth);

		// 굴절 광선을 구하는 부분. 구현하지 못하였다. 아쉽다.
		// P0와 구의 중심의 거리를 구하여 이를 구안의 굴절 광선인지 구 밖의 굴절 광선인지를 구분하려 했다.
		// 만약 구 안의 굴절 광선일 때는 반사 광선이 없도록 하려고 하였다.
		/*double pcircledist = sqrt(((ray.p[0] - SphereList[sidx].Pos[0]) * (ray.p[0] - SphereList[sidx].Pos[0])) + ((ray.p[1] - SphereList[sidx].Pos[1]) * (ray.p[1] - SphereList[sidx].Pos[1])) + ((ray.p[2] - SphereList[sidx].Pos[2]) * (ray.p[2] - SphereList[sidx].Pos[2])));

		// p0과 구의 중심사이의 거리가 가장 가까운 구의 반지름보다 길 때는 구 밖의 굴절 광선이다
		// 반사광선을 발생
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
		// 반사광선을 안 발생
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

//퐁 셰이딩
GVec3 Phong(GPos3 P, GVec3 N, GSphere Obj)
{
	GVec3 C;

	GVec3 L, V, R;

	C[0] = 0;
	C[1] = 0;
	C[2] = 0;

	int lightsize = LightList.size(); // 광원의 개수

	// 각 광원으로 발생한 색감을 더한다
	for (int i = 0; i < lightsize; i++)
	{
		V = cal_norvec(P, GPos3(0.0, 0.0, 0.0));
		L = cal_norvec(P, LightList[i].Pos);

		// N과 V의 내적
		GVec3 sv = cal_norvec(LightList[i].Pos, P);
		double nsvdot = (N[0] * sv[0]) + (N[1] * sv[1]) + (N[2] * sv[2]);

		R = sv - 2 * (nsvdot * N);

		// N과 L의 내적
		double nldot = (N[0] * L[0]) + (N[1] * L[1]) + (N[2] * L[2]);
		// N과 R의 내적
		double vrdot = (V[0] * R[0]) + (V[1] * R[1]) + (V[2] * R[2]);

		C[0] += (Obj.Ka[0] * LightList[i].Ia[0]) + (Obj.Kd[0] * LightList[i].Id[0] * nldot) + (Obj.Kd[0] * LightList[i].Id[0] * pow(vrdot, Obj.ns));
		C[1] += (Obj.Ka[1] * LightList[i].Ia[1]) + (Obj.Kd[1] * LightList[i].Id[1] * nldot) + (Obj.Kd[1] * LightList[i].Id[1] * pow(vrdot, Obj.ns));
		C[2] += (Obj.Ka[2] * LightList[i].Ia[2]) + (Obj.Kd[2] * LightList[i].Id[2] * nldot) + (Obj.Kd[2] * LightList[i].Id[2] * pow(vrdot, Obj.ns));
	}
	return C;
}

// 구와 만나는지 여부를 검사
bool intersect_line_sphere(GLine ray, int &sidx, double &t)
{
	double finalt = -1;
	int finalsidx = -1;

	int spheresize = SphereList.size();

	for (int i = 0; i < spheresize; i++)
	{
		GVec3 u = ray.p - SphereList[i].Pos; // 직선 위의 점 - 구의 중심

		// U와 V의 내적
		double uvdot = u[0] * ray.v[0] + u[1] * ray.v[1] + u[2] * ray.v[2];
		//U의 크기
		double u_distance = (u[0] * u[0]) + (u[1] * u[1]) + (u[2] * u[2]);

		// (U와 V의 내적)제곱 - (U의 크기의 제곱 - 구의 반지름 제곱)
		// 이게 0보다 크거나 같으면 구와 광선에 교차점이 생긴다.
		double t_distinct = (uvdot * uvdot) - (u_distance - (SphereList[i].Rad * SphereList[i].Rad));
		// 구와 먼저 만나는 교차점의 시간 t를 구한다.
		double pret = ((-1) * uvdot) - sqrt(t_distinct);

		// 교차점이 여러개 일시 가장 가까운 교차점을 고른다
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

// 노말 벡터를 구하는 함수 무시한다.
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

// 타이머 30밀리초마다 노란색 구가 보는 방향으로 5씩 이동한다(이동 거리가 70일시 멈춘다.)
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

// 굴절광선의 벡터를 구하는 함수. 제대로 구현 못 했으니 무시한다 아쉽다.
GVec3 cal_refract(GPos3 P, GVec3 N, GVec3 V)
{
	GVec3 vect_refract;

	double index_refract = 1.33;

	//코사인을 구한다.
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
