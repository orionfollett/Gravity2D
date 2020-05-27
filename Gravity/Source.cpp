#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <cmath>
#include <map>
#include <string>
#include <vector>
//for testing purposes
#include <iostream>
#include <chrono>

class UI {

public:
	//InputMapping
	enum InputAction {
		ZOOMIN, ZOOMOUT, PAUSEMENU, PAUSESIM, EXIT, ADDBODY, DELETEBODY, ADDMASS, TOGGLEVECTORS, TOGGLECENTER
	};

	std::map<InputAction, olc::Key> inputMap;

	UI() {
		//default input values: (in future read from file)
		inputMap[ZOOMIN] = olc::X;
		inputMap[ZOOMOUT] = olc::SHIFT;
		inputMap[PAUSESIM] = olc::SPACE;
		inputMap[EXIT] = olc::END;
		inputMap[PAUSEMENU] = olc::ESCAPE;
		inputMap[ADDBODY] = olc::E;
		inputMap[DELETEBODY] = olc::D;
		inputMap[TOGGLEVECTORS] = olc::V;
		inputMap[ADDMASS] = olc::A;
		inputMap[TOGGLECENTER] = olc::C;
	}

	//save controls function
	//load controls function

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

	void normalize() {

		float mag = this->mag();

		this->x /= mag;
		this->y /= mag;
	}

	//multiplies vector by a scalar
	void scale(float sFactor) {
		this->x *= sFactor;
		this->y *= sFactor;
	}

	//returns square of magnitude of the vector
	float magSquared() {
		return ((this->x * this->x) + (this->y * this->y));
	}

	//clamps magnitude of vec between min and max
	void clamp(float min, float max) {
		if (this->magSquared() < (min*min)) {
			this->normalize();
			this->scale(min);
		}
		else if (this->magSquared() > (max*max)) {
			this->normalize();
			this->scale(max);
		}
			
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

	//static const int numBodies = 9;
	

	//Vector velocity
	//acceleration
	//position
	//radius
	//light source?
	bool active;
	int mass, radius;
	Vec2D pos, vel, acc;
	olc::Pixel color;

	Vec2D velDrawArrowEnd;

	bool toggleAsCenter = false;

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


	//STATIC FUNCTIONS

	static void UpdateVelandPos(std::vector<Body2D> &b, float fElapsedTime) {
		int len = b.size();

		for (int counter = 0; counter < len; counter++) {
			b[counter].UpdateVel(fElapsedTime);
			b[counter].UpdatePos(fElapsedTime);
		}
	}

	static void InitBodies(std::vector<Body2D> &b) {
		//sun, earth, moon
		b.push_back(Body2D(750, 400, 0, 150, 0, 0, 1, 9, olc::BLUE));
		b.push_back(Body2D(400, 400, 0, 0, 0, 0, 100, 35, olc::YELLOW));
		b.push_back(Body2D(790, 400, 0, 100, 0, 0, .01, 3, olc::GREY));
		
		/*
		b.push_back(Body2D(750 + 1000, 400, 0, 150, 0, 0, 1, 9, olc::BLUE));
		b.push_back(Body2D(400 + 1000, 400, 0, 0, 0, 0, 100, 35, olc::YELLOW));
		b.push_back(Body2D(790 + 1000, 400, 0, 100, 0, 0, .01, 3, olc::GREY));

		b.push_back(Body2D(750 - 1000, 400, 0, 150, 0, 0, 1, 9, olc::BLUE));
		b.push_back(Body2D(400 - 1000, 400, 0, 0, 0, 0, 100, 35, olc::YELLOW));
		b.push_back(Body2D(790 - 1000, 400, 0, 100, 0, 0, .01, 3, olc::GREY));

		*/

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
	static void UpdateGravity(std::vector<Body2D> &b) {
		
		int len = b.size();

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
				if (!b[in].active) {
					Body2D temp = b[b.size() - 1];
					b[b.size() - 1] = b[in];
					b[in] = temp;
					b.resize(b.size() - 1);
					len = b.size();
				}
			}

			if (!b[out].active) {
				Body2D temp = b[b.size() - 1];
				b[b.size() - 1] = b[out];
				b[out] = temp;
				b.resize(b.size() - 1);
				len = b.size();
			}
		}
	}

	static void ResolveCollision(std::vector<Body2D> &b,int i1, int i2) {
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
		

		//set radius of new planet/star
		b[index].radius = sqrt((b[index].radius * b[index].radius) + (b[toBeDeactivated].radius * b[toBeDeactivated].radius));

		//set mass to sum and deactivate other planet.
		b[index].mass += b[toBeDeactivated].mass;
		b[toBeDeactivated].active = false;
	}

	static void AddBodyAt(std::vector<Body2D> &b, Vec2D pos) {
		b.push_back(Body2D(pos.x, pos.y, 0, 0, 0, 0, 1, 10, olc::GREEN));
		//UpdateGravity(b);
	}

	static void DeleteBodyAt(std::vector<Body2D> &b, Vec2D mousePos) {
		for (int counter = 0; counter < b.size(); counter++) {
			if (Vec2D::VectorDistanceSquared(mousePos, b[counter].pos) < (b[counter].radius * b[counter].radius)) {
				b[counter].active = false;

				Body2D temp = b[b.size() - 1];
				b[b.size() - 1] = b[counter];
				b[counter] = temp;
				b.resize(b.size() - 1);
			}
		}
	}

	static void AddMassAt(std::vector<Body2D> &b, Vec2D mousePos) {
		for (int counter = 0; counter < b.size(); counter++) {
			if (Vec2D::VectorDistanceSquared(mousePos, b[counter].pos) < (b[counter].radius * b[counter].radius)) {
				b[counter].mass += 10;
			}
		}
	}

	static void ToggleCenterPlanet(std::vector<Body2D> &b, Vec2D mousePos) {
		for (int counter = 0; counter < b.size(); counter++) {
			if (Vec2D::VectorDistanceSquared(mousePos, b[counter].pos) < (b[counter].radius * b[counter].radius)) {
				b[counter].toggleAsCenter = !b[counter].toggleAsCenter;
			}
			else {
				b[counter].toggleAsCenter = false;
			}
		}
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
	float time = 0.0; //time in ms
	float nSec = 1;//number of seconds program has run

	//Body2D b[Body2D::numBodies];
	std::vector<Body2D> b;

	//toggle variables
	bool pause = false; //paused or not
	bool toggleVectors = true; //draw vel and acc vectors or not

	//WASD panning coordinates
	Vec2D worldCenter = Vec2D(0, 0);
	Vec2D worldCenterVel = Vec2D(0, 0);
	float worldVel = 200;

	//mouse extra panning coordinates
	Vec2D originalMousePanPos = Vec2D(0, 0); //used to store original mouse pos for panning camera
	bool isDragging = false; //very important variable that determines if camera is in process of moving

	//vector dragging
	int vectorDraggingIndex = -1;

	//vectorScale is scale vector factor - later add realism or sense of scale to this
	float vectorScale = 1 / 2.0;

	//zooming numbers
	float zoomFactor = 1.0;
	float zoomVel = 0;
	float zoomVelConstant = .3;

	//mouse constants - need to be dynamically figured out, only works for specific mouse
	const int L_CLICK = 0;
	const int R_CLICK = 1;
	const int M_CLICK = 2;

	
	UI IO = UI(); //carrier for all input things, will allow for changing controls in game by changing this 

	//sprite and decal for paused menu
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

		if (GetKey(IO.inputMap[UI::TOGGLEVECTORS]).bPressed) {
			toggleVectors = !toggleVectors;
		}

		// called once per frame
		Clear(olc::Pixel(0, 0, 0));
		time += fElapsedTime;

		
		//pan and zoom camera
		PanCamera(fElapsedTime);
		ZoomCamera(fElapsedTime);

		//INPUT
		EditObjects(b);

		//print debug values every second
		if (time > nSec) {
			nSec += 1;
			std::cout << " " << nSec << " seconds.\n";
		}

		//UPDATE GRAVITY- GETS CALLED TO UPDATE VECTORS EVEN WHEN PAUSED
		//update gravity also handles planet collisions as distances are all calculated
		Body2D::UpdateGravity(b);

		//doesnt get called if paused
		if (GetFPS() >= 20) {
			if (!pause) {

				//UPDATE POS AND VEL
				Body2D::UpdateVelandPos(b, fElapsedTime);
			}//end pause if
			else {
				DrawSprite(0, 0, pausedSprite);
			}
		}

		//DRAW
		DrawBodies(b);

		if (toggleVectors) {
			DrawBodyVelAndAccVectors(b);
		}

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

	void DrawBodies(std::vector<Body2D>& b) {
		//int len = sizeof(b) -1;
		int len = b.size();

		for (int counter = 0; counter < len; counter++) {

			//make sure worldcenter is corrected if there is a planet that is supposed to be the center
			if (b[counter].toggleAsCenter && !isDragging) {

				worldCenter.x = (ScreenWidth() / 2) - b[counter].pos.x;
				worldCenter.y = (ScreenHeight() / 2) - b[counter].pos.y;
			}
			//cancel center if camera if being panned
			if (isDragging) {
				b[counter].toggleAsCenter = false;
			}

			DrawBody(b[counter]);
		}
	}

	void DrawBodyVelAndAccVectors(std::vector<Body2D> &b) {
		
		//at min draw arrow length 1, max length 50
		//int min = 0;
		//int max = 1000000;
		
		//vectorScale is scale vector factor - later add realism or sense of scale to this
		

		for (int counter = 0; counter < b.size(); counter++) {

			Vec2D vel = Vec2D(b[counter].vel.x, b[counter].vel.y *-1);
			
			//either clamp between two values which is irretrievable, or scale which is retrievable
			//vel.clamp(min, max);
			vel.scale(vectorScale);

			vel = Vec2D::VectorAdd(vel, b[counter].pos);


			Vec2D acc = Vec2D(b[counter].acc.x, b[counter].acc.y * -1);
			//acc.clamp(min, max);
			acc.scale(vectorScale);

			acc = Vec2D::VectorAdd(acc, b[counter].pos);

			//only set velocity vector arrow if its not being changed
			if (vectorDraggingIndex == -1) {
				b[counter].velDrawArrowEnd = Vec2D(vel.x, vel.y);
			}
			
			DrawVector(b[counter].pos, b[counter].velDrawArrowEnd, olc::RED);
			DrawVector(b[counter].pos, acc, olc::GREEN);
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

	//collects various input that will add, delete, add or subtract mass, or move planets
	void EditObjects(std::vector<Body2D> &b) {
		//add body
		if (GetKey(IO.inputMap[UI::ADDBODY]).bHeld && GetMouse(L_CLICK).bPressed) {
			Body2D::AddBodyAt(b, Vec2D((GetMouseX() - worldCenter.x)/zoomFactor, (GetMouseY() - worldCenter.y) / zoomFactor));
		}
		else if (GetKey(IO.inputMap[UI::DELETEBODY]).bHeld && GetMouse(L_CLICK).bPressed) {
			Body2D::DeleteBodyAt(b, Vec2D((GetMouseX() - worldCenter.x) / zoomFactor, (GetMouseY() - worldCenter.y) / zoomFactor));
		}
		else if(GetKey(IO.inputMap[UI::ADDMASS]).bHeld && GetMouse(L_CLICK).bPressed) {
			Body2D::AddMassAt(b, Vec2D((GetMouseX() - worldCenter.x) / zoomFactor, (GetMouseY() - worldCenter.y) / zoomFactor));
		}
		else if (GetKey(IO.inputMap[UI::TOGGLECENTER]).bHeld && GetMouse(L_CLICK).bPressed) {
			Body2D::ToggleCenterPlanet(b, Vec2D((GetMouseX() - worldCenter.x) / zoomFactor, (GetMouseY() - worldCenter.y) / zoomFactor));
		}
		else if(pause && toggleVectors){
			DragVectors(b, Vec2D((GetMouseX() - worldCenter.x) / zoomFactor, (GetMouseY() - worldCenter.y) / zoomFactor), 20);
		}

		if (GetMouse(L_CLICK).bPressed) {
			//std::cout << "L CLICK\n";
		}
	}

	void DrawVector(Vec2D origin, Vec2D end, olc::Pixel color = olc::WHITE) {
		//center line
		DrawLine((origin.x * zoomFactor) + worldCenter.x, (origin.y * zoomFactor) + worldCenter.y, (end.x*zoomFactor) + worldCenter.x, (end.y*zoomFactor) + worldCenter.y, color);

		//draw arrowheads
		float angleForArrowHeads = 30 * (3.141596 / 180.0);
		float magForArrowHeads = 20;

		float lenSquared = Vec2D(origin.x - end.x, origin.y - end.y).magSquared();

		magForArrowHeads = lenSquared / 100;
		if (magForArrowHeads > 20) {
			magForArrowHeads = 20;
		}
		else if(lenSquared < 15){
			magForArrowHeads = .1;
		}

		float angleBetween = origin.angleBetween(end);
		float theta1 = angleBetween + angleForArrowHeads;

		float theta2 = angleBetween - angleForArrowHeads;

		DrawLine((end.x*zoomFactor) + worldCenter.x, (end.y*zoomFactor) + worldCenter.y, (end.x - magForArrowHeads*sin(theta1))*zoomFactor + worldCenter.x, (end.y - magForArrowHeads*cos(theta1))*zoomFactor + worldCenter.y, color);
		DrawLine((end.x*zoomFactor) + worldCenter.x, (end.y*zoomFactor) + worldCenter.y, (end.x - magForArrowHeads * sin(theta2))*zoomFactor + worldCenter.x, (end.y - magForArrowHeads * cos(theta2))*zoomFactor + worldCenter.y, color);
	}

	//allows user to click and drag on velocity vectors
	void DragVectors(std::vector<Body2D> &b, Vec2D mousePos, float buttonRadius) {
		//each vector needs a collision circle
		//click and drag on vectors
		//exclusive action, so if u click and are holding E, nothing should happen?
		//this function will be called in edit objects

		//only gets called on first click
		if (GetMouse(L_CLICK).bPressed) {
			//figure out which vector to drag

			//search through all vector collision circles
			int len = b.size();
			for (int counter = 0; counter < len; counter++) {
				//circle point collision detection
				if (Vec2D::VectorDistanceSquared(mousePos, b[counter].velDrawArrowEnd) < (buttonRadius * buttonRadius)) {
				

					//set vectorDragginIndex to body counter so that dont have to search everytime now
					vectorDraggingIndex = counter;
				}
			}
		}

		if (vectorDraggingIndex != -1 && GetMouse(L_CLICK).bHeld) {
			Vec2D newMousePos = Vec2D((GetMouseX() - worldCenter.x) / zoomFactor, (GetMouseY() - worldCenter.y) / zoomFactor);
			b[vectorDraggingIndex].velDrawArrowEnd = newMousePos;

			DrawCircle(b[vectorDraggingIndex].velDrawArrowEnd.x * zoomFactor + worldCenter.x, b[vectorDraggingIndex].velDrawArrowEnd.y * zoomFactor + worldCenter.y, buttonRadius);
		}
			
		//drag vector
		if (GetMouse(L_CLICK).bReleased && vectorDraggingIndex != -1) {

			//calculate old veldrawarrowend based off velocity which doesnt change since this is only called when paused
			Vec2D vel = Vec2D(b[vectorDraggingIndex].vel.x, b[vectorDraggingIndex].vel.y * -1);
			vel.scale(vectorScale);
			vel = Vec2D::VectorAdd(vel, b[vectorDraggingIndex].pos);

			//reverse process to get newVel
			Vec2D newVel = b[vectorDraggingIndex].velDrawArrowEnd;
			Vec2D pos = b[vectorDraggingIndex].pos;
			pos.scale(-1);
			newVel = Vec2D::VectorAdd(newVel, pos);

			newVel.scale(1 / vectorScale);
			newVel.y *= -1;

			//change vel
			b[vectorDraggingIndex].vel = newVel;
			
			vectorDraggingIndex = -1;
		}
	}

	void ConvertScreentoWorld(Vec2D &v) {
		v = Vec2D((v.x - worldCenter.x) / zoomFactor, (v.y - worldCenter.y) / zoomFactor);
	}

	void ConvertWorldtoScreen(Vec2D &v) {
		v = Vec2D((v.x * zoomFactor) + worldCenter.x, (v.y * zoomFactor) + worldCenter.y);
	}

};

int main()
{
	//while (1) {}

	Graphics g;
	if (g.Construct(900, 900, 1, 1))
		g.Start();

	g.OnUserDestroy();

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
//click and drag to pan around, shift and x to zoom -Done
//E + click to add planet at rest-done
//need way to give planet mass...maybe just a short click on a planet will add mass, so keep clicking on planet to increase mass -done
//D+click deletes planet - done

//need UI so when you click on planet it gives its radius and mass and coordinates in world space, 
//think more about scale of world, 
//add UI Pause menu with above controls listed

//space to pause simulation , but doesnt go to pause menu, allows freezing time - done

//SWITCH FROM DRAWING PIXEL CIRCLES TO DRAWING DECALS WITH DIFFERENT COLORS AND DIFFERENT SIZES FOR PLANETS AND STARS, WILL INCREASE PERFORMANCE SIGNIFICANTLY
//DRAG VECTORS TO GIVE PLANETS INITIAL VELOCITY
//INCREASE SIZE OF PLANETS THAT SWALLOW OTHER PLANETS - done

//click + c to toggle center to follow a specific body - done

//lock cursor
/*
RECT rect;

		rect.left = 0;
		rect.top = 0;

		rect.right = ScreenWidth();
		rect.bottom = ScreenHeight();

		ClipCursor(&rect);
*/



//KNOWN BUGS
/*
	OUTSTANDING
	1. mouse input for clicking doesnt work correctly on track pads, need to dynamically figure out what left and right click are on the mouse
	2. scale is not set properly, vel vectors look too big

	FIXED
	3. toggle planet cannot untoggle planet, only dragging can cancel a planet as toggled
	2. zooming and panning messes up clicking vectors
	*/