#include "MovingPlatform.h"
#include "Engine/Model.h"

namespace
{
	// 動く床の初期位置
	// Playerが通常ジャンプでも届く高さに設定する
	const float PLATFORM_START_X = 20.0f;
	const float PLATFORM_START_Y = 2.0f;
	const float PLATFORM_START_Z = 0.0f;

	// 左右への移動距離
	const float PLATFORM_MOVE_DISTANCE = 5.0f;

	// 移動速度
	const float PLATFORM_MOVE_SPEED = 0.05f;

	// 床の横幅
	const float PLATFORM_HALF_WIDTH = 2.0f;

	// 床の高さ
	const float PLATFORM_HALF_HEIGHT = 0.375f;
}

MovingPlatform::MovingPlatform(GameObject* parent)
	: GameObject(parent, "MovingPlatform"),
	hModel_(-1),
	startX_(PLATFORM_START_X),
	platformY_(PLATFORM_START_Y),
	moveDistance_(PLATFORM_MOVE_DISTANCE),
	moveSpeed_(PLATFORM_MOVE_SPEED),
	moveDirection_(1.0f),
	previousX_(PLATFORM_START_X),
	halfWidth_(PLATFORM_HALF_WIDTH),
	halfHeight_(PLATFORM_HALF_HEIGHT)
{
}

void MovingPlatform::Initialize()
{
	// 既存のBrickGモデルを動く床として使用
	hModel_ = Model::Load("BrickG.fbx");

	// 床の初期位置
	transform_.position_ =
	{
		PLATFORM_START_X,
		PLATFORM_START_Y,
		PLATFORM_START_Z
	};

	// 横長の床にする
	transform_.scale_ =
	{
		2.0f,
		0.5f,
		1.0f
	};

	// 前フレームの位置を初期位置にする
	previousX_ =
		transform_.position_.x;
}

void MovingPlatform::Update()
{
	// 更新前のX座標を保存
	previousX_ =
		transform_.position_.x;

	// X方向へ移動
	transform_.position_.x +=
		moveSpeed_ * moveDirection_;

	// 右端まで到達したら左へ移動
	if (transform_.position_.x >=
		startX_ + moveDistance_)
	{
		transform_.position_.x =
			startX_ + moveDistance_;

		moveDirection_ = -1.0f;
	}

	// 左端まで到達したら右へ移動
	if (transform_.position_.x <=
		startX_ - moveDistance_)
	{
		transform_.position_.x =
			startX_ - moveDistance_;

		moveDirection_ = 1.0f;
	}
}

bool MovingPlatform::IsPlayerOnPlatform(
	float playerX,
	float playerBottom) const
{
	float platformLeft =
		transform_.position_.x -
		halfWidth_;

	float platformRight =
		transform_.position_.x +
		halfWidth_;

	float platformTop =
		GetTopY();

	// プレイヤーの左右が床と重なっているか
	bool horizontalHit =
		playerX + 0.45f > platformLeft &&
		playerX - 0.45f < platformRight;

	// プレイヤーの足元が床の上面付近にあるか
	// 通常の着地判定より少し広めに設定する
	bool verticalHit =
		playerBottom >=
		platformTop - 0.35f &&
		playerBottom <=
		platformTop + 0.15f;

	return horizontalHit && verticalHit;
}

float MovingPlatform::GetTopY() const
{
	// 床の上面のY座標を返す
	return transform_.position_.y +
		halfHeight_;
}

float MovingPlatform::GetDeltaX() const
{
	// 前フレームから現在フレームまでの
	// X方向の移動量を返す
	return transform_.position_.x -
		previousX_;
}

void MovingPlatform::Draw()
{
	// モデルへTransformを設定
	Model::SetTransform(
		hModel_,
		transform_);

	// モデルを描画
	Model::Draw(hModel_);
}

void MovingPlatform::Release()
{
	// モデルを解放
	if (hModel_ != -1)
	{
		Model::Release(hModel_);

		hModel_ = -1;
	}
}