#include "ui_widgets.h"


bool uiWidgetsInputText_256(std::string& text_buffer , const std::string& label)
{

	std::string buffer;
	buffer.resize(256);

	for (uint32_t i = 0; i < text_buffer.length(); i++) {

		buffer[i] = text_buffer[i];

	}

	

	if (ImGui::InputText(label.c_str(), buffer.data(), buffer.length(), ImGuiInputTextFlags_EnterReturnsTrue)) {
		text_buffer = buffer;
		return true;
	}

	return false;
}
