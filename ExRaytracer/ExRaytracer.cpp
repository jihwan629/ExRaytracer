#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gl/glut.h"
#include "gmath.h"

#define MAX_DEPTH 3
#define H 600
#define W 800


int x0 = -W / 2; // -400
int y0 = H / 2 - 1; // 299
double z = -(H / 2) / tan(M_PI * 15 / 180.0);

unsigned char Image[H * W * 3];
std::vector<GSphere> SphereList;
std::vector<GLight> LightList;

// �ݹ� �Լ� ����
void Render();
void Reshape(int w, int h);

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

	// �� ��ġ

	// �Ķ��� ��
	GSphere Sphere0 (GPos3(-50, -20, -350.0), 35.0f); 
	Sphere0.Ka.Set(0.2, 0.2, 0.8);
	Sphere0.Kd.Set(0.0, 0.0, 0.7);
	Sphere0.Ks.Set(0.9, 0.9, 0.9);
	Sphere0.ns = 8.0;
	SphereList.push_back(Sphere0);

	// �ʷϻ� ��
	GSphere Sphere1(GPos3(50, -45, -380.0), 35.0f);
	Sphere1.Ka.Set(0.8, 0.2, 0.2);
	Sphere1.Kd.Set(0.7, 0.0, 0.0);
	Sphere1.Ks.Set(0.9, 0.9, 0.9);
	Sphere1.ns = 8.0;
	SphereList.push_back(Sphere1);

	// ����� ��
	GSphere Sphere2(GPos3(50, 50, -440.0), 35.0f);
	Sphere2.Ka.Set(0.8, 0.8, 0.2);
	Sphere2.Kd.Set(0.7, 0.7, 0.0);
	Sphere2.Ks.Set(0.9, 0.9, 0.9);
	Sphere2.ns = 8.0;
	SphereList.push_back(Sphere2);

	// �̹����� ����


	// Ÿ�̸� : 30�и� �ʸ��� ����� ���� ������(z��) 5 �̵�(70 �̵��ϸ� �����.)
	

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


