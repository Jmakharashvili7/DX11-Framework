#include "MouseClass.h"

void MouseClass::OnLeftClick(int x, int y)
{
	m_IsLeftClick = true;
	MouseEvent me(MouseEvent::EventType::L_CLICK, x, y);
	m_EventBuffer.push(me);
}

void MouseClass::OnLeftRelease(int x, int y)
{
	m_IsLeftClick = false;
	m_EventBuffer.push(MouseEvent(MouseEvent::EventType::L_RELEASE, x, y));
}

void MouseClass::OnRightClick(int x, int y)
{
	m_IsRightClick = true;
	MouseEvent me(MouseEvent::EventType::R_CLICK, x, y);
	m_EventBuffer.push(me);
}

void MouseClass::OnRightRelease(int x, int y)
{
	m_IsRightClick = false;
	m_EventBuffer.push(MouseEvent(MouseEvent::EventType::R_RELEASE, x, y));
}

void MouseClass::OnScrollClick(int x, int y)
{
	m_IsScrollClick = true;
	MouseEvent me(MouseEvent::EventType::SCROLL_CLICK, x, y);
	m_EventBuffer.push(me);
}

void MouseClass::OnScrollRelease(int x, int y)
{
	m_IsLeftClick = true;
	m_EventBuffer.push(MouseEvent(MouseEvent::EventType::SCROLL_RELEASE, x, y));
}

void MouseClass::OnScrollUp(int x, int y)
{
	m_EventBuffer.push(MouseEvent(MouseEvent::EventType::SCROLL_UP, x, y));
}

void MouseClass::OnScrollDown(int x, int y)
{
	m_EventBuffer.push(MouseEvent(MouseEvent::EventType::SCROLL_DOWN, x, y));
}

void MouseClass::OnMouseMove(int x, int y)
{
	m_Distance.x = x - m_Position.x;
	m_Distance.y = y - m_Position.y;

	m_Position.x = x;
	m_Position.y = y;
	m_EventBuffer.push(MouseEvent(MouseEvent::EventType::MOVE, m_Position, m_Distance));
}

void MouseClass::OnMouseMoveRaw(int x, int y)
{
	m_EventBuffer.push(MouseEvent(MouseEvent::EventType::MOVE_RAW, x, y));
}

MouseEvent MouseClass::ReadEvent()
{
	if(m_EventBuffer.empty())
	{
		return MouseEvent();
	}
	else
	{
		MouseEvent e  = m_EventBuffer.front(); // Get the first event in the queue
		m_EventBuffer.pop(); // remove the first event from the buffer
		return e;
	}
}