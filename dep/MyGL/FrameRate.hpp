#pragma once
#define LIMIT_FPS 10
namespace MyGL
{
	class FrameRateManager
	{
	private:
		float last{}, delta{}, i = 0, delta_plus = 0.0001f;
	public:
		void UpdateFrameRateInfo();
		float GetFps();
		float GetMovementDistance(float originDis) const;
	};
}
