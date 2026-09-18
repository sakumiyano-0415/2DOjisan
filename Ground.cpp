#include "Ground.h"
#include "Food.h"
#include "Engine/Model.h"
#include "Engine/CsvReader.h"

namespace
{
	const float GROUND_WIDTH = 20.0f;
	const float GROUND_Y = 10.0f;
	const float GROUND_Z = 1.0f;

	const float BLOCK_INTERVAL_X = 2.0f;
	const float BLOCK_INTERVAL_Y = 1.0f;
}

Ground::Ground(GameObject* parent)
	: GameObject(parent, "Ground"),
	hModel_(-1),
	hModelt_(-1),
	mapWidth_(0),
	mapHeight_(0)
{
	CsvReader csvData;

	if (!csvData.Load("map.csv"))
		return;

	if (csvData.GetHeight() == 0 ||
		csvData.GetWidth() == 0)
		return;

	mapWidth_ =
		static_cast<int>(csvData.GetWidth());

	mapHeight_ =
		static_cast<int>(csvData.GetHeight()) / 2;

	if (mapHeight_ <= 0)
	{
		mapWidth_ = 0;
		mapHeight_ = 0;
		return;
	}

	mapData_.resize(
		mapHeight_,
		std::vector<int>(mapWidth_, 0));

	for (int y = 0; y < mapHeight_; y++)
	{
		for (int x = 0; x < mapWidth_; x++)
		{
			mapData_[y][x] =
				csvData.GetValue(x, y);
		}
	}
}

void Ground::Initialize()
{
	hModel_ = Model::Load("jimen3.fbx");
	hModelt_ = Model::Load("BrickG.fbx");

	for (int y = 0; y < mapHeight_; y++)
	{
		for (int x = 0; x < mapWidth_; x++)
		{
			float posX = x * BLOCK_INTERVAL_X;
			float posY =
				(mapHeight_ - 1 - y) *
				BLOCK_INTERVAL_Y;

			if (mapData_[y][x] == 2)
			{
				Food* food =
					Instantiate<Food>(GetParent());

				food->SetFoodType(
					FoodType::FOODTYPE_NORMAL);

				food->SetPosition(
					posX,
					posY + 0.8f,
					0.0f);
			}
			else if (mapData_[y][x] == 3)
			{
				Food* food =
					Instantiate<Food>(GetParent());

				food->SetFoodType(
					FoodType::FOODTYPE_POWER);

				food->SetPosition(
					posX,
					posY + 1.0f,
					0.0f);
			}
		}
	}
}

void Ground::Update()
{
}

void Ground::Draw()
{
	for (int i = 0; i < 3; i++)
	{
		transform_.position_ =
		{
			GROUND_WIDTH * 0.5f +
			GROUND_WIDTH * i,
			GROUND_Y,
			GROUND_Z
		};

		transform_.rotate_ =
		{
			-90.0f,
			0.0f,
			0.0f
		};

		Model::SetTransform(
			hModel_,
			transform_);

		Model::Draw(hModel_);
	}

	for (int y = 0; y < mapHeight_; y++)
	{
		for (int x = 0; x < mapWidth_; x++)
		{
			if (mapData_[y][x] != 1)
				continue;

			Transform tr;

			tr.position_ =
			{
				x * BLOCK_INTERVAL_X,
				(mapHeight_ - 1 - y) *
				BLOCK_INTERVAL_Y,
				0.0f
			};

			Model::SetTransform(
				hModelt_,
				tr);

			Model::Draw(hModelt_);
		}
	}
}

void Ground::Release()
{
}