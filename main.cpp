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

Rect rectBackground = {0, 720, 48, 384}; 
Rect rectGround = {0, 720, 0, 48};

Image imgBackground;
Image imgGround;

int Map[MAX_Y][MAX_X]; 
float gravity = -1.0f;
const float LEFT_BOUNDARY =  30.0f;
const float RIGHT_BOUNDARY = 690.0f;

class Platformer {
	public:
		static Image imgSave;
		static void loadImage() {
			Load_Texture_Swap(&imgSave, "img/Platformer.png");
			Zoom_Image(&imgSave, SCALE);
		}
		 
		
		Rect rect;
		Image *img;
		void init(int _x, int _y) {
			Map[_y][_x] = Map[_y][_x+1] = Map[_y][_x+2] = Map[_y][_x+3] = 1;  
			img = &imgSave;
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
Image Platformer::imgSave;
Platformer platformers[PLATFORMER_AMOUNT];

class Cloud {
	public:
		static Image imgSave;
		
		static void loadImage() {
			Load_Texture_Swap(&imgSave, "img/Cloud.png");
			Zoom_Image(&imgSave, SCALE);
		}
		
		Rect rect;
		Image *img;
		float x, y; 
		
		void init(float _x, float _y) {
			img = &imgSave;
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
Image Cloud::imgSave;
Cloud clouds[CLOUD_AMOUNT];
 
 class Frog {
 	public:
 		static Image imgSave[2][2][2];
 		static float mapOffset[2];
 		static float mapBaseAngle[2];
 		Rect rect;
 		Image *img;
 		float x, y, speedX, speedY, angle;
 		int player, direction, animation, action, angleDirection, score;
 		bool isJumping, isKeyPressed;
 		
 		static void loadImage() {
 			Image img;
 			Load_Texture(&img, "img/Frogs.png");
 			Crop_Image(&img, &imgSave[0][1][0], 0, 0, 18, 16);
 			Crop_Image(&img, &imgSave[0][1][1], 0, 16, 18, 16);
 			Crop_Image(&img, &imgSave[1][1][0], 18, 0, 18, 16);
 			Crop_Image(&img, &imgSave[1][1][1], 18, 16, 18, 16);
 			Swap_Image(imgSave[0][1][0].img, 18, 16);
 			Swap_Image(imgSave[0][1][1].img, 18, 16);
 			Swap_Image(imgSave[1][1][0].img, 18, 16);
 			Swap_Image(imgSave[1][1][1].img, 18, 16);
 			Zoom_Image(&imgSave[0][1][0], SCALE);
 			Zoom_Image(&imgSave[0][1][1], SCALE);
 			Zoom_Image(&imgSave[1][1][0], SCALE);
 			Zoom_Image(&imgSave[1][1][1], SCALE);
 			Flip_Horizontal(&imgSave[0][1][0], &imgSave[0][0][0]);
 			Flip_Horizontal(&imgSave[0][1][1], &imgSave[0][0][1]);
 			Flip_Horizontal(&imgSave[1][1][0], &imgSave[1][0][0]);
 			Flip_Horizontal(&imgSave[1][1][1], &imgSave[1][0][1]);
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
 			img = &imgSave[player][direction][animation];
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
		 
		 void update() {
		 	if (isJumping) {
		 		float oldY = y;
		 		x += speedX;
		 		y += speedY;
		 		speedY += gravity;
		 		if (speedY < - 24.0f) 
		 			speedY = -24.0f;
				if (speedY <= 0.0f) {
					int col1 = (x - 9.0f) / CELL_SIZE;
					int col2 = (x + 9.0f) / CELL_SIZE;
					int oldRow = oldY / CELL_SIZE;
					int row = y / CELL_SIZE;
					if ((!Map[oldRow][col1] && !Map[oldRow][col2]) && (Map[row][col1] || Map[row][col2])) {
						isJumping = false;
						y = (row + 1) * CELL_SIZE;
						speedX = 0.0f;
						speedY = 0.0f;
						animation = 0;
						updateImage();
					}
				}
				if (checkBoundary[direction](x)) {
					direction = 1 - direction;
					speedX = - speedX;
					updateImage();
				}
				updatePosition();	
		 	}
		 }
		 
		 void keyDown() {
		 	isKeyPressed = true;
		 }		
		 
		 void keyUp() {
		 	isKeyPressed = false;
		 	speedX = 10.0f;
		 	speedY = 10.0f;
		 	jump();
		 }
		 
 		static bool reachedLeftBoundary(float x) {
 			return x < LEFT_BOUNDARY;
 		}
		 
		static bool reachedRightBoundary(float x) {
 			return x < RIGHT_BOUNDARY;
 		} 
 		
 		static bool (*checkBoundary[2])(float x);
 };
 Image Frog::imgSave[2][2][2];
 float Frog::mapOffset[2] = {-1.0f, 1.0f};
 float Frog::mapBaseAngle[2] = {160.0f, 20.0f};
 bool (*Frog::checkBoundary[2])(float x) = {Frog::reachedLeftBoundary, Frog::reachedRightBoundary};
 
 Frog frogs[2];
 
 
void Display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	Map_Texture(&imgBackground);
	Draw_Rect(&rectBackground);
	Map_Texture(&imgGround);
	Draw_Rect(&rectGround);
	for (int i = 0; i < CLOUD_AMOUNT; i++) 
		clouds[i].draw();
		
	for (int i = 0; i < PLATFORMER_AMOUNT; i++) 
		platformers[i].draw();
	
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
		for (int j = 0; j < MAX_X; j++)
			Map[i][j] = 1;
	for (int i = 2; i < MAX_Y; i++)
	 	for (int j = 0; j < MAX_X; j++)
	 		Map[i][j] = 0;
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
	for (int i = 0; i < CLOUD_AMOUNT; i++) 
		clouds[i].updatePositionX();
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