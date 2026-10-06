#pragma once

#include "Engine/GameObject.h"

class MovingPlatform :
	public GameObject
{
public:
	// コンストラクタ
	// 引数：parent 親オブジェクト
	MovingPlatform(GameObject* parent);

	// 初期化
	void Initialize() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// 解放
	void Release() override;

	// 前フレームからのX方向の移動量を取得
	float GetDeltaX() const;

	// プレイヤーが床の上にいるか判定
	bool IsPlayerOnPlatform(
		float playerX,
		float playerBottom) const;

	// 床の上面のY座標を取得
	float GetTopY() const;

private:
	// 床のモデルハンドル
	int hModel_;

	// 床の開始位置
	float startX_;

	// 床のY座標
	float platformY_;

	// 床の移動距離
	float moveDistance_;

	// 床の移動速度
	float moveSpeed_;

	// 移動方向
	float moveDirection_;

	// 前フレームのX座標
	float previousX_;

	// 床の横幅の半分
	float halfWidth_;

	// 床の高さの半分
	float halfHeight_;
};