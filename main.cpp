#include "TextBox.hpp"
#include "Explore.hpp"
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
std::string getExePath()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return std::string(buffer).substr(0, std::string(buffer).find_last_of("\\/"));
}
#else
#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>

std::string getExePath()
{
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	const char* path;
	if(count != -1) {
		path = dirname(result);
	}
	return *path;
}
#endif

//not my code
bool isValidUTF8(std::string& str)
{
	for(size_t i = 0; i < str.length(); ) {
		unsigned char current = static_cast<unsigned char>(str[i]);
		if(current <= 0x7F) {
			// Single-byte character (0xxxxxxx)
			i += 1;
		}
		else if(current <= 0xDF && i + 1 < str.length()) {
			// Two-byte character (110xxxxx 10xxxxxx)
			if((static_cast<unsigned char>(str[i + 1]) & 0xC0) != 0x80) {
				return false; // Invalid UTF-8 sequence
			}
			i += 2;
		}
		else if(current <= 0xEF && i + 2 < str.length()) {
			// Three-byte character (1110xxxx 10xxxxxx 10xxxxxx)
			if((static_cast<unsigned char>(str[i + 1]) & 0xC0) != 0x80 ||
				(static_cast<unsigned char>(str[i + 2]) & 0xC0) != 0x80) {
				return false; // Invalid UTF-8 sequence
			}
			i += 3;
		}
		else {
			return false; // Invalid UTF-8 character
		}
	}
	return true;
}
//end

sf::String readFile(std::string filepath)
{
	std::ifstream file(filepath, std::ios::binary);
	if(!file.is_open()) return "";
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	if(isValidUTF8(content))
	{
		std::wstringstream buffer;
		std::wifstream file;

		file.imbue(std::locale(".UTF8"));

		file.open(filepath);
		if(file)buffer << file.rdbuf();
		else return "";
		file.close();

		return buffer.str();
	}
	return content;
}

bool fileWrite(std::string path, std::string text)
{
	std::fstream file;
	file.open(path, std::ios::out);
	if(!file) return false;
	file << text;
	file.close();
	return true;
}
bool fileWrite(std::string path, std::wstring text)
{
	std::wfstream file;
	file.open(path, std::ios::out);
	if(!file) return false;
	file << text;
	file.close();
	return true;
}

//(68, 166, 239)
//(182, 89, 210)
//(122, 178, 212)

struct Tab
{
	gui::TextBox textbox;
	std::string file_path;
	std::string encode;
};

sf::Color
	//textbox
	textbox_text_color = sf::Color(200, 200, 200),
	textbox_background_color = sf::Color(40, 44, 52, 150),
	textbox_cursor_color = sf::Color(200, 200, 200),
	textbox_selection_color,
	//explorer
	explorer_background_color,
	explorer_file_text_color,
	explorer_folder_text_color;

std::string textbox_background_image, font_path;

void loadWindowIcon(sf::RenderWindow&, std::string);
void setUpTextBox();

int main(int argc, char* argv[])
{
	sf::String current_path = std::filesystem::current_path().wstring(), exe_path = getExePath();

	sf::RenderWindow win(sf::VideoMode(800, 500), "SpicyCode");
	sf::View view;
	sf::Event e;
	
	loadWindowIcon(win, exe_path + "/icon.png");
	font_path = exe_path + "/font/font.otf";

	sf::Font font;
	font.loadFromFile(exe_path + "/font/font.otf");

	bool edit_mode = false;
	gui::TextBox* base_textbox = new gui::TextBox;
	base_textbox->init(win);
	base_textbox->setSize((sf::Vector2f)win.getSize());
	base_textbox->setFocus(true);
	base_textbox->setFont(font);
	base_textbox->setTextColor(textbox_text_color);
	base_textbox->setCursorColor(textbox_cursor_color);
	base_textbox->setBackgroundColor(textbox_background_color);

	bool explore_mode = true;
	gui::Explore explore;
	explore.init(win);
	explore.setCurrentPath(current_path);
	explore.setFont(font);
	explore.setSize((sf::Vector2f)win.getSize());
	explore.setFileImage(exe_path + "/icons/explorer/file.png");
	explore.setFolderImage(exe_path + "/icons/explorer/folder.png");

	std::vector<Tab> tabs;
	std::vector<sf::String> file_paths;
	std::size_t current_file = 0;
	
	if(argc > 1)
	{
		
		for(int i = 1; i < argc; i++)
		{
			
		}
	}
	if(tabs.size() == 0)
	{
		
	}

	win.setFramerateLimit(120);
	while(win.isOpen())
	{
		while(win.pollEvent(e))
		{
			if(explore_mode) explore.listen(e);
			if(edit_mode) explore.listen(e);

			if(e.type == sf::Event::Resized)
			{
				view.reset({0, 0, (float)win.getSize().x, (float)win.getSize().y});
				win.setView(view);
				
				for(Tab& tab_i : tabs) tab_i.textbox.setSize((sf::Vector2f)win.getSize());
				explore.setSize((sf::Vector2f)win.getSize());
			}
			if(e.type == sf::Event::Closed) win.close();
		}
		win.clear();
		
		if(explore_mode) explore.draw();
		if(edit_mode) explore.draw();
		
		win.display();
	}
	return 0;
}

void loadWindowIcon(sf::RenderWindow& window, std::string path)
{
	sf::Image img;
	if(!img.loadFromFile(path)) return;
	window.setIcon(img.getSize().x, img.getSize().y, img.getPixelsPtr());
}