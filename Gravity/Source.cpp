#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <cmath>
#include <map>
#include <string>

//for testing purposes
#include <iostream>
#include <chrono>

class UI {

public:
	//InputMapping
	enum InputAction {
		ZOOMIN, ZOOMOUT, PAUSEMENU, PAUSESIM, EXIT, ADDPLANET
	};

	std::map<InputAction, olc::Key> inputMap;

	UI() {
		//default input values: (in future read from file)
		inputMap[ZOOMIN] = olc::X;
		inputMap[ZOOMOUT] = olc::SHIFT;
		inputMap[PAUSESIM] = olc::SPACE;
		inputMap[EXIT] = olc::END;
		inputMap[PAUSEMENU] = olc::ESCAPE;
	}





};


class Vec2D {
public:
	float x, y;

	Vec2D(float xPos = 0.0f, float yPos = 0.0f) {
		x = xPos;
		y = yPos;
	}

	//returns the magnitude of the vector compared to 0, 0
	float mag() {
		return sqrtf(x*x + y*y);
	}

	//returns distance between this vector and vector passed in.
	float dist(Vec2D v2) {
		return sqrtf(VectorDistanceSquared(*this, v2));
	}

	//angle in radians, use sinAngleBetween, cosAngleBetween, for more efficiency (I think).
	float angleBetween(Vec2D v2) {
		return atan2f((v2.x - this->x), (v2.y - this->y));
	}

	float sinAngleBetween(Vec2D v2) {
		//opposite over hypoteneuse
		return -1*((v2.y - this->y) / sqrtf(VectorDistanceSquared(*this, v2)));
	}

	float cosAngleBetween(Vec2D v2) {
		//adjacent over hypoteneuse
		return ((v2.x - this->x) / sqrtf(VectorDistanceSquared(*this, v2)));
	}

	float tanAngleBetween(Vec2D v2) {
		return (this->sinAngleBetween(v2) / this->cosAngleBetween(v2));
	}

	//Static functions
	//adds two vectors using vector addition
	static Vec2D VectorAdd(Vec2D v1, Vec2D v2) {
		return Vec2D(v1.x + v2.x, v1.y + v2.y);
	}

	//returns distance between two vectors squared
	static float VectorDistanceSquared(Vec2D v1, Vec2D v2) {
		return ((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y));
	}
};

class Body2D {
public:

	static const int METERS_TO_PIXELS = 500000000; //number of square meters per pixel
	static const int STAR_ENLARGEMENT_FACTOR = 10; //multiply star radius by this number to make star visible
	static const int PLANET_ENLARGEMENT_FACTOR = 20; // same as above but for planets

	static const int numBodies = 9;

	//Vector velocity
	//acceleration
	//position
	//radius
	//light source?
	bool active;
	int mass, radius;
	Vec2D pos, vel, acc;
	olc::Pixel color;

	Body2D(float xPos, float yPos, float xVel, float yVel, float xAcc, float yAcc, int m, int r, olc::Pixel colorPixel) {
		pos = Vec2D(xPos, yPos);
		vel = Vec2D(xVel, yVel);
		acc = Vec2D(xAcc, yAcc);
		mass = m;
		radius = r;
		active = true;
		color = colorPixel;
	}

	Body2D() = default;

	void UpdateVel(float fElapsedTime) {
		if (this->active) {
			this->vel.x += (this->acc.x * fElapsedTime);
			this->vel.y += (this->acc.y * fElapsedTime);
		}
	}

	void UpdatePos(float fElapsedTime) {
		if (this->active) {
			this->pos.x += (this->vel.x * fElapsedTime);
			this->pos.y -= (this->vel.y * fElapsedTime);
		}
	}

	static void UpdateVelandPos(Body2D b[], float fElapsedTime) {
		int len = numBodies;

		for (int counter = 0; counter < len; counter++) {
			b[counter].UpdateVel(fElapsedTime);
			b[counter].UpdatePos(fElapsedTime);
		}
	}

	static void InitBodies(Body2D b[]) {
		//sun, earth, moon
		b[0] = Body2D(750, 400, 0, 150, 0, 0, 1, 9, olc::BLUE);
		b[1] = Body2D(400, 400, 0, 0, 0, 0, 100, 35, olc::YELLOW);
		b[2] = Body2D(790, 400, 0, 100, 0, 0, .01, 3, olc::GREY);
		
		b[3] = Body2D(750 + 1000, 400, 0, 150, 0, 0, 1, 9, olc::BLUE);
		b[4] = Body2D(400 + 1000, 400, 0, 0, 0, 0, 100, 35, olc::YELLOW);
		b[5] = Body2D(790 + 1000, 400, 0, 100, 0, 0, .01, 3, olc::GREY);

		b[6] = Body2D(750 - 1000, 400, 0, 150, 0, 0, 1, 9, olc::BLUE);
		b[7] = Body2D(400 - 1000, 400, 0, 0, 0, 0, 100, 35, olc::YELLOW);
		b[8] = Body2D(790 - 1000, 400, 0, 100, 0, 0, .01, 3, olc::GREY);

		/*
		//collision
		b[0] = Body2D(600, 600, -160, 200, 0, 0, 1, 30, olc::BLUE);
		b[1] = Body2D(400, 400, 50, 0, 0, 0, 10, 30, olc::YELLOW);
		*/

		//b[0] = Body2D(100, 120, 200, 0, 0, 0, 10, 10, olc::DARK_RED);
		//b[1] = Body2D(400, 400, 0, 0, 0, 0, 100, 10, olc::GREEN);
		//b[3] = Body2D(400, 1800, 100, 100, 0, 0, 3, 15, olc::YELLOW);
	}

	//Updates gravity on all the objects, and resolves collisions
	static void UpdateGravity(Body2D b[]) {
		
		const int len = numBodies;

		for (int counter = 0; counter < len; counter++) {
			b[counter].acc.x = 0;
			b[counter].acc.y = 0;
		}
		//float distMatrix[len][len] = { {0} };
		for (int out = 0; out < len; out++) {
			for (int in = 0; in < len; in++) {
				if (in != out && b[in].active && b[out].active) {
					float rSquared = Vec2D::VectorDistanceSquared(b[in].pos, b[out].pos);
					//distMatrix[in][out] = rSquared;
					//distMatrix[out][in] = rSquared;

					float gravity = 0.0;
					if (rSquared != 0) {
						gravity = 100000 * (b[in].mass / rSquared);
					}
					b[out].acc.x += gravity * b[out].pos.cosAngleBetween(b[in].pos);
					b[out].acc.y += gravity * b[out].pos.sinAngleBetween(b[in].pos);

					//resolve collisions
					float radius1 = b[in].radius / 2;
					float radius2 = b[out].radius / 2;
					float minDist = radius1 + radius2;
					if (rSquared < minDist*minDist) {
						ResolveCollision(b, in, out);
					}
				}
			}
		}
	}

	static void ResolveCollision(Body2D b[],int i1, int i2) {
		int index = 0;
		int toBeDeactivated = 0;
		if (b[i1].mass > b[i2].mass) {
			toBeDeactivated = i2;
			index = i1;
		}
		else {
			toBeDeactivated = i1;
			index = i2;
		}

		//set velocties to conserve momentum
		b[index].vel.x = ((b[i1].mass * b[i1].vel.x) + (b[i2].mass * b[i2].vel.x)) / (b[i1].mass + b[i2].mass);
		b[index].vel.y = ((b[i1].mass * b[i1].vel.y) + (b[i2].mass * b[i2].vel.y)) / (b[i1].mass + b[i2].mass);
		
		//set mass to sum and deactivate other planet.
		b[index].mass += b[toBeDeactivated].mass;
		b[toBeDeactivated].active = false;
	}
};

class Graphics : public olc::PixelGameEngine
{
public:
	Graphics()
	{
		sAppName = "Gravity Simulation";
	}

public:
	//variables AAA
	float time = 0.0;
	float nSec = 1;

	Body2D b[Body2D::numBodies];
	
	bool pause = false;
	
	//WASD panning coordinates
	Vec2D worldCenter = Vec2D(0, 0);
	Vec2D worldCenterVel = Vec2D(0, 0);
	float worldVel = 200;
	//mouse extra panning coordinates
	Vec2D originalMousePanPos = Vec2D(0, 0);
	bool isDragging = false;

	//zooming numbers
	float zoomFactor = 1.0;
	float zoomVel = 0;
	float zoomVelConstant = .3;

	//mouse constants
	const int L_CLICK = 0;
	const int R_CLICK = 1;
	const int M_CLICK = 2;

	UI IO = UI();

	olc::Sprite* pausedSprite = nullptr;
	olc::Decal* pausedDecal = nullptr;


	

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		Body2D::InitBodies(b);

		pausedSprite = new olc::Sprite("../Assets/paused.png");
		pausedDecal = new olc::Decal(pausedSprite);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		//pause
		if (GetKey(IO.inputMap[UI::PAUSESIM]).bPressed) {
			pause = !pause;
		}

		// called once per frame
		Clear(olc::Pixel(0, 0, 0));
		time += fElapsedTime;

		PanCamera(fElapsedTime);
		ZoomCamera(fElapsedTime);

		//print debug values every second
		if (time > nSec) {
			nSec += 1;
			std::cout <<  " " << nSec << " seconds.\n";
		}

		//doesnt get called if paused
		if (GetFPS() >= 10) {
			if (!pause) {
				//UPDATE
				//update gravity also handles planet collisions
				Body2D::UpdateGravity(b);
				Body2D::UpdateVelandPos(b, fElapsedTime);
			}//end pause if
			else {
				DrawSprite(0, 0, pausedSprite);
			}
		}

		//DRAW
		DrawBodies(b);

		//quit program
		if (GetKey(IO.inputMap[UI::EXIT]).bPressed) {
			return false;
		}

		return true;
	}

	void DrawBody(Body2D b) {
		if (b.active) {
			FillCircle((b.pos.x * zoomFactor) + worldCenter.x, (b.pos.y * zoomFactor) + worldCenter.y, b.radius * zoomFactor, b.color);
		}		
	}

	void DrawBodies(Body2D b[]) {
		//int len = sizeof(b) -1;
		int len = Body2D::numBodies;

		for (int counter = 0; counter < len; counter++) {
			DrawBody(b[counter]);
		}
	}

	void PanCamera(float fElapsedTime) {

		//Mouse way for camera panning, click and drag
		
		if (GetMouse(R_CLICK).bHeld && !isDragging) {
			originalMousePanPos = Vec2D(GetMouseX(), GetMouseY());
			isDragging = true;
		}
		
		if (isDragging) {	
			Vec2D diff = Vec2D(GetMouseX() - originalMousePanPos.x, GetMouseY() - originalMousePanPos.y);
			worldCenter.x += diff.x;
			worldCenter.y += diff.y;

			originalMousePanPos = Vec2D(GetMouseX(), GetMouseY());
		}
		
		if (GetMouse(R_CLICK).bReleased) {
			isDragging = false;
		}

		//WASD way for camera panning
		/*
		if (GetKey(olc::W).bHeld) {
			worldCenterVel.y = worldVel;
		}
		else if (GetKey(olc::S).bHeld) {
			worldCenterVel.y = -1*worldVel;
		}
		else {
			worldCenterVel.y = 0;
		}

		if (GetKey(olc::D).bHeld) {
			worldCenterVel.x = -1 * worldVel;
		}
		else if (GetKey(olc::A).bHeld) {
			worldCenterVel.x = worldVel;
		}
		else {
			worldCenterVel.x = 0;
		}

		worldCenter.x += (worldCenterVel.x * fElapsedTime);
		worldCenter.y += (worldCenterVel.y * fElapsedTime);
		*/
	}

	void ZoomCamera(float fElapsedTime) {
		if (GetKey(IO.inputMap[UI::ZOOMIN]).bHeld) {
			zoomVel = zoomVelConstant;
		}
		else if (GetKey(IO.inputMap[UI::ZOOMOUT]).bHeld) {
			zoomVel = -1 * zoomVelConstant;
		}
		else {
			zoomVel = 0;
		}

		zoomFactor *= ((zoomVel * fElapsedTime) + 1);
	}
};

int main()
{
	Graphics g;
	if (g.Construct(900, 900, 1, 1))
		g.Start();

	g.OnUserDestroy();
	//add some items to the map
	return 0;
}

//LIST OF THINGS TO DO
//ZOOMING IN - done
//PANNING - done, can change so its mouse drag that pans around universe
//PLANETS COMBINE MASS WHEN THEY COLLIDE - done
//PAUSING - done, can add pause menu

//PATH TRACING
//ADD PLANETS ON THE FLY, CLICK, PROMPT POPS UP, TYPE MASS, SO ON, AND PLANET APPEARS
//INTERACTIVE GAME SOMETHING
//LIGHT SOURCES

//UI IDEAS
//click and drag to pan around, scoll to zoom.
//shift click to add planet at rest, clicking and holding to drag on planet to "throw" it, 
//gives it an impulse, use momentum to resolve impulse
//need way to give planet mass...maybe just a short click on a planet will add mass, so keep clicking on planet to increase mass
//need way to delete planet...
//need UI so when you click on planet it gives its radius and mass and coordinates in world space, 
//think more about scale of world, 
//add UI Pause menu with above controls listed

//space to pause


//SWITCH FROM DRAWING PIXEL CIRCLES TO DRAWING DECALS WITH DIFFERENT COLORS AND DIFFERENT SIZES FOR PLANETS AND STARS, WILL INCREASE PERFORMANCE SIGNIFICANTLY

//lock cursor
/*
RECT rect;

		rect.left = 0;
		rect.top = 0;

		rect.right = ScreenWidth();
		rect.bottom = ScreenHeight();

		ClipCursor(&rect);
*/