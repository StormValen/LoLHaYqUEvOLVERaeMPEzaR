#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include "GL_framework.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>


float radius = 0.05f;
bool check;
bool check2;
bool waterfall = true;
bool show_test_window = true;

float *partVerts;

int updateRange = 20;
float timePerFrame = 0.033;
int maxLife = 160;
glm::vec3 gravity = { 0, -9.8, 0 };
glm::vec3 normal = { 0,0,0 };

//planes and collisions
float d;
float coefElasticity = 0.9f;

//HUD velocity
int waterfallIncrementX = -3;
int fountainIncrementX, fountainIncrementY = 10, fountainIncrementZ;

//friction
glm::vec3 vNormal;
glm::vec3 vTangencial;
float coefFriction = 0.f;

//modes
bool euler = true;

void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//TODO
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		check = ImGui::SmallButton("Waterfall/Fountain");

		if (check) {
			waterfall = !waterfall;
		}

		check2 = ImGui::SmallButton("Euler/Verlet");

		if (check2) {
			euler = !euler;
		}

		ImGui::SliderFloat("Gravity", &gravity.y, -15, 15);
		
		//GUI Waterfall
		ImGui::SliderFloat("Coef.Elasticity", &coefElasticity, 0, 1);
		ImGui::SliderFloat("Coef.Friction", &coefFriction, 0, 1);
		if (waterfall) {
			ImGui::SliderInt("Velocity Y", &waterfallIncrementX, -10, -1);
		} //GUI Fountain
		else if (!waterfall) {
			ImGui::SliderInt("Velocity X", &fountainIncrementX, -10, 10);
			ImGui::SliderInt("Velocity Y", &fountainIncrementY, 1, 15);
			ImGui::SliderInt("Velocity Z", &fountainIncrementZ, -10, 10);
		}
		
	}
}

namespace LilSpheres {
	extern const int maxParticles;
	extern void setupParticles(int numTotalParticles, float radius);
	extern void cleanupParticles();
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void drawParticles(int startIdx, int count);
}

namespace Sphere {
	extern glm::vec3 centro = { 0.f, 1.f, 0.f };
	extern void setupSphere(glm::vec3 pos = centro, float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	extern void drawSphere();
	
}

struct laSphere {
	glm::vec3 pos;
	float radius;
};

laSphere *sphere = new laSphere();

struct Particle {
	glm::vec3 pos;
	glm::vec3 lastPos;
	glm::vec3 vel;
	glm::vec3 lastVel;
	float life;
};

Particle *particlesContainer = new Particle[LilSpheres::maxParticles];


void PhysicsInit() {
	
	//Sphere::setupSphere();
	partVerts = new float[LilSpheres::maxParticles * 3];

	sphere->pos = { 1.f, 3.f, 0.5f };
	sphere->radius = 1.0f;
	
	//init position particles
	for (int i = 0; i < LilSpheres::maxParticles; ++i) {
		if (waterfall) {
			partVerts[i * 3 + 0] = 5;
			partVerts[i * 3 + 1] = 8 + ((float)rand() / RAND_MAX) * 0.2f;
			partVerts[i * 3 + 2] = ((float)rand() / RAND_MAX) * 6.f - 3.f;
		}
		else if (!waterfall) {
			partVerts[i * 3 + 0] = 0;
			partVerts[i * 3 + 1] = 1;
			partVerts[i * 3 + 2] = 0;
		}
		
	}

	//sets de initial position, velocity and life
	srand(time(NULL));
	for (int i = 0; i < LilSpheres::maxParticles; i++) {
		if (waterfall) {
			particlesContainer[i].pos = glm::vec3(partVerts[i * 3], partVerts[i * 3 + 1], partVerts[i * 3 + 2]);
			if (!euler) { particlesContainer[i].lastPos =  particlesContainer[i].pos - glm::vec3(5,0,0)*timePerFrame; }//verlet}
			particlesContainer[i].vel = glm::vec3(waterfallIncrementX, 0, ((float)rand() / RAND_MAX) * 6.f - 3.f); //random
			particlesContainer[i].life = 0;
		}
		else if (!waterfall) {
			particlesContainer[i].pos = glm::vec3(partVerts[i * 3], partVerts[i * 3 + 1], partVerts[i * 3 + 2]);
			particlesContainer[i].vel = glm::vec3(fountainIncrementX + ((float)rand() / RAND_MAX) * 6.f - 3.f, fountainIncrementY + ((float)rand() / RAND_MAX) * 6.f - 3.f, fountainIncrementZ + ((float)rand() / RAND_MAX) * 6.f - 3.f); //random
			if (!euler) { particlesContainer[i].lastPos = particlesContainer[i].pos - particlesContainer[i].vel*timePerFrame; }//verlet}
			particlesContainer[i].life = 0;
		}
		
	}
}
void PhysicsUpdate(float dt) {

	glm::vec3 temp;

	//for all particles
	for (int i = 0; i < updateRange; i++) {

		if (euler) {
			//save last velocity module
			particlesContainer[i].lastVel = particlesContainer[i].vel;

			//update vector velocity velocity with formula
			particlesContainer[i].vel = particlesContainer[i].lastVel + gravity * timePerFrame;

			//save last position 
			particlesContainer[i].lastPos = particlesContainer[i].pos;

			//update position with formula
			particlesContainer[i].pos = particlesContainer[i].lastPos + timePerFrame * particlesContainer[i].lastVel; //components x and z have 0 gravity.
		}
		if (!euler) { //mass particule =1
			temp = particlesContainer[i].pos;
			particlesContainer[i].pos = temp + (temp - particlesContainer[i].lastPos) + gravity * glm::pow(timePerFrame, 2);
			particlesContainer[i].lastPos = temp;
		}


		//colisions
		//left wall
		if (particlesContainer[i].pos.x <= -5 + radius) {
			normal = { 1,0,0 };
			d = 5;

			if(euler){
				//friction values
				vNormal = glm::dot(normal, particlesContainer[i].vel) * normal;
				vTangencial = particlesContainer[i].vel - vNormal;

				//elasticity and friction
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
				particlesContainer[i].vel = particlesContainer[i].vel - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].vel))* normal - coefFriction*vTangencial;
			}
			if (!euler) {
				particlesContainer[i].lastPos = particlesContainer[i].lastPos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].lastPos) + d)*normal;
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
			}

		}
		//right wall
		if (particlesContainer[i].pos.x >= 5 - radius) {
			normal = { -1,0,0 };
			d = 5;

			if (euler) {
				//friction values
				vNormal = glm::dot(normal, particlesContainer[i].vel) * normal;
				vTangencial = particlesContainer[i].vel - vNormal;

				//elasticity and friction
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
				particlesContainer[i].vel = particlesContainer[i].vel - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].vel))* normal - coefFriction*vTangencial;
			}
			if (!euler) {
				particlesContainer[i].lastPos = particlesContainer[i].lastPos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].lastPos) + d)*normal;
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
			}

		}
		//front wall
		if (particlesContainer[i].pos.z <= -5 + radius) {
			normal = { 0,0,1 };
			d = 5;

			if (euler) {
				//friction values
				vNormal = glm::dot(normal, particlesContainer[i].vel) * normal;
				vTangencial = particlesContainer[i].vel - vNormal;

				//elasticity and friction
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
				particlesContainer[i].vel = particlesContainer[i].vel - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].vel))* normal - coefFriction*vTangencial;
			}
			if (!euler) {
				particlesContainer[i].lastPos = particlesContainer[i].lastPos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].lastPos) + d)*normal;
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
			}

		}
		//back wall
		if (particlesContainer[i].pos.z >= 5 - radius) {
			normal = { 0,0,-1 };
			d = 5;

			if (euler) {
				//friction values
				vNormal = glm::dot(normal, particlesContainer[i].vel) * normal;
				vTangencial = particlesContainer[i].vel - vNormal;

				//elasticity and friction
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
				particlesContainer[i].vel = particlesContainer[i].vel - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].vel))* normal - coefFriction*vTangencial;
			}
			if (!euler) {
				particlesContainer[i].lastPos = particlesContainer[i].lastPos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].lastPos) + d)*normal;
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
			}

		}
		//floor
		if (particlesContainer[i].pos.y <= 0 + radius) {
			normal = { 0,1,0 };
			d = 0;

			if (euler) {
				//friction values
				vNormal = glm::dot(normal, particlesContainer[i].vel) * normal;
				vTangencial = particlesContainer[i].vel - vNormal;

				//elasticity and friction
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
				particlesContainer[i].vel = particlesContainer[i].vel - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].vel))* normal - coefFriction*vTangencial;
			}
			if (!euler) {
				particlesContainer[i].lastPos = particlesContainer[i].lastPos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].lastPos) + d)*normal;
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
			}

		}
		//top wall
		if (particlesContainer[i].pos.y >= 10 - radius) {
			normal = { 0,-1,0 };
			d = 10;

			if (euler) {
				//friction values
				vNormal = glm::dot(normal, particlesContainer[i].vel) * normal;
				vTangencial = particlesContainer[i].vel - vNormal;

				//elasticity and friction
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
				particlesContainer[i].vel = particlesContainer[i].vel - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].vel))* normal - coefFriction*vTangencial;
			}
			if (!euler) {
				particlesContainer[i].lastPos = particlesContainer[i].lastPos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].lastPos) + d)*normal;
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
			}

		}


		//Sphere 
		if (glm::pow((particlesContainer[i].pos.x - sphere->pos.x), 2) + glm::pow((particlesContainer[i].pos.y - sphere->pos.y), 2) + glm::pow((particlesContainer[i].pos.z - sphere->pos.z), 2) <= glm::pow((sphere->radius + radius), 2)) {
			normal = { particlesContainer[i].pos-sphere->pos};
			d = -(particlesContainer[i].pos.x*normal.x) - (particlesContainer[i].pos.y*normal.y) - (particlesContainer[i].pos.z*normal.z);

			if (euler) {
				//friction values
				vNormal = glm::dot(normal, particlesContainer[i].vel) * normal;
				vTangencial = particlesContainer[i].vel - vNormal;

				//elasticity and friction
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
				particlesContainer[i].vel = particlesContainer[i].vel - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].vel))* normal - coefFriction*vTangencial;
			}
			if (!euler) {
				particlesContainer[i].lastPos = particlesContainer[i].lastPos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].lastPos) + d)*normal;
				particlesContainer[i].pos = particlesContainer[i].pos - (1 + coefElasticity) * (glm::dot(normal, particlesContainer[i].pos) + d)*normal;
			}

		}



		//life manager
		if (particlesContainer[i].life < maxLife) {
			particlesContainer[i].life += 1;
		}
		else { //init the particle (center and new random vector)
			if (waterfall) {
				particlesContainer[i].pos = glm::vec3(5, 8 + ((float)rand() / RAND_MAX) * 0.2f, ((float)rand() / RAND_MAX) * 6.f - 3.f);
				if (!euler) { particlesContainer[i].lastPos = particlesContainer[i].pos - glm::vec3(5, 0, 0)*timePerFrame; }//verlet}
				particlesContainer[i].vel = glm::vec3(waterfallIncrementX, 0, ((float)rand() / RAND_MAX) * 6.f - 3.f); //random
				particlesContainer[i].life = 0;
			}
			else if (!waterfall) {
			particlesContainer[i].pos = glm::vec3(0,1,0);
			particlesContainer[i].vel = glm::vec3(fountainIncrementX + ((float)rand() / RAND_MAX) * 6.f - 3.f, fountainIncrementY + ((float)rand() / RAND_MAX) * 6.f - 3.f, fountainIncrementZ + ((float)rand() / RAND_MAX) * 6.f - 3.f); //random
			if (!euler) { particlesContainer[i].lastPos = particlesContainer[i].pos - particlesContainer[i].vel*timePerFrame; }//verlet}
			particlesContainer[i].life = 0;
			}
		}
	
		//update partVerts vector with the new position
		partVerts[3 * i] = particlesContainer[i].pos.x;
		partVerts[3 * i + 1] = particlesContainer[i].pos.y;
		partVerts[3 * i + 2] = particlesContainer[i].pos.z;

	}

	if (updateRange < LilSpheres::maxParticles) {
		updateRange += 20; //each frame update 3 plus particles
	}

	Sphere::updateSphere(sphere->pos, sphere->radius);
	LilSpheres::updateParticles(0, LilSpheres::maxParticles, partVerts);
	
}
void PhysicsCleanup() {
	//TODO
	delete[] partVerts;
}