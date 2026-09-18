#pragma once
#include "Engine/GameObject.h"
#include "Engine/Model.h"

class Player;
class Text;

class TestScene : public GameObject
{
public:
	TestScene(GameObject* parent);

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Release() override;

	void AddScore(int score)
	{
		myScore += score;
	}

	void AddFoodCount()
	{
		foodCount++;

		if (foodCount >= 2)
		{
			isClear = true;
		}
	}

private:
	void StartGame();

	Text* pText_;

	int myScore;
	int foodCount;
	bool isClear;

	Player* pPlayer_;
};