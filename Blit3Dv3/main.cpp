/*
	Simple example of loading/rotating/displaying sprites in Blit3D
*/
//memory leak detection
#define CRTDBG_MAP_ALLOC
#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif
#endif  // _DEBUG

#include <stdlib.h>
#include <crtdbg.h>

#include "Blit3D.h"
#include "Boid.h"
#include "DieRoller.h"

Blit3D *blit3D = NULL;
DieRoller *rd = NULL;
std::vector<Boid*> boidList;

Sprite *background = NULL;

//mouse cursor
float cursor_x = 0;
float cursor_y = 0;
bool leftClick = false;

void Init()
{
	//load our background image: the arguments are upper-left corner x, y, width to copy, height to copy, and file name.
	background = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\underwater_5k-1920x1200.png");
	
	rd = new DieRoller();

	for (int i = 0; i < 10; i++)//Create 10 RED boid that goes fast
	{									  //cohe   alig   separ
		Boid *boid = new Boid(500.f, 300.f, 300.f, 100.f, 35.f, &boidList, glm::radians(rd->RollDN(360) - 180), 
			glm::vec2(blit3D->screenWidth/2, blit3D->screenHeight/2), blit3D->MakeSprite(0, 0, 100, 50, "Media\\Boid2.png")); 
		boidList.push_back(boid);
	}
}

void DeInit(void)
{
	//any sprites/fonts still allocated are freed automatically by the Blit3D object when we destroy it
	for (auto b : boidList)
		delete b;
}

void Update(double seconds)
{
	for (auto b : boidList)//Update every boid in the world
		b->Update((float)seconds);

	if (leftClick)//If the left button of the mouse is clicked, create new boid
	{									  //cohe   alig   separ
		Boid *boid = new Boid(200.f, 200.f, 200.f, 100.f, 35.f, &boidList, glm::radians(180.f), glm::vec2(cursor_x, cursor_y), blit3D->MakeSprite(0, 0, 100, 50, "Media\\BoidWhite.png"));
		boidList.push_back(boid);
	}

}

void Draw(void)
{
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);	//clear colour: r,g,b,a 	
	// wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	background->Blit(blit3D->screenWidth / 2, blit3D->screenHeight / 2);

	//draw stuff here
	for (auto b : boidList)
		b->Draw();
}

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence
}

void DoCursor(double x, double y)
{
	//convert values in case we are in a "fake" fullscreen,
	//becuase the OS reports mouse positions based on ACTUAL screen resolution.
	cursor_x = blit3D->screenWidth / blit3D->trueScreenWidth * (float)x;
	cursor_y = -blit3D->screenHeight / blit3D->trueScreenHeight * (float)y;
	
}

void DoMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		leftClick = true;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		leftClick = false;
}

int main(int argc, char *argv[])
{
	//memory leak detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//set X to the memory allocation number in order to force a break on the allocation:
	//useful for debugging memory leaks, as long as your memory allocations are deterministic.
	//_crtBreakAlloc = X;

	blit3D = new Blit3D(Blit3DWindowModel::BORDERLESSFULLSCREEN_1080P, 1920, 1080);

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);
	blit3D->SetDoMouseButton(DoMouseButton);
	blit3D->SetDoCursor(DoCursor);

	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);
	if (blit3D) delete blit3D;
}