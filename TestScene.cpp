#define _CRT_SECURE_NO_WARNINGS

#include "TestScene.h"
#include "Player.h"
#include "Ground.h"
#include "Engine/Camera.h"
#include "Engine/Text.h"
#include "Engine/Input.h"

namespace
{
	Ground* pGround = nullptr;

	const float CAMERA_HEIGHT = 8.0f;

	const XMFLOAT3 START_POS =
	{
		15.0f,
		0.75f,
		0.5f
	};

	const float END_POS_X = 43.0f;
}

TestScene::TestScene(GameObject* parent)
	: GameObject(parent, "TestScene"),
	myScore(0),
	foodCount(0),
	isClear(false),
	pPlayer_(nullptr),
	pText_(nullptr)
{
}

void TestScene::Initialize()
{
	pText_ = new Text;
	pText_->Initialize();
}

void TestScene::StartGame()
{
	myScore = 0;
	foodCount = 0;
	isClear = false;

	pPlayer_ = Instantiate<Player>(this);
	pGround = Instantiate<Ground>(this);

	pPlayer_->SetGround(pGround);

	Camera::SetPosition(
		{
			pPlayer_->GetPosition().x,
			pPlayer_->GetPosition().y + CAMERA_HEIGHT,
			-22.0f
		});

	Camera::SetTarget(
		{
			pPlayer_->GetPosition().x,
			pPlayer_->GetPosition().y + CAMERA_HEIGHT,
			0.0f
		});
}

void TestScene::Update()
{
	//タイトル画面
	if (pPlayer_ == nullptr && !isClear)
	{
		if (Input::IsKeyDown(DIK_RETURN))
		{
			StartGame();
		}

		return;
	}

	//クリア画面
	if (isClear)
	{
		if (Input::IsKeyDown(DIK_RETURN))
		{
			myScore = 0;
			foodCount = 0;
			isClear = false;
		}

		return;
	}

	//ゲーム中
	if (pPlayer_ != nullptr)
	{
		float playerX =
			pPlayer_->GetPosition().x;

		//プレイヤーを中心にカメラを横スクロールさせる
		if (playerX > START_POS.x &&
			playerX < END_POS_X)
		{
			Camera::SetPosition(
				{
					playerX,
					START_POS.y + CAMERA_HEIGHT,
					-22.0f
				});

			Camera::SetTarget(
				{
					playerX,
					START_POS.y + CAMERA_HEIGHT,
					0.0f
				});
		}

		//ゴール到達
		if (playerX >= END_POS_X)
		{
			isClear = true;
		}
	}
}

void TestScene::Draw()
{
	//タイトル画面
	if (pPlayer_ == nullptr && !isClear)
	{
		//メインタイトル
		pText_->Draw(
			500,
			180,
			"OJISAN RUN");

		//サブタイトル
		pText_->Draw(
			515,
			250,
			"THE OJISAN IS GOING TO WORK");

		//ゲームの目的
		pText_->Draw(
			480,
			340,
			"GET THE OJISAN TO THE GOAL!");

		//操作説明
		pText_->Draw(
			500,
			410,
			"LEFT / RIGHT : MOVE");

		pText_->Draw(
			500,
			450,
			"SPACE : DOUBLE JUMP");

		pText_->Draw(
			500,
			490,
			"SHIFT : DASH");

		//スタート
		pText_->Draw(
			535,
			570,
			"PRESS ENTER TO START");

		return;
	}

	//クリア画面
	if (isClear)
	{
		pText_->Draw(
			570,
			250,
			"GOAL!");

		pText_->Draw(
			480,
			330,
			"THE OJISAN MADE IT!");

		pText_->Draw(
			500,
			430,
			"PRESS ENTER TO TITLE");

		return;
	}
}

void TestScene::Release()
{
	if (pText_ != nullptr)
	{
		pText_->Release();

		delete pText_;

		pText_ = nullptr;
	}
}