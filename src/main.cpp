#include "raylib.h"
#include <raymath.h>
#include <iostream>
#include <deque>

Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDequeue(Vector2& element, std::deque<Vector2>& deque)
{
	for (auto body_part : deque)
	{
		if (Vector2Equals(body_part, element))
		{
			return true;
		}
	}
	return false;
}

bool eventTriggered(double interval)
{
	double currentTime = GetTime();
	if (currentTime - lastUpdateTime >= interval)
	{
		lastUpdateTime = currentTime;
		return true;
	}

	return false; 
}

class Snake
{
public:
	void Draw()
	{
		for (auto body_part : body)
		{
			float x = body_part.x;
			float y = body_part.y;
			Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, static_cast<float>(cellSize), static_cast<float>(cellSize)};
			DrawRectangleRounded(segment, 0.5, 6, darkGreen);
		}
	}

	void Update()
	{
		body.push_front(Vector2Add(body[0], direction));
		if (addSegment == true)
		{
			addSegment = false;
		}
		else
		{
			body.pop_back();
		}
	}

	void UpdateDirection(Vector2 new_direction)
	{
		direction = new_direction;
	}

	Vector2 GetDirection()
	{
		return direction;
	}

	std::deque<Vector2>& GetBody()
	{
		return body;
	}

	bool& GetAddSegment()
	{
		return addSegment;
	}

	void Reset()
	{
		body = {Vector2{6, 9}, Vector2{5,9}, Vector2{4, 9}};
		direction = {1, 0};
	}

private:
	std::deque<Vector2> body = {Vector2{6, 9}, Vector2{5,9}, Vector2{4, 9}};
	Vector2 direction = {1, 0};
	bool addSegment = false;
};

class Food
{
public:
	Food(std::deque<Vector2>& snakePosition)
	{
		Image image = LoadImage("Graphics/banana.png");
		texture = LoadTextureFromImage(image);
		UnloadImage(image);
		position = GenerateRandomPosition(snakePosition);
	}

	~Food()
	{
		//UnloadTexture(texture);
	}

	void Draw()
	{
		DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
	};

	Vector2 GenerateRandomCell()
	{
		float x = GetRandomValue(0, cellCount - 1);
		float y = GetRandomValue(0, cellCount - 1);
		return Vector2{x, y};
	}

	Vector2 GenerateRandomPosition(std::deque<Vector2>& snake_loaction)
	{
		Vector2 new_position = GenerateRandomCell();
		while (ElementInDequeue(new_position, snake_loaction))
		{
			new_position = GenerateRandomCell();
		}

		return new_position;
	}

	Vector2& GetPosition()
	{
		return position;
	}

	Texture2D& GetTexture()
	{
		return texture;
	}

private:
	Vector2 position;
	Texture2D texture;
};

class Game
{
public:
	Game() : snake(Snake()), food(Food(snake.GetBody())) 
	{
		InitAudioDevice();
		eatSound = LoadSound("Sounds/eatfood.mp3");
		gameOverSound = LoadSound("Sounds/gameover.mp3");
		soundtrack = LoadSound("Sounds/soundtrack.mp3");

		PlaySound(soundtrack);
	}

	~Game()
	{
		UnloadSound(eatSound);
		UnloadSound(gameOverSound);
		CloseAudioDevice();
	}

	void Draw()
	{
		food.Draw();
		snake.Draw();
	}

	void Update()
	{
		if (running)
		{
			snake.Update();
			CheckCollisionWithFood();
			CheckCollisionWithEdges();
			CheckCollisionWithTail();
			if (!IsSoundPlaying(soundtrack))
			{
				PlaySound(soundtrack);
			}
		}
	}

	Snake& GetSnake()
	{
		return snake;
	}

	Food& GetFood()
	{
		return food;
	}

	void CheckCollisionWithFood()
	{
		if (Vector2Equals(snake.GetBody()[0], food.GetPosition()))
		{
			food.GetPosition() = food.GenerateRandomPosition(snake.GetBody());
			snake.GetAddSegment() = true;
			++score;
			PlaySound(eatSound);
		}
	}

	void CheckCollisionWithEdges()
	{
		if (snake.GetBody()[0].x == cellCount || snake.GetBody()[0].x == -1)
		{
			GameOver();
		}
		if (snake.GetBody()[0].y == cellCount || snake.GetBody()[0].y == -1)
		{
			GameOver();
		}
	}

	void CheckCollisionWithTail()
	{
		std::deque<Vector2> headlessBody = snake.GetBody();
		headlessBody.pop_front();
		if (ElementInDequeue(snake.GetBody()[0], headlessBody))
		{
			GameOver();
		}
	}

	void GameOver()
	{
		snake.Reset();
		food.GetPosition() = food.GenerateRandomPosition(snake.GetBody());
		running = false;
		score = 0;
		PlaySound(gameOverSound);
	}

	bool& GetRunning()
	{
		return running;
	}

	int GetScore()
	{
		return score;
	}

private:
	Snake snake;
	Food food;
	bool running = true;
	int score = 0;
	Sound eatSound;
	Sound gameOverSound;
	Sound soundtrack;
};

int main ()
{
	std::cout << "Starting the game..." << std::endl;
	InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
	SetTargetFPS(60);

	Game game;

	while(WindowShouldClose() == false)
	{
		BeginDrawing();

		if (eventTriggered(0.2))
		{
			game.Update();
		}

		if(IsKeyPressed(KEY_UP) && game.GetSnake().GetDirection().y != 1)
		{
			game.GetSnake().UpdateDirection(Vector2{0, -1});
			game.GetRunning() = true;
		}
		if(IsKeyPressed(KEY_DOWN) && game.GetSnake().GetDirection().y != -1)
		{
			game.GetSnake().UpdateDirection(Vector2{0, 1});
			game.GetRunning() = true;
		}
		if(IsKeyPressed(KEY_RIGHT) && game.GetSnake().GetDirection().x != -1)
		{
			game.GetSnake().UpdateDirection(Vector2{1, 0});
			game.GetRunning() = true;
		}
		if(IsKeyPressed(KEY_LEFT) && game.GetSnake().GetDirection().x != 1)
		{
			game.GetSnake().UpdateDirection(Vector2{-1, 0});
			game.GetRunning() = true;
		}
		
		ClearBackground(green);
		DrawRectangleLinesEx(Rectangle{static_cast<float>(offset - 5), 
									   static_cast<float>(offset - 5), 
									   static_cast<float>(cellSize * cellCount + 10),
									   static_cast<float>(cellSize * cellCount + 10)}, 
									   5, 
									   darkGreen);
		DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
		DrawText(TextFormat("Score: %i", game.GetScore()), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
		game.Draw();

		EndDrawing();
	}

	CloseWindow();

	return 0;
}