#include "Player.h"
#include "Engine/Model.h"
#include "Engine/Input.h"
#include "Ground.h"
#include <vector>

namespace
{
	//通常移動
	const float MAX_SPEED = 0.18f;
	const float BASE_SPEED = 0.10f;
	const float ACCELERATION = 0.006f;
	const float FRICTION = 0.010f;
	const float AIR_CONTROL = 0.55f;

	//ダッシュ
	const float DASH_SPEED = 0.55f;
	const float DASH_FRAME = 8.0f;
	const float DASH_COOLDOWN = 45.0f;

	//ジャンプ
	const float JUMP_POWER = 0.23f;
	const float DOUBLE_JUMP_POWER = 0.20f;
	const float GRAVITY = 0.012f;

	//ステージ
	const float BLOCK_SIZE = 2.0f;
	const float BLOCK_INTERVAL_Y = 1.0f;
	const float BLOCK_HEIGHT = 1.5f;

	//プレイヤー
	const float START_Y = 0.75f;
	const XMFLOAT3 START_POS = { 15.0f, START_Y, 0.5f };

	const float PLAYER_HALF_WIDTH = 0.45f;
	const float PLAYER_HALF_HEIGHT = 0.5f;

	enum PLAYER_STATE
	{
		PLAYER_IDLE,
		PLAYER_WALK,
		PLAYER_DASH
	};

	enum PLAYER_DIRECTION
	{
		PLAYER_LEFT,
		PLAYER_RIGHT
	};

	PLAYER_STATE pstate = PLAYER_IDLE;
	PLAYER_DIRECTION pdirection = PLAYER_RIGHT;

	float currentSpeed = 0.0f;

	//ジャンプ関連
	float jumpVelocity = 0.0f;
	bool isGrounded = true;
	bool hasDoubleJumped = false;

	//ダッシュ関連
	bool isDashing = false;
	float dashFrame = 0.0f;
	float dashCooldown = 0.0f;

	float ClampFloat(
		float value,
		float minValue,
		float maxValue)
	{
		if (value < minValue)
			return minValue;

		if (value > maxValue)
			return maxValue;

		return value;
	}
}

Player::Player(GameObject* parent)
	: GameObject(parent, "Player"),
	hWalkModel_(-1),
	hIdleModel_(-1),
	ground_(nullptr)
{
}

void Player::Initialize()
{
	hWalkModel_ = Model::Load("Walking.fbx");
	Model::SetAnimFrame(
		hWalkModel_,
		0,
		59,
		1.0f);

	hIdleModel_ = Model::Load("Idle.fbx");
	Model::SetAnimFrame(
		hIdleModel_,
		0,
		117,
		1.0f);

	transform_.position_ = START_POS;

	//Z方向には絶対に移動しない
	transform_.position_.z = START_POS.z;

	SphereCollider* collision =
		new SphereCollider(
			XMFLOAT3(0.0f, 0.25f, 0.0f),
			0.5f);

	AddCollider(collision);

	currentSpeed = 0.0f;
	jumpVelocity = 0.0f;

	isGrounded = true;
	hasDoubleJumped = false;

	isDashing = false;
	dashFrame = 0.0f;
	dashCooldown = 0.0f;

	pstate = PLAYER_IDLE;
	pdirection = PLAYER_RIGHT;
}

void Player::Update()
{
	//ダッシュのクールタイムを減らす
	if (dashCooldown > 0.0f)
	{
		dashCooldown -= 1.0f;

		if (dashCooldown < 0.0f)
			dashCooldown = 0.0f;
	}

	//ダッシュ中
	if (isDashing)
	{
		dashFrame -= 1.0f;

		transform_.position_.x +=
			(pdirection == PLAYER_RIGHT)
			? DASH_SPEED
			: -DASH_SPEED;

		if (dashFrame <= 0.0f)
		{
			isDashing = false;
			pstate = PLAYER_WALK;

			//ダッシュ終了後に少し勢いを残す
			currentSpeed = MAX_SPEED;
		}

		//Z方向は常に固定
		transform_.position_.z = START_POS.z;

		ResolveBlockCollision();

		return;
	}

	bool isMoving = false;

	//左右移動
	if (Input::IsKey(DIK_LEFT))
	{
		pdirection = PLAYER_LEFT;
		isMoving = true;
	}
	else if (Input::IsKey(DIK_RIGHT))
	{
		pdirection = PLAYER_RIGHT;
		isMoving = true;
	}

	//通常移動
	if (isMoving)
	{
		float acceleration = ACCELERATION;

		//空中では操作能力を少し下げる
		if (!isGrounded)
			acceleration *= AIR_CONTROL;

		currentSpeed += acceleration;

		if (currentSpeed > MAX_SPEED)
			currentSpeed = MAX_SPEED;

		pstate = PLAYER_WALK;
	}
	else
	{
		float friction = FRICTION;

		if (!isGrounded)
			friction *= AIR_CONTROL;

		currentSpeed -= friction;

		if (currentSpeed < 0.0f)
			currentSpeed = 0.0f;

		if (currentSpeed == 0.0f)
			pstate = PLAYER_IDLE;
	}

	//左右方向へ移動
	if (pdirection == PLAYER_RIGHT)
	{
		transform_.position_.x += currentSpeed;
	}
	else
	{
		transform_.position_.x -= currentSpeed;
	}

	//おじさんの向きを変更
	if (pdirection == PLAYER_RIGHT)
	{
		transform_.rotate_.y = 270.0f;
	}
	else
	{
		transform_.rotate_.y = 90.0f;
	}

	//ジャンプ処理
	if (Input::IsKeyDown(DIK_SPACE))
	{
		//地上ジャンプ
		if (isGrounded)
		{
			jumpVelocity = JUMP_POWER;
			isGrounded = false;
			hasDoubleJumped = false;
		}
		//空中2段ジャンプ
		else if (!hasDoubleJumped)
		{
			jumpVelocity = DOUBLE_JUMP_POWER;
			hasDoubleJumped = true;
		}
	}

	//空中ダッシュ
	if (Input::IsKeyDown(DIK_LSHIFT))
	{
		if (dashCooldown <= 0.0f)
		{
			isDashing = true;
			dashFrame = DASH_FRAME;
			dashCooldown = DASH_COOLDOWN;

			pstate = PLAYER_DASH;

			//ダッシュ開始時に通常速度を無視する
			currentSpeed = DASH_SPEED;
		}
	}

	//ジャンプ・重力
	UpdateJump();

	//ブロックとの衝突
	ResolveBlockCollision();

	//横方向の壁衝突
	XMVECTOR pos =
		XMLoadFloat3(
			&transform_.position_);

	XMVECTOR move =
		XMVectorSet(
			(pdirection == PLAYER_RIGHT)
			? 1.0f
			: -1.0f,
			0.0f,
			0.0f,
			0.0f);

	ResolveWallCollision(
		pos,
		move);

	//Z方向を完全固定
	transform_.position_.z = START_POS.z;

	//歩行アニメーション速度
	if (currentSpeed > 0.0f)
	{
		Model::SetAnimSpeed(
			hWalkModel_,
			currentSpeed / BASE_SPEED);
	}
	else
	{
		Model::SetAnimSpeed(
			hWalkModel_,
			0.0f);
	}
}

void Player::UpdateJump()
{
	if (isGrounded)
		return;

	transform_.position_.y += jumpVelocity;

	jumpVelocity -= GRAVITY;

	//最低位置を超えないようにする
	if (transform_.position_.y <= START_Y)
	{
		transform_.position_.y = START_Y;

		jumpVelocity = 0.0f;
		isGrounded = true;
		hasDoubleJumped = false;
	}
}

void Player::ResolveBlockCollision()
{
	if (ground_ == nullptr)
		return;

	const std::vector<std::vector<int>>& map =
		ground_->GetMapData();

	if (map.empty() || map[0].empty())
		return;

	int mapHeight =
		static_cast<int>(map.size());

	int mapWidth =
		static_cast<int>(map[0].size());

	float playerX =
		transform_.position_.x;

	float playerBottom =
		transform_.position_.y -
		PLAYER_HALF_HEIGHT;

	int minX =
		static_cast<int>(
			(playerX - PLAYER_HALF_WIDTH +
				BLOCK_SIZE * 0.5f) /
			BLOCK_SIZE);

	int maxX =
		static_cast<int>(
			(playerX + PLAYER_HALF_WIDTH +
				BLOCK_SIZE * 0.5f) /
			BLOCK_SIZE);

	if (minX < 0)
		minX = 0;

	if (maxX >= mapWidth)
		maxX = mapWidth - 1;

	float landingTop = -999.0f;
	bool foundBlock = false;

	for (int x = minX; x <= maxX; x++)
	{
		for (int y = 0; y < mapHeight; y++)
		{
			if (x >= static_cast<int>(map[y].size()))
				continue;

			if (map[y][x] != 1)
				continue;

			float blockCenterY =
				(mapHeight - 1 - y) *
				BLOCK_INTERVAL_Y;

			float blockTop =
				blockCenterY +
				BLOCK_HEIGHT * 0.5f;

			//落下中に足場へ着地
			if (jumpVelocity <= 0.0f &&
				playerBottom >= blockTop - 0.10f &&
				playerBottom <= blockTop + 0.15f)
			{
				if (!foundBlock ||
					blockTop > landingTop)
				{
					landingTop = blockTop;
					foundBlock = true;
				}
			}
		}
	}

	if (foundBlock)
	{
		transform_.position_.y =
			landingTop +
			PLAYER_HALF_HEIGHT;

		jumpVelocity = 0.0f;
		isGrounded = true;
		hasDoubleJumped = false;
	}
	else if (transform_.position_.y > START_Y)
	{
		isGrounded = false;
	}
}

void Player::ResolveWallCollision(
	XMVECTOR& pos,
	const XMVECTOR& move)
{
	if (ground_ == nullptr)
		return;

	const std::vector<std::vector<int>>& map =
		ground_->GetMapData();

	if (map.empty() || map[0].empty())
		return;

	int mapHeight =
		static_cast<int>(map.size());

	float currentX =
		transform_.position_.x;

	float moveX =
		XMVectorGetX(move);

	float previousX =
		currentX -
		currentSpeed * moveX;

	float playerLeft =
		currentX -
		PLAYER_HALF_WIDTH;

	float playerRight =
		currentX +
		PLAYER_HALF_WIDTH;

	float playerBottom =
		transform_.position_.y -
		PLAYER_HALF_HEIGHT;

	float playerTop =
		transform_.position_.y +
		PLAYER_HALF_HEIGHT;

	for (int y = 0; y < mapHeight; y++)
	{
		for (int x = 0;
			x < static_cast<int>(map[y].size());
			x++)
		{
			if (map[y][x] != 1)
				continue;

			float blockCenterY =
				(mapHeight - 1 - y) *
				BLOCK_INTERVAL_Y;

			//床は壁として扱わない
			if (blockCenterY <= 0.0f)
				continue;

			float blockCenterX =
				x * BLOCK_SIZE;

			float blockLeft =
				blockCenterX -
				BLOCK_SIZE * 0.5f;

			float blockRight =
				blockCenterX +
				BLOCK_SIZE * 0.5f;

			float blockBottom =
				blockCenterY -
				BLOCK_HEIGHT * 0.5f;

			float blockTop =
				blockCenterY +
				BLOCK_HEIGHT * 0.5f;

			bool verticalHit =
				playerBottom < blockTop &&
				playerTop > blockBottom;

			bool horizontalHit =
				playerRight > blockLeft &&
				playerLeft < blockRight;

			if (verticalHit && horizontalHit)
			{
				transform_.position_.x =
					previousX;

				pos =
					XMLoadFloat3(
						&transform_.position_);

				currentSpeed = 0.0f;

				return;
			}
		}
	}
}

void Player::Draw()
{
	if (pstate == PLAYER_IDLE)
	{
		Model::SetTransform(
			hIdleModel_,
			transform_);

		Model::Draw(hIdleModel_);
	}
	else
	{
		Model::SetTransform(
			hWalkModel_,
			transform_);

		Model::Draw(hWalkModel_);
	}
}

void Player::Release()
{
}

void Player::OnCollision(GameObject* pTarget)
{
	//現在の2Dアクションでは
	//食べ物との衝突処理は使用しない
}