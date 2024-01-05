#include "TextBox.hpp"
#include "Explore.hpp"
#include <fstream>
#include <filesystem>
#include <locale>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
bool isAbsolutePath(std::wstring path)
{
	if(path.size() < 3) return false;
	if(	path[0] == 'C' && path[1] == ':' && (path[2] == '/' || path[2] == '\\')) return true;
	return false;
}
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
bool isAbsolutePath(std::wstring path)
{
	if(path.size() < 1) return false;
	if(path[0] == '/') return true;
	return false;
}
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

std::wstring adjustSlash(std::wstring path)
{
	std::wstring res = path;
	for(auto& ch : res)
		if(ch == '\\') ch = '/';
	return res;
}

std::wstring normalizePath(std::wstring path)
{
	auto v = split(path, '/');
	std::wstring res = v[0] + "/";

	for(size_t i = 1; i < v.size(); i++)
	{
		if(v[i] == "..")
		{
			if(i == 1) continue;
			res = res.substr(0, res.rfind('/')+1);
			continue;
		}
		if(v[i] == ".") continue;
		res += v[i] + "/";
	}
	res.erase(res.size()-1);

	return res;
}

//not my code
bool isValidUTF8(std::string& str)
{
	for(size_t i = 0; i < str.length();)
	{
		unsigned char current = static_cast<unsigned char>(str[i]);
		// Single-byte character (0xxxxxxx)
		if(current <= 0x7F)	i += 1;
		else if(current <= 0xDF && i + 1 < str.length())
		{
			// Two-byte character (110xxxxx 10xxxxxx)
			if((static_cast<unsigned char>(str[i + 1]) & 0xC0) != 0x80) {
				return false; // Invalid UTF-8 sequence
			}
			i += 2;
		}
		else if(current <= 0xEF && i + 2 < str.length())
		{
			// Three-byte character (1110xxxx 10xxxxxx 10xxxxxx)
			if((static_cast<unsigned char>(str[i + 1]) & 0xC0) != 0x80 ||
				(static_cast<unsigned char>(str[i + 2]) & 0xC0) != 0x80) {
				return false; // Invalid UTF-8 sequence
			}
			i += 3;
		}
		else return false; // Invalid UTF-8 character
	}
	return true;
}
//end

struct ReadedFile
{
	sf::String content;
	bool utf8_encode;
};

ReadedFile readFile(std::wstring filepath)
{
	std::ifstream file(filepath, std::ios::binary);
	if(!file.is_open()) return {"", false};
	
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	if(isValidUTF8(content))
	{
		std::wstringstream buffer;
		std::wifstream file;

		file.imbue(std::locale(".UTF8"));

		file.open(filepath);
		
		if(file) buffer << file.rdbuf();
		else return {"", true};
		
		file.close();

		return {buffer.str(), true};
	}
	return {content, false};
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
	sf::String file_path;
	bool utf8_encode;
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
void addTab(std::vector<Tab>&, gui::TextBox*, sf::String, size_t&);

int main(int argc, char* argv[])
{
	sf::String current_path = std::filesystem::current_path().wstring(), exe_path = getExePath();
	sf::String visual_path = current_path;

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
		addTab(tabs, base_textbox, "", current_file);
	}

	bool explore_mode = true;
	gui::Explore explore;
	explore.init(win);
	explore.setCurrentPath(visual_path);
	explore.setFont(font);
	explore.setSize((sf::Vector2f)win.getSize());
	explore.setFileImage(exe_path + "/icons/explorer/file.png");
	explore.setFolderImage(exe_path + "/icons/explorer/folder.png");
	sf::String path_to_open;

	win.setFramerateLimit(120);
	while(win.isOpen())
	{
		while(win.pollEvent(e))
		{
			if(edit_mode) tabs[current_file].textbox.listen(e);
			if(explore_mode)
			{
				explore.listen(e);
				path_to_open = explore.getSelectedFile();
				if(path_to_open != L"")
				{
					addTab(tabs, base_textbox, adjustSlash(path_to_open), current_file);

					explore.clearSelectedFile();
					edit_mode = true;
					explore_mode = false;
					win.pollEvent(e);
				}
			}

			switch(e.type)
			{
				case sf::Event::KeyPressed:
					if(e.key.code == sf::Keyboard::Escape && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
					{
						if(edit_mode)
						{
							edit_mode = false;
							explore_mode = true;
						}
						else
						{
							explore_mode = false;
							edit_mode = true;
						}
					}
					if(e.key.code == sf::Keyboard::Tab)
					{
						if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
						{
							//switch tabs
						}
					}
					break;
				case sf::Event::Resized:

					view.reset({0, 0, (float)win.getSize().x, (float)win.getSize().y});
					win.setView(view);

					base_textbox->setSize((sf::Vector2f)win.getSize());
					for(Tab& tab_i : tabs) tab_i.textbox.setSize((sf::Vector2f)win.getSize());
					explore.setSize((sf::Vector2f)win.getSize());

					break;
				case sf::Event::Closed:
					win.close();
					break;
			}
		}
		win.clear();
		
		if(explore_mode) explore.draw();
		if(edit_mode) tabs[current_file].textbox.draw();
		
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

void addTab(std::vector<Tab>& tabs, gui::TextBox* bt, sf::String path, size_t& index)
{
	if(tabs.size() != 0 && tabs[0].file_path == "" && tabs[0].textbox.getTextAsString() == "")
	{
		index = 0;
		goto overrideTab;
	}
	
	for(size_t i = 0; i < tabs.size(); i++)
	{
		if(tabs[i].file_path == path)
		{
			index = i;
			return;
		}
	}

	index = tabs.size();
	tabs.push_back(Tab{*bt, path});
	
	overrideTab:{}

	ReadedFile file = readFile(path);
	tabs[index].utf8_encode = file.utf8_encode;
	tabs[index].textbox.writeText(file.content);
}
