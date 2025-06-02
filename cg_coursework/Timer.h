#pragma once
#include <chrono>
class TIMER {
private:
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point lastTime;
public:
	TIMER() {
		reset();
	}
	void reset() {
		startTime = std::chrono::high_resolution_clock::now();
		lastTime = startTime;
	}
	float getDeltaTime() {
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = currentTime - lastTime;
		lastTime = currentTime;
		return elapsed.count();
	}
};