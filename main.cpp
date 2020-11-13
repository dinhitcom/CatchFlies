#include <GL/glut.h>
#include <math.h>
#include <vector>
#include "lib/loadpng.h"
#include "lib/process_image.h"
#include "lib/gl_texture.h"

#define WIDTH 720
#define HEIGHT 384
#define SCALE 3
#define INTERVAL 15

#define PLATFORMER_AMOUNT 6
#define CLOUD_AMOUNT 3

#define CELL_SIZE 24
#define MAX_X 30
#define MAX_Y 24

#define PI 3.141592
#define RAD 57.295780 

Rect rectBackground = {0, 720, 48, 384}; 
Rect rectGround = {0, 720, 0, 48};

Image imgBackground;
Image imgGround;

int Map[MAX_Y][MAX_X]; 
float gravity = -1.2f;
const float LEFT_BOUNDARY =  30.0f;
const float RIGHT_BOUNDARY = 690.0f;

class Platformer {
	public:
		static Image imageHolder;
		static void loadImage() {
			Load_Texture_Swap(&imageHolder, "img/Platformer.png");
			Zoom_Image(&imageHolder, SCALE);
		}
		 
		
		Rect rect;
		Image *img;
		void init(int _x, int _y) {
			Map[_y][_x] = Map[_y][_x+1] = Map[_y][_x+2] = Map[_y][_x+3] = 1;  
			img = &imageHolder;
			float x = (_x + 2) * CELL_SIZE;
			float y = _y * CELL_SIZE;
			rect.Left = x - img->w / 2;
			rect.Right = rect.Left + img->w;
			rect.Bottom = y;
			rect.Top = rect.Bottom + img->h;
		}
		
		void draw() {
			Map_Texture(img);
			Draw_Rect(&rect);
		}
};
Image Platformer::imageHolder;
Platformer platformers[PLATFORMER_AMOUNT];

class Cloud {
	public:
		static Image imageHolder;
		
		static void loadImage() {
			Load_Texture_Swap(&imageHolder, "img/Cloud.png");
			Zoom_Image(&imageHolder, SCALE);
		}
		
		Rect rect;
		Image *img;
		float x, y; 
		
		void init(float _x, float _y) {
			img = &imageHolder;
			x = _x;
			y = _y;
			updatePosition();
			rect.Bottom = y;
			rect.Top = rect.Bottom + img->h;
		}
		
		void draw() {
			Map_Texture(img);
			Draw_Rect(&rect);
		}
		
		void updatePosition() {
			rect.Left = x - img->w / 2;
			rect.Right = rect.Left + img->w;
		}
		
		void updatePositionX() {
			x -= 0.4f;
			if (x < -120.0f)
				x += 1080.0f;
			updatePosition();
		}
		
};

Image Cloud::imageHolder;
Cloud clouds[CLOUD_AMOUNT];

 class Line {
 	public:
 		static Image imageHolder[2];
 		static Rect rect;
 		Image *img;
 		float x, y, angle;
 		int player;
 		
 		Line(int _player, float _x, float _y, float _angle) {
 			player = _player;
 			x = _x;
 			y = _y;
 			angle = _angle;
 			img = &imageHolder[player];
 		}
 		
 		static void loadImage() {
 			Image img;
 			Load_Texture(&img, "img/Lines.png");
 			Crop_Image(&img, &imageHolder[0], 0, 0, 8, 4);
 			Crop_Image(&img, &imageHolder[1], 0, 4, 8, 4);
 			Zoom_Image(&imageHolder[0], SCALE);
 			Zoom_Image(&imageHolder[1], SCALE);
 			Delete_Image(&img);
 			rect.Left = -12.0f;
 			rect.Right = 12.0f;
 			rect.Bottom = -8.0f;
 			rect.Top = 8.0f;
 		}
 		
 		void draw() {
 			glTranslatef(x, y, 0.0f);
 			glRotatef(angle, 0.0f, 0.0f, 1.0f);
 			Map_Texture(img);
 			Draw_Rect(&rect);
 			glLoadIdentity();
 		}
 		
 };
 Image Line::imageHolder[2];
 Rect Line::rect;
 std::vector<Line> lines;
 
 class Fly {
 	public:
 		static Image imageHolder[2];
 		static float baseAccelerationX, baseAccelerationY;
 		static float maxVelocityX, maxVelocityY;
 		Rect rect;
 		Image *img;
 		float x, y, velocityX, velocityY, accelerationX, accelerationY, scale;
 		int timer, animation, region;
 		bool isAlive;
 		
 		static void loadImage() {
 			Image img;
 			Load_Texture(&img, "img/Fly.png");
 			Crop_Image(&img, &imageHolder[0], 0, 0, 10, 6);
 			Crop_Image(&img, &imageHolder[1], 0, 6, 10, 6);
 			Swap_Image(imageHolder[0].img, 10, 6);
 			Swap_Image(imageHolder[1].img, 10, 6);
 			Zoom_Image(&imageHolder[0], SCALE);
 			Zoom_Image(&imageHolder[1], SCALE);
 		}
 		
 		Fly(float _x, float _y, int _region) {
 			x = _x + rand() % 41 - 20;
 			y = _y + rand() % 41 - 20;
 			velocityX = maxVelocityX;
 			velocityY = 0.0f;
 			accelerationX = baseAccelerationX;
 			accelerationY = baseAccelerationY;
 			region = _region;
 			timer = 0;
 			animation = 0;
 			scale = 0.0f;
 			isAlive = false;
 			img = &imageHolder[0];
 		}
 		
 		void updatePosition() {
 			rect.Left = x - img->w /2 * scale;
 			rect.Right = rect.Left + img->w * scale;
 			rect.Bottom = y - img->h / 2 * scale;
 			rect.Top = rect.Bottom + img->h * scale;
 		}
 		
 		void draw() {
 			Map_Texture(img);
 			Draw_Rect(&rect);
 		}
 		
 		void update() {
 			 if (!isAlive && scale < 1.0f) {
 			 	scale += 0.05f;
 			 } else {
 			 	isAlive = true;
 			 }
 			 
 			 x += velocityX;
 			 y += velocityY;
 			 velocityX += accelerationX;
 			 velocityY += accelerationY;
 			 
 			 if (velocityX >= maxVelocityX || velocityX <= -maxVelocityX) {
 			 	accelerationX = velocityX < 0 ? baseAccelerationX : -baseAccelerationX;
 			 }
 			 if (velocityY >= maxVelocityY || velocityY <= -maxVelocityY) {
 			 	accelerationY = velocityY < 0 ? baseAccelerationY : -baseAccelerationY;
 			 }
 			 
 			 timer++;
 			 if (timer == 6) {
 			 	timer = 0;
 			 	animation = 1 - animation;
 			 	img = &imageHolder[animation];
 			 }
 			 updatePosition();
 		}
 		
 		bool isCaught(float _x, float _y ) {
 			if (_x - 20.0f < x && _x + 20.0f > x && _y - 6.0f < y && _y + 34.0f > y) {
 				return true;
 			} else {
 				return false;
 			}
 		}
 }; 
 Image Fly::imageHolder[2];
 float Fly::baseAccelerationX = 0.015f;
 float Fly::baseAccelerationY = 0.02f;
 float Fly::maxVelocityX = 0.3f;
 float Fly::maxVelocityY = 0.8f;
 std::vector<Fly> flies;
 
 class SpawnPoint {
 	public: 
 		float x, y;
 		SpawnPoint(float _x, float _y) {
 			x = _x;
 			y = _y;
 		}
 };
 
 class FlySpawner {
 	public:
 		SpawnPoint spawnPoints[6] = {
		 	SpawnPoint(100.0f, 300.0f), SpawnPoint(620.0f, 300.0f), SpawnPoint(360.0f, 280.0f),
		 	SpawnPoint(360.0f, 130.0f), SpawnPoint(100.0f, 120.0f), SpawnPoint(620.0f, 120.0f)
		 }; 
		int maxFliesAmount, spawnPointsCounter, timer; 
		
		FlySpawner (int _maxFliesAmount) {
			printf("Fly spawned");
			maxFliesAmount = _maxFliesAmount;
			timer = 60;
			spawnPointsCounter = sizeof(spawnPoints) / sizeof(SpawnPoint);
		}
		
		void update() {
			timer++;
			if (timer == 90) {
				timer = 0;
				if (flies.size() < maxFliesAmount) {
					bool isOccupated;
					int region;
					do {
						isOccupated = false;
						region = rand() % spawnPointsCounter;
						for (Fly fly : flies) {
							if (fly.region == region) {
								isOccupated = true;
								break;
							}
						}
					} while (isOccupated);
					flies.push_back(Fly(spawnPoints[region].x, spawnPoints[region].y, region));
				}
			}
		}
 };
 FlySpawner FlySpawner(2);
 
 class Frog {
 	public:
 		static Image imageHolder[2][2][2];
 		static float mapOffset[2];
 		static float mapBaseAngle[2];
 		Rect rect;
 		Image *img;
 		float x, y, velocityX, velocityY, angle;
 		int player, direction, animation, action, angleDirection, score;
 		bool isJumping, isKeyPressed;
 		
 		static void loadImage() {
 			Image img;
 			Load_Texture(&img, "img/Frogs.png");
 			Crop_Image(&img, &imageHolder[0][1][0], 0, 0, 18, 16);
 			Crop_Image(&img, &imageHolder[0][1][1], 0, 16, 18, 16);
 			Crop_Image(&img, &imageHolder[1][1][0], 18, 0, 18, 16);
 			Crop_Image(&img, &imageHolder[1][1][1], 18, 16, 18, 16);
 			Swap_Image(imageHolder[0][1][0].img, 18, 16);
 			Swap_Image(imageHolder[0][1][1].img, 18, 16);
 			Swap_Image(imageHolder[1][1][0].img, 18, 16);
 			Swap_Image(imageHolder[1][1][1].img, 18, 16);
 			Zoom_Image(&imageHolder[0][1][0], SCALE);
 			Zoom_Image(&imageHolder[0][1][1], SCALE);
 			Zoom_Image(&imageHolder[1][1][0], SCALE);
 			Zoom_Image(&imageHolder[1][1][1], SCALE);
 			Flip_Horizontal(&imageHolder[0][1][0], &imageHolder[0][0][0]);
 			Flip_Horizontal(&imageHolder[0][1][1], &imageHolder[0][0][1]);
 			Flip_Horizontal(&imageHolder[1][1][0], &imageHolder[1][0][0]);
 			Flip_Horizontal(&imageHolder[1][1][1], &imageHolder[1][0][1]);
 			Delete_Image(&img);        
		}
			                   
 		
 		
 		void init(int _player) {
 			player = _player;
 			direction = 1 - player;
 			animation = 0;
 			action = 0;
 			isJumping = false;
 			isKeyPressed = false;
 			float offset = 11.0f * CELL_SIZE * (player == 0 ? -1 : 1);
 			x = (WIDTH / 2) + offset;
 			y = CELL_SIZE * 2.0f;
 			updateImage();
 			updatePosition();
 		}
 		
 		void updateImage() {
 			img = &imageHolder[player][direction][animation];
 		}
 		
 		void updatePosition() {
 			rect.Left = x - img->w / 2;
 			rect.Right = rect.Left + img->w;
 			rect.Bottom = y;
 			rect.Top = rect.Bottom + img->h;
 		}
 		
 		void draw() {
 			Map_Texture(img);
 			Draw_Rect(&rect);
 		}
		  
		 void jump() {
		 	if (!isJumping) {
		 		isJumping = true;
		 		animation = 1;
		 		updateImage();
		 	} 
		 }
		 
		 void keyDown() {
		 	isKeyPressed = true;
		 }		
		 
		 void keyUp() {
		 	isKeyPressed = false;
//		 	if (!isJumping) {
//		 		//velocityY = 10.0f;
//		 		if (direction == 1) {
//		 			velocityX = 10.0f;
//		 		} else {
//		 			velocityX = -10.0f;
//		 		}
//		 		jump();
//			 }
		 	
		 }
		 
		 void prepareJump() {
		 	if (action == 0) {
		 		action = 1;
		 		angleDirection = direction;
		 		angle = mapBaseAngle[direction];
		 	} 
		 }
		 
		 void endPrepareJump() {
		 	if (action == 1) {
		 		action = 2;
		 	}
		 }
		 
		 void update() {
		 	if (!isJumping) {
		 		//printf("\n vx = %f, vy = %f, gravity = %f", velocityX, velocityY, gravity);
		 		if (isKeyPressed) {
		 			prepareJump();
		 		} else {
		 			endPrepareJump();
		 		}
		 		if (action > 0) {
		 			if (action == 2) {
		 				action = 0;
		 				jump();
		 			} else {
		 				angle += mapOffset[angleDirection];
		 				if (reachedAngle[direction][angleDirection](angle)) {
		 					angleDirection = 1 - angleDirection;
		 				}
		 				float angle2 = angle / RAD;
		 				float x2 = x;
		 				float y2 = y + 4.0f;
		 				float velocityX2, velocityY2;
		 				velocityX2 = cos(angle2) * 4 + (direction == 0 ? angle2 - PI : angle2) * 9;
		 				velocityY2 = sin(angle2) * 21;
		 				velocityX = velocityX2;
		 				velocityY = velocityY2;
		 				for (int i = 0; i < 18; i++) {
		 					x2 += velocityX2;
		 					y2 += velocityY2;
		 					if (i % 3 == 2) {
		 						angle2 = atan2(velocityY2, velocityX2) * RAD;
		 						lines.push_back(Line(player, x2, y2, angle2));
		 					}
		 					velocityY2 += gravity;
		 				}	
		 			}
		 		}
		 	} else {
		 		float oldY = y;
		 		x += velocityX;
		 		y += velocityY;
		 		velocityY += gravity;
		 		if (velocityY < - 24.0f) {
		 			velocityY = -24.0f;
		 		}
				if (velocityY <= 0.0f) {
					int col1 = (x - 9.0f) / CELL_SIZE;
					int col2 = (x + 9.0f) / CELL_SIZE;
					int oldRow = oldY / CELL_SIZE;
					int row = y / CELL_SIZE;
					if ((!Map[oldRow][col1] && !Map[oldRow][col2]) && (Map[row][col1] || Map[row][col2])) {
						isJumping = false;
						y = (row + 1) * CELL_SIZE;
						velocityX = 0.0f;
						velocityY = 0.0f;
						animation = 0;
						updateImage();
					}
				}
				if (reachedBoundary[direction](x)) {
					direction = 1 - direction;
					velocityX = -velocityX;
					updateImage();
				}
				updatePosition();	
		 	}
		 }
		 
 		static bool reachedLeftBoundary(float x) {
 			return x < LEFT_BOUNDARY;
 		}
		 
		static bool reachedRightBoundary(float x) {
 			return x > RIGHT_BOUNDARY;
 		} 
 		
 		static bool (*reachedBoundary[2])(float x);
 		
 		static bool reachedMaxAngleLeft(float angle) {
 			return angle <= 110.0f;
 		}
 		
 		static bool reachedMinAngleLeft(float angle) {
 			return angle >= 160.0f;
 		}
 		
 		static bool reachedMaxAngleRight(float angle) {
 			return angle >= 70.0f;
 		}
 		
 		static bool reachedMinAngleRight(float angle) {
 			return angle <= 20.0f; 
 		}
 		
 		static bool (*reachedAngle[2][2])(float angle);
 		
 };
 Image Frog::imageHolder[2][2][2];
 float Frog::mapOffset[2] = {-1.5f, 1.5f};
 float Frog::mapBaseAngle[2] = {160.0f, 20.0f};
 bool (*Frog::reachedBoundary[2])(float x) = {Frog::reachedLeftBoundary, Frog::reachedRightBoundary};
 bool (*Frog::reachedAngle[2][2])(float angle) = {
 	{reachedMaxAngleLeft, reachedMinAngleLeft},
 	{reachedMinAngleRight, reachedMaxAngleRight}
 };
 
 Frog frogs[2];
 
 
 
 
 
 
//====================================================// 
 
void Display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	Map_Texture(&imgBackground);
	Draw_Rect(&rectBackground);
	
	for (Line line : lines) {
		line.draw();
	}
	
	Map_Texture(&imgGround);
	Draw_Rect(&rectGround);
	
	for (int i = 0; i < CLOUD_AMOUNT; i++) {
		clouds[i].draw();
	}
		
	for (int i = 0; i < flies.size(); i++) {
		flies[i].draw();
	}
	
	for (int i = 0; i < PLATFORMER_AMOUNT; i++) {
		platformers[i].draw();
	}
	
	frogs[0].draw();
	frogs[1].draw();
	
	glutSwapBuffers();
}

void loadBackground() {
	Load_Texture_Swap(&imgBackground, "img/Background.png");
	Zoom_Image(&imgBackground, SCALE);
	Load_Texture_Swap(&imgGround, "img/Ground.png");
	Zoom_Image(&imgGround, SCALE);
}

void initMap() {
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < MAX_X; j++) {
			Map[i][j] = 1;
		}
	for (int i = 2; i < MAX_Y; i++)
	 	for (int j = 0; j < MAX_X; j++) {
	 		Map[i][j] = 0;
	 	}
}

void initPlatformers() {
	Platformer::loadImage();
	platformers[0].init(7, 5);
	platformers[1].init(19, 5);
	platformers[2].init(4, 9);
	platformers[3].init(22, 9);
	platformers[4].init(9, 13);
	platformers[5].init(17, 13);
}

void initClouds() {
	Cloud::loadImage();
	clouds[0].init(570.0f, 210.0f);
	clouds[1].init(930.0f, 300.0f);
	clouds[2].init(240.0f, 255.0f);	
}

void initFrogs() {
	Frog::loadImage();
	frogs[0].init(0);
	frogs[1].init(1);
}

void initGame() {
	loadBackground();
	initMap();	
	initPlatformers();
	initClouds(); 
	initFrogs();
	Line::loadImage();
	Fly::loadImage();
}


void initGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, WIDTH, HEIGHT);
	gluOrtho2D(0, WIDTH, 0, HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_2D);
}

void timer(int value) {
	for (int i = 0; i < CLOUD_AMOUNT; i++) {
		clouds[i].updatePositionX();
	}
	FlySpawner.update();
	for (Fly fly : flies) {
		fly.update();
	}
	lines.clear();
	frogs[0].update();
	frogs[1].update();
	
	glutPostRedisplay();
	glutTimerFunc(INTERVAL, timer, 0);
}

void keyboardDown(GLubyte key, int x, int y) {
	switch (key) {
		case 32:
			frogs[0].keyDown();
			break;
		case 13:
			frogs[1].keyDown();
			break;
	}
}

void keyboardUp(GLubyte key, int x, int y) {
	switch (key) {
		case 32:
			frogs[0].keyUp();
			break;
		case 13:
			frogs[1].keyUp();
			break;
	}
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	int POS_X = (glutGet(GLUT_SCREEN_WIDTH) - WIDTH) / 2;
	int POS_Y = (glutGet(GLUT_SCREEN_HEIGHT) - HEIGHT) / 2;
	glutInitWindowPosition(POS_X, POS_Y);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Catch Flies");
	initGL();
	initGame();
	glutDisplayFunc(Display);
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
	glutTimerFunc(0, timer, 0);
	glutMainLoop();
	return 0;
}


//TODO
/* --
	- Select jump direction using keyboard
	- loading screen, menu;
	- select map;
	- credits;
	- Select difficult mode (moving platformers, moving flies, faster aiming (2.0f);
	- Select jump aiming mode: auto, manual arrow up/down;
-- */
