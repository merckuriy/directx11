#pragma once

inline wchar_t* CharToWChar(char* mbString)
{
	int len = 0;
	size_t returnValue;
	len = static_cast<int>(strlen(mbString)) + 1;
	//������ ��������, �� ���-��, �� ����, ������ ���� �������.
	wchar_t *ucString = new wchar_t[len]; 
	mbstowcs_s(&returnValue, ucString, len, mbString, len);
	return ucString;
}