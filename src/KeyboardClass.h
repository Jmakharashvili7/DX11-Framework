#pragma once
#include <Queue>
#include "KeyEvent.h"

class KeyboardClass
{
private:
	bool m_AutoRepeatKeys = false;
	bool m_AutoRepeatChars = false;
	bool m_KeyStates[256];
	std::queue<KeyEvent> m_KeyBuffer;
	std::queue<unsigned char> m_CharBuffer;
public:
	KeyboardClass();
	inline bool IsKeyPressed(const unsigned char keycode) { return m_KeyStates[keycode]; }
	inline bool KeyBufferIsEmpty() { return m_KeyBuffer.empty(); }
	inline bool CharBufferIsEmpty() { return m_CharBuffer.empty(); }
	KeyEvent ReadKey();
	unsigned char ReadChar();
	void OnKeyPressed(const unsigned char key);
	void OnKeyReleased(const unsigned char key);
	void OnCharInput(const unsigned char key);
	inline void EnableAutoRepeatKeys() { m_AutoRepeatKeys = true; }
	inline void DisableAutoRepeatKeys() { m_AutoRepeatKeys = false; }
	inline void EnableAutoRepeatChars() { m_AutoRepeatChars = true; }
	inline void DisableAutoRepeatChars() { m_AutoRepeatChars = false; }
	inline bool IsKeyAutoRepeat() const { return m_AutoRepeatKeys; }
	inline bool IsCharsAutoRepeat() const { return m_AutoRepeatChars; }
};

