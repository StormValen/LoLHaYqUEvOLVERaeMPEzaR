#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include "GL_framework.h"
#include <stdlib.h>
#include <time.h>

bool waterfall = true;
bool check;
bool show_test_window = true;
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

	}
}

namespace LilSpheres {
	extern const int maxParticles;
	extern void setupParticles(int numTotalParticles, float radius = 0.05f);
	extern void cleanupParticles();
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void drawParticles(int startIdx, int count);
}

float *partVerts;
glm::vec3 gravity = {0, -9.8, 0};
int updateRange = 20;
float timePerFrame = 0.033;
int maxLife = 60;

struct Particle {
	glm::vec3 pos;
	glm::vec3 lastPos;
	glm::vec3 vel;
	glm::vec3 lastVel;
	float life;
};

Particle *particlesContainer = new Particle[LilSpheres::maxParticles];

void PhysicsInit() {
	
	partVerts = new float[LilSpheres::maxParticles * 3];
	
	//init position particles
	for (int i = 0; i < LilSpheres::maxParticles; ++i) {
		if (waterfall) {
			partVerts[i * 3 + 0] = 5;
			partVerts[i * 3 + 1] = 8 + ((float)rand() / RAND_MAX) * 0.2f;
			partVerts[i * 3 + 2] = ((float)rand() / RAND_MAX) * 6.f - 3.f;
		}
		else if (waterfall == false) {
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
			particlesContainer[i].vel = glm::vec3(-3, 0, ((float)rand() / RAND_MAX) * 6.f - 3.f); //random
			particlesContainer[i].life = 0;
		}
		else if (waterfall == false) {
			particlesContainer[i].pos = glm::vec3(partVerts[i * 3], partVerts[i * 3 + 1], partVerts[i * 3 + 2]);
			particlesContainer[i].vel = glm::vec3(((float)rand() / RAND_MAX) * 6.f - 3.f,(10 +((float)rand() / RAND_MAX) * 6.f - 3.f), ((float)rand() / RAND_MAX) * 6.f - 3.f); //random
			particlesContainer[i].life = 0;
		}
		
	}
}
void PhysicsUpdate(float dt) {


	//for all particles
	for (int i = 0; i < updateRange; i++) {

		//save last velocity module
		particlesContainer[i].lastVel = particlesContainer[i].vel;


		//update vector velocity velocity with formula
		particlesContainer[i].vel = particlesContainer[i].lastVel + gravity * timePerFrame;

		//save last position 
		particlesContainer[i].lastPos = particlesContainer[i].pos;

		//update position with formula
		particlesContainer[i].pos = particlesContainer[i].lastPos + timePerFrame * particlesContainer[i].lastVel + 0.5f * gravity * (pow(timePerFrame, 2)); //components x and z have 0 gravity.

		//life manager
		if (particlesContainer[i].life < maxLife) {
			particlesContainer[i].life += 1;
		}
		else { //init the particle (center and new random vector)
			if (waterfall) {
				particlesContainer[i].pos = glm::vec3(5, 8 + ((float)rand() / RAND_MAX) * 0.2f, ((float)rand() / RAND_MAX) * 6.f - 3.f);
				particlesContainer[i].vel = glm::vec3(-3, 0, ((float)rand() / RAND_MAX) * 6.f - 3.f); //random
				particlesContainer[i].life = 0;
			}
			else if (waterfall == false) {
			particlesContainer[i].pos = glm::vec3(0,1,0);
			particlesContainer[i].vel = glm::vec3(((float)rand() / RAND_MAX) * 6.f - 3.f, (10 + ((float)rand() / RAND_MAX) * 6.f - 3.f), ((float)rand() / RAND_MAX) * 6.f - 3.f); //random
			particlesContainer[i].life = 0;
			}
		}

		/*
		//check colision
		if (particlesContainer[i].pos.x <= -5 + radius || particlesContainer[i].pos.x >= 5 - radius) {
		particlesContainer[i].pos.x = -particlesContainer[i].pos.x - 2 * (-5 - particlesContainer[i].lastPos.x + radius);
		particlesContainer[i].vel.x = -particlesContainer[i].vel.x;
		}
		else if (particlesContainer[i].pos.y >= 10 - radius || particlesContainer[i].pos.y < 0 - radius) {

		}
		else if (particlesContainer[i].pos.z >= 5 - radius || particlesContainer[i].pos.z <= -5 + radius) {

		}*/

		//update partVerts vector with the new position
		partVerts[3 * i] = particlesContainer[i].pos.x;
		partVerts[3 * i + 1] = particlesContainer[i].pos.y;
		partVerts[3 * i + 2] = particlesContainer[i].pos.z;

	}

	if (updateRange < LilSpheres::maxParticles) {
		updateRange += 20; //each frame update 3 plus particles
	}

	LilSpheres::updateParticles(0, LilSpheres::maxParticles, partVerts);
	
}
void PhysicsCleanup() {
	//TODO
	delete[] partVerts;
}