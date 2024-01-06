#include "TextBox.hpp"
#include "Explore.hpp"
#include <fstream>
#include <filesystem>
#include <locale>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
bool isAbsolutePath(sf::String path)
{
	if(path.getSize() < 3) return false;
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
bool isAbsolutePath(sf::String path)
{
	if(path.getSize() < 1) return false;
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

sf::String normalizePath(sf::String path)
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
		if(v[i] == "." || v[i] == "") continue;
		res += v[i] + "/";
	}
	res.erase(res.size()-1);

	return res;
}

int rfind(sf::String str, sf::Uint32 ch)
{
	if(str.getSize() == 0) return -1;
	for(size_t i = str.getSize()-1; i > 0; i--)
	{
		if(str[i] == ch) return i;
	}
	if(str[0] != ch) return -1;
	return 0;
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
	sf::String current_path = adjustSlash(std::filesystem::current_path().wstring()), exe_path = adjustSlash(getExePath());
	sf::String visual_path = current_path;

	sf::RenderWindow win(sf::VideoMode(800, 500), "SpicyCode");
	sf::View view;
	sf::Event e;
	
	loadWindowIcon(win, exe_path + "/icons/window/icon.png");
	font_path = exe_path + "/font/font.otf";

	sf::Font font;
	font.loadFromFile(exe_path + "/font/font.otf");

	bool explore_mode = true;
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

	sf::String temp;
	if(argc > 1)
	{
		
		for(int i = 1; i < argc; i++)
		{
			if(!isAbsolutePath(argv[i])) temp = normalizePath(current_path + "/" + adjustSlash(argv[i]));
			std::filesystem::file_status file_status = std::filesystem::status(temp.toWideString());
			if(!std::filesystem::exists(file_status))
			{
				sf::String temp1 = temp.substring(rfind(temp, '/')+1);
				temp = temp.substring(0, rfind(temp, '/'));
				if(!std::filesystem::exists(temp.toWideString())) continue;
				if(std::filesystem::is_directory(std::filesystem::status(temp.toWideString())))
				{
					visual_path = temp;
					temp += "/" + temp1;
					fileWrite(temp, L"");
					addTab(tabs, base_textbox, temp, current_file);

					edit_mode = true;
					explore_mode = false;
				}
				continue;
			}
			if(std::filesystem::is_directory(file_status))
			{
				visual_path = temp;
				continue;
			}
			else
			{
				addTab(tabs, base_textbox, temp, current_file);
				visual_path = temp.substring(0, rfind(temp, '/'));

				edit_mode = true;
				explore_mode = false;
			}
		}
	}
	if(tabs.size() == 0)
	{
		addTab(tabs, base_textbox, "", current_file);
	}

	gui::Explore explore;
	explore.init(win);
	explore.setFont(font);
	explore.setCurrentPath(visual_path);
	explore.setPos(sf::Vector2f(0.f, 0.f));
	explore.setSize((sf::Vector2f)win.getSize());
	explore.setFileImage(exe_path + "/icons/explorer/file.png");
	explore.setFolderImage(exe_path + "/icons/explorer/folder.png");
	explore.setTextSize(20);
	explore.setSelectionColor(sf::Color(25, 34, 71));

	win.setFramerateLimit(120);
	while(win.isOpen())
	{
		while(win.pollEvent(e))
		{
			if(edit_mode) tabs[current_file].textbox.listen(e);
			if(explore_mode)
			{
				explore.listen(e);
				temp = explore.getSelectedFile();
				if(temp != L"")
				{
					addTab(tabs, base_textbox, adjustSlash(temp), current_file);

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
						break;
					}
					if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
					{
						if(edit_mode)
						{
							if(e.key.code == sf::Keyboard::Tab)
							{
								if(current_file == (tabs.size() - 1)) current_file = 0;
								else current_file++;
								break;
							}
							if(e.key.code == sf::Keyboard::W)
							{
								if(!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
								{
									if(tabs[current_file].utf8_encode)
										fileWrite(tabs[current_file].file_path, tabs[current_file].textbox.getTextAsString().toAnsiString());
									else
										fileWrite(tabs[current_file].file_path, tabs[current_file].textbox.getTextAsString().toWideString());
								}
								if(tabs.size() > 1)
								{
									tabs.erase(tabs.begin() + current_file);
									if(current_file != 0) current_file--;
								}
								else
								{
									tabs[current_file].textbox.clearText();
									tabs[current_file].file_path = "";
								}
							}
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
