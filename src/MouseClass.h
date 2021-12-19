#pragma once
#include <Queue>
#include "MouseEvent.h"

class MouseClass
{
private:
	std::queue<MouseEvent> m_EventBuffer;
	bool m_IsLeftClick = false;
	bool m_IsRightClick = false;
	bool m_IsScrollClick = false;
	Vertex2 m_Position, m_Distance;
public:
	MouseClass() : m_Position(0,0), m_Distance(0,0) {}
	void OnLeftClick(int x, int y);
	void OnLeftRelease(int x, int y);
	void OnRightClick(int x, int y);
	void OnRightRelease(int x, int y);
	void OnScrollClick(int x, int y);
	void OnScrollRelease(int x, int y);
	void OnScrollUp(int x, int y);
	void OnScrollDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseMoveRaw(int x, int y);

	inline bool IsLeftClick() { return m_IsLeftClick; }
	inline bool IsRightClick() { return m_IsRightClick; }
	inline bool IsScrollClick() { return m_IsScrollClick; }

	inline int GetPosX() { return m_Position.x; }
	inline int GetPosY() { return m_Position.y; }
	inline Vertex2 GetPos() { return m_Position; }
	inline Vertex2 GetDis() { return m_Distance; }

	inline bool IsEventBufferEmpty() { return m_EventBuffer.empty(); }
	MouseEvent ReadEvent();
};

