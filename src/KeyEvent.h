#pragma once

class KeyEvent
{
public:
	enum EventType
	{
		PRESS,
		RELEASE,
		INVALID
	};

	KeyEvent();
	KeyEvent(const EventType type, const unsigned char key);

	bool IsPressed() const;
	bool IsReleased() const;
	bool IsValid() const;
	unsigned char GetKeyCode() const; 

private:
	EventType m_Type;
	unsigned char m_Key;
};

