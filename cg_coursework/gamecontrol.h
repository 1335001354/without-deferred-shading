#pragma once
#include "Camera.h"
//void HandleMouseInput(HWND hwnd, Camera& camera, float dt, float& lastX, float& lastY) {
//	POINT mousePos;
//	RECT rect;
//	GetClientRect(hwnd, &rect);
//	if (GetCursorPos(&mousePos)) {//GetCursorPos retrieves the mouse's current position in screen coordinates.
//		ScreenToClient(hwnd, &mousePos);//converts these screen coordinate to client-area coordinates
//
//		float deltaX = static_cast<float>(mousePos.x - lastX);
//		float deltaY = static_cast<float>(mousePos.y - lastY);
//
//		float sensitivity = 0.005f; // Adjust sensitivity to taste
//		camera.Update(dt, deltaY * sensitivity, deltaX * sensitivity);
//	}
//	POINT center = { rect.right / 2, rect.bottom / 2 };
//	ClientToScreen(hwnd, &center);
//	SetCursorPos(center.x, center.y);
//	lastX = static_cast<float>(mousePos.x);
//	lastY = static_cast<float>(mousePos.y);
//}

//void Render(Shaders& shader, GEMModel& model, DXcore& dx) {
//	shader.apply(dx);
//	shader.DrawModel(model, dx);
//}

struct gm
{
	float dt;
	float lastX = 0.f, lastY = 0.f;
};