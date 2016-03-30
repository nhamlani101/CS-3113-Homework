//Noman Hamlani
//N16943203
//CS3113
//Homework 5

#include "GameApp.h"
#define TILE_SIZE 2.5f

GameApp::GameApp() : done(false)
{
	Setup();
	tileID = LoadTexture("arne_sprites.png");
	buildPlayer();
	solids = {0, 1};
	buildLevel();
}

GameApp::~GameApp() { 
	SDL_Quit(); 
}

GLuint GameApp::LoadTexture(const char* image_path)
{
	SDL_Surface* surface = IMG_Load(image_path);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_FreeSurface(surface);

	return textureID;
}

void GameApp::buildPlayer()
{
	playerID = LoadTexture("characters_3.png");
	player = new Entity();
	player->x = 7.0;
	player->y = -17.0;
	player->height = 2 * TILE_SIZE;
	player->width = 1.5 * TILE_SIZE;
	player->sprite = new SheetSprite(playerID, player->width, player->height, 5.0);
	player->isStatic = false;
	player->acceleration_y = -15.0;
	player->friction = 8.0;
}

void GameApp::buildLevel()
{
	std::ifstream tileStream("newmap.txt");
	std::string line;
	while (getline(tileStream, line))
	{
		if (line == "[header]")
		{
			if (!readHeader(tileStream)) {
				return;
			}
		}
		else if (line == "[layer]")
		{
			readLayerData(tileStream);
		}
	}
	tileStream.close();
	for (int y = 0; y < mapHeight; y++)
	{
		for (int x = 0; x < mapWidth; x++)
		{
			if (isSolid((int)levelData[y][x]))
			{
				blocks.push_back(new Entity());
				blocks.back()->x = (TILE_SIZE * x) + (TILE_SIZE / 2.0);
				blocks.back()->y = (-TILE_SIZE * y) - (TILE_SIZE / 2.0);
				blocks.back()->height = TILE_SIZE;
				blocks.back()->width = TILE_SIZE;
				blocks.back()->isStatic = true;
			}
		}
	}
}

bool GameApp::readHeader(std::ifstream &stream)
{
	std::string line;
	mapWidth = -1;
	mapHeight = -1;
	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned char*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned char[mapWidth];
		}
		return true;
	}
}

bool GameApp::readLayerData(std::ifstream &stream) {
	std::string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				std::istringstream lineStream(line);
				std::string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned char val = (unsigned char)atoi(tile.c_str());
					std::cout << val << std::endl;
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

void GameApp::renderTiles()
{
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	int SPRITE_COUNT_X = 8;
	int SPRITE_COUNT_Y = 3;
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
			float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
			float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
			float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
			vertexData.insert(vertexData.end(), {
				TILE_SIZE * x, -TILE_SIZE * y,
				TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
				TILE_SIZE * x, -TILE_SIZE * y,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
			});
			texCoordData.insert(texCoordData.end(), {
				u, v,
				u, v + (spriteHeight),
				u + spriteWidth, v + (spriteHeight),
				u, v,
				u + spriteWidth, v + (spriteHeight),
				u + spriteWidth, v
			});
		}
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	modelMatrix.identity();
	//modelMatrix.Translate(-16.0, 9.0, 0.0);
	program->setModelMatrix(modelMatrix);
	glBindTexture(GL_TEXTURE_2D, tileID);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 32 * 18);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void GameApp::scroll()
{
	viewMatrix.identity();
		viewMatrix.Translate(-player->x, -player->y, 0.0);

	program->setViewMatrix(viewMatrix);
}

bool GameApp::isSolid(int c)
{
	for (int i = 0; i < solids.size(); i++)
	{
		if (solids[i] == c)
			return true;
	}
	return false;
}

void GameApp::Setup()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Noman Hamlani - HW5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);

	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	projectionMatrix.setOrthoProjection(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, 1.0f);

	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);
	glUseProgram(program->programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void GameApp::ProcessEvents(float elapsed)
{
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}
		const Uint8* keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_RIGHT])
		{
			player->acceleration_x = 12.5;
		}
		else if (keys[SDL_SCANCODE_LEFT])
		{
			player->acceleration_x = -12.5;
		}
		else
			player->acceleration_x = 0.0;
		if (keys[SDL_SCANCODE_SPACE])
		{
			if (player->collidedBottom)
				player->velocity_y = 12.5;
		}
	
}

void GameApp::Update(float elapsed)
{
	ProcessEvents(elapsed);
	player->Update(elapsed);
	playerCollisions();
	scroll();
}

void GameApp::Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	renderTiles();
	player->Render(program, modelMatrix);
	SDL_GL_SwapWindow(displayWindow);
}

bool GameApp::UpdateAndRender()
{
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	Update(elapsed);
	Render();
	return done;
}

void GameApp::playerCollisions()
{
	for (int i = 0; i < blocks.size(); i++)
	{
		player->collidesWith(blocks[i]);
	}
}