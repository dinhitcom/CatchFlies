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
#define MATCH_DURATION 90
#define PLATFORMER_AMOUNT 6
#define CLOUD_AMOUNT 3

#define CELL_SIZE 24
#define MAX_X 30
#define MAX_Y 24

#define PI 3.141592
#define RAD 57.295780 

Rect rectBackground = {0, 720, 48, 384}; 
Rect rectGround = {0, 720, 0, 48};
Rect title, result, continueKey;

Image imgTitle;
Image imgContinueKey;
Image imgBackground;
Image imgGround;
Image imgNumbers[10];
Image imgResults[3];
int Map[MAX_Y][MAX_X];
int time, counter = 0; 
float gravity = -1.2f;
const float LEFT_BOUNDARY =  30.0f;
const float RIGHT_BOUNDARY = 690.0f;
bool isPlaying = false;
bool isMatchEnd = false;

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
 			 	scale = scale + 0.05f;
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
 
 class Score {
 	public:
 		Image *imgDigit1, *imgDigit2;
 		Rect rect, rect2;
 		
 		void init(int player) {
 			imgDigit1 = &imgNumbers[0];
 			imgDigit2 = &imgNumbers[0];
 			if (player == 0) {
 				rect.Left = 10;
				rect.Right = rect.Left + 6 * SCALE;
				rect.Bottom = 5;
				rect.Top = rect.Bottom + 7 * SCALE;
				rect2.Left = rect.Right + 2;
				rect2.Right = rect2.Left + 6 * SCALE;
				rect2.Bottom = rect.Bottom;
				rect2.Top = rect.Top;
 			} else 
			 	if (player == 1) {
 					rect.Left = WIDTH - (10 + 2 + 6 * SCALE * 2);
					rect.Right = rect.Left + 6 * SCALE;
					rect.Bottom = 5;
					rect.Top = rect.Bottom + 7 * SCALE;
					rect2.Left = rect.Right + 2;
					rect2.Right = rect2.Left + 6 * SCALE;
					rect2.Bottom = rect.Bottom;
					rect2.Top = rect.Top;
 				}	
			
		}
		
		void updateScore(int score) {
			int digit1 = score / 10;
			int digit2 = score % 10;
			imgDigit1 = &imgNumbers[digit1];
 			imgDigit2 = &imgNumbers[digit2];
		}
		
		void draw() {
			Map_Texture(imgDigit1);
			Draw_Rect(&rect);
			Map_Texture(imgDigit2);
			Draw_Rect(&rect2);
		}
 };
 Score scores[2];
 
 
 
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
		 }
		 
		 void lookLeft() {
		 	if (!isJumping) {
		 		direction = 0;
		 		updateImage();
		 	}
		 }
		 
		 void lookRight() {
		 	if (!isJumping) {
		 		direction = 1;
		 		updateImage();
		 	}
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
				
 				std::vector<Fly>::iterator it = flies.begin();
 				while (it != flies.end()) {
 					if (it->isCaught(x, y)) {
 						it = flies.erase(it);
 						score++;
 						scores[player].updateScore(score);
 					} else {
 						it++;
 					}
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
 
 class TimerClock {
 	public:
 		Image *imgDigit1, *imgDigit2;
 		Rect rect, rect2;
 		void init(int duration) {	
 			imgDigit1 = &imgNumbers[duration / 10];
 			imgDigit2 = &imgNumbers[duration % 10];
 			rect.Left = (WIDTH - (6 * SCALE * 2 + 2)) / 2;
			rect.Right = rect.Left + 6 * SCALE;
			rect.Bottom = HEIGHT - (7 * SCALE + 10);
			rect.Top = rect.Bottom + 7 * SCALE;
			rect2.Left = rect.Right + 2;
			rect2.Right = rect2.Left + 6 * SCALE;
			rect2.Bottom = rect.Bottom;
			rect2.Top = rect.Top;
 		}
 		void updateTime(int time) {
			int digit1 = time / 10;
			int digit2 = time % 10;
			imgDigit1 = &imgNumbers[digit1];
 			imgDigit2 = &imgNumbers[digit2];
		}
 		void draw() {
 			Map_Texture(imgDigit1);
			Draw_Rect(&rect);
			Map_Texture(imgDigit2);
			Draw_Rect(&rect2);
 		}
 };
 TimerClock timerClock;
//====================================================// 
 
void loadBackground() {
	Load_Texture_Swap(&imgBackground, "img/Background.png");
	Zoom_Image(&imgBackground, SCALE);
	Load_Texture_Swap(&imgGround, "img/Ground.png");
	Zoom_Image(&imgGround, SCALE);
}

void loadAndInitTitle() {
	Load_Texture_Swap(&imgTitle, "img/Title.png");
	//Zoom_Image(&imgTitle, 2);
	Image *img = &imgTitle;
	title.Left = (WIDTH - img->w) / 2;
	title.Right = title.Left + img->w;
	title.Bottom = (HEIGHT - img->h) / 2;
	title.Top = title.Bottom + img->h;
}

void loadAndInitContinueKey() {
	Load_Texture_Swap(&imgContinueKey, "img/ContinueKey.png");
	Image *img = &imgContinueKey;
	continueKey.Left = (WIDTH - img->w) / 2 - 10;
	continueKey.Right = continueKey.Left + img->w;
	continueKey.Bottom = 5;
	continueKey.Top = continueKey.Bottom + img->h - 3;
}

void loadAndInitResults() {
	Load_Texture_Swap(&imgResults[0], "img/Result1.png");
	Load_Texture_Swap(&imgResults[1], "img/Result2.png");
	Load_Texture_Swap(&imgResults[2], "img/Result3.png");
	Image *img = &imgResults[0];
	result.Left = (WIDTH - img->w) / 2;
	result.Right = title.Left + img->w;
	result.Bottom = (HEIGHT - img->h) / 2;
	result.Top = title.Bottom + img->h;
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

void initNumbers() {
 	Image img;
 	Load_Texture_Swap(&img, "img/Numbers.png");
 	for (int i = 0; i < 10; i++) {
        Crop_Image(&img, &imgNumbers[i], i * 6, 0, 6, 7);
        Zoom_Image(&imgNumbers[i], SCALE);
   		}
   	Delete_Image(&img);
}

void showResult(int score1, int score2) {
	if (score1 > score2) {
		Map_Texture(&imgResults[0]);
		Draw_Rect(&result);
	} else 
		if (score1 < score2) {
			Map_Texture(&imgResults[1]);
			Draw_Rect(&result);
		} else {
			Map_Texture(&imgResults[2]);
			Draw_Rect(&result);	
		}
	Map_Texture(&imgContinueKey);
	Draw_Rect(&continueKey);
}

void initGame() {
	loadBackground();
	loadAndInitTitle();
	initMap();	
	initPlatformers();
	initClouds(); 
	initFrogs();
	Line::loadImage();
	Fly::loadImage();
	initNumbers();
	loadAndInitResults();
	loadAndInitContinueKey();
	scores[0].init(0);
	scores[1].init(1);
	timerClock.init(MATCH_DURATION);
	isMatchEnd = false;
	time = MATCH_DURATION;
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

void Display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	Map_Texture(&imgBackground);
	Draw_Rect(&rectBackground);
	Map_Texture(&imgGround);
	Draw_Rect(&rectGround);
	
	for (int i = 0; i < CLOUD_AMOUNT; i++) {
		clouds[i].draw();
	}
	
	if (!isPlaying && !isMatchEnd) {
		Map_Texture(&imgTitle);
		Draw_Rect(&title);	
	} else {
		timerClock.draw();
		if (!isMatchEnd) {
			for (Line line : lines) {
				line.draw();
			}
		}
		
		for (int i = 0; i < flies.size(); i++) {
			flies[i].draw();
		}
	
		for (int i = 0; i < PLATFORMER_AMOUNT; i++) {
			platformers[i].draw();
		}
	
		frogs[0].draw();
		frogs[1].draw();
	
		scores[0].draw();
		scores[1].draw();
	}
	if (isMatchEnd) {
		showResult(frogs[0].score, frogs[1].score);
	}	
	glutSwapBuffers();
}

void timer(int value) {
	for (int i = 0; i < CLOUD_AMOUNT; i++) {
		clouds[i].updatePositionX();
	}
	FlySpawner.update();
	for (int i = 0; i < flies.size(); i++) {
		flies[i].update();
	}

	lines.clear();
	frogs[0].update();
	frogs[1].update();
	
	if (counter < 66) {
		counter++;
	} else {
		counter = 0;
		time--;
		if (time >= 0) {
			timerClock.updateTime(time);	
		} else {
			isMatchEnd = true;
			isPlaying = false;
		}
	}
	
	glutPostRedisplay();
	glutTimerFunc(INTERVAL, timer, 0);
}

void keyboardDown(GLubyte key, int x, int y) {
	if (isPlaying) {
		switch (key) {
			case 32:
				frogs[0].keyDown();
				break;
			case 13:
				frogs[1].keyDown();
				break;
			case 97:
				frogs[0].lookLeft();
				break;
			case 100:
				frogs[0].lookRight();
				break;		
		}	
	} else {
		switch (key) {
			
		}
	}
}

void keyboardUp(GLubyte key, int x, int y) {
	if (isPlaying) {
		switch (key) {
			case 32:
				frogs[0].keyUp();
				break;	
			case 13:
				frogs[1].keyUp();
				break;
		}	
	} else {
		switch (key) {
			case 27:
				initGame();
				break;
			case 32:
				if (!isMatchEnd) {
					isPlaying = true;
				}
				break;
		}
	}
}
void specialKeyDown(int key, int x, int y) {
	if (isPlaying) {
		switch (key) {
			case GLUT_KEY_LEFT:
				frogs[1].lookLeft();
				break;
			case GLUT_KEY_RIGHT:
				frogs[1].lookRight();
				break;
		}
	}
}
void specialKeyUp(int key, int x, int y) {
		
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
	glutSpecialFunc(specialKeyDown);
	glutSpecialUpFunc(specialKeyUp);
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
