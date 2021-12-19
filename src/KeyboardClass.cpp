#include "KeyboardClass.h"

KeyboardClass::KeyboardClass()
{
}

KeyEvent KeyboardClass::ReadKey()
{
	if (m_KeyBuffer.empty())
	{
		// return empty key event if no event is in the queue
		return KeyEvent();
	}
	else
	{
		// retreive the first event and remove it from the queue, after this return the event.
		KeyEvent e = m_KeyBuffer.front(); 
		m_KeyBuffer.pop(); 
		return e;
	}
}

unsigned char KeyboardClass::ReadChar()
{
	if (m_CharBuffer.empty())
	{
		// return empty key event if no event is in the queue
		return 0u;
	}
	else
	{
		// retreive the first event and remove it from the queue, after this return the event.
		unsigned char c = m_CharBuffer.front(); 
		m_CharBuffer.pop(); 
		return c;
	}
}

void KeyboardClass::OnKeyPressed(const unsigned char key)
{
	m_KeyStates[key] = true;
	m_KeyBuffer.push(KeyEvent(KeyEvent::EventType::PRESS, key));
}

void KeyboardClass::OnKeyReleased(const unsigned char key)
{
	m_KeyStates[key] = false;
	m_KeyBuffer.push(KeyEvent(KeyEvent::EventType::RELEASE, key));
}

void KeyboardClass::OnCharInput(const unsigned char key)
{
	m_CharBuffer.push(key);
}