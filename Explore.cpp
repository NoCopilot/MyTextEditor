#include "Explore.hpp"
#include <iostream>

sf::String adjustSlash(sf::String path)
{
	std::wstring res = path;
	for(auto& ch : res)
		if(ch == '\\') ch = '/';
	return res;
}

sf::String formatSize(sf::String _size)
{
	if(_size == "0") return "1 KB";
	if(_size.getSize() <= 3) return _size + " KB";
	size_t i = _size.getSize()-3;
	while(true)
	{
		_size.insert(i, ".");
		if(i <= 3) break;
		i -= 3;
	}
	return _size + " KB";
}

sf::String toString(int n)
{
	int i = 0;
	sf::String res("");

	while(n > 0)
	{
		res = (char)(n % 10 + 48) + res;
		n /= 10;
	}
	if(res == "") return "0";
	return res;
}

namespace gui
{
	////////////////////////////////////
	//         CLASS FUNCTIONS        //
	////////////////////////////////////

	void Explore::init(sf::RenderWindow& window)
	{
		win = &window;
		pos = sf::Vector2f(0.f, 0.f);
		size = sf::Vector2f(300.f, 300.f);
		background.setFillColor(sf::Color::Black);
		path_background_rect.setFillColor(sf::Color::Black);
		resetView();

		offset = 30.f;
		icon_offset = 10.f;
		setTextSize(18);
		dir_color = sf::Color::White;
		file_color = sf::Color::White;
		path_color = sf::Color::White;

		selection_rect.setFillColor(sf::Color(200, 200, 200, 50));
		selection_index = 0;
	}
	void Explore::listen(sf::Event& e)
	{
		switch(e.type)
		{
			case sf::Event::Resized:
				view.setSize((sf::Vector2f)win->getSize());
				resetView();
				break;
			case sf::Event::KeyPressed:
				if(e.key.code == sf::Keyboard::Up)
				{
					if(selection_index <= 0) selection_index = current_dir.size() - 1;
					else selection_index--;

					goto selViewCheck;
				}
				if(e.key.code == sf::Keyboard::Down)
				{
					if(selection_index >= (current_dir.size() - 1)) selection_index = 0;
					else selection_index++;

					goto selViewCheck;
				}
				if(e.key.code == sf::Keyboard::Enter)
				{
					//navigate back | open folder | select file
					if(selection_index == 0)
					{
						parentDirectory();
						return;
					}
					if(current_dir[selection_index].size == "")
					{
						goTo(current_dir[selection_index].name);
						return;
					}
					selected_file = current_path + current_dir[selection_index].name;
				}
				if(e.key.code == sf::Keyboard::N)
				{
					if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
					{
						if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
						{
							//create folder
						}
						else
						{
							//create file
						}
					}
				}
				break;
		}
		return;
		selViewCheck:{}
		if(((selection_index + 1) * line_height) > (view.getCenter().y + view.getSize().y * 0.5f))
		{
			background.move(sf::Vector2f(0.f, ((selection_index + 1) * line_height) - (view.getCenter().y + view.getSize().y * 0.5f)));
			view.move(sf::Vector2f(0.f, ((selection_index + 1) * line_height) - (view.getCenter().y + view.getSize().y * 0.5f)));
		}
		if((selection_index * line_height) < (view.getCenter().y - view.getSize().y * 0.5f))
		{
			background.move(sf::Vector2f(0.f, (selection_index * line_height) - (view.getCenter().y - view.getSize().y * 0.5f)));
			view.move(sf::Vector2f(0.f, (selection_index * line_height) - (view.getCenter().y - view.getSize().y * 0.5f)));
		}
	}
	void Explore::draw()
	{
		if(win == nullptr) return;

		win->draw(path_background_rect);

		//draw path
		text.setFillColor(path_color);
		text.setString(current_path);
		text.setPosition(sf::Vector2f(pos.x, pos.y-line_height));
		win->draw(text);

		sf::View temp_view = win->getView();
		win->setView(view);

		//draw background
		win->draw(background);

		selection_rect.setPosition(sf::Vector2f(0.f, selection_index * line_height));
		win->draw(selection_rect);

		size_t i;
		//draw directories
		text.setFillColor(dir_color);
		icon_rect.setTexture(&folder_image);
		for(i = 0; i < current_dir.size() && current_dir[i].size == ""; i++)
		{
			icon_rect.setPosition(sf::Vector2f(icon_offset * 0.5f, i * line_height + icon_offset * 0.5f));
			win->draw(icon_rect);

			text.setString(current_dir[i].name);
			text.setPosition(sf::Vector2f(line_height + icon_offset * 0.5f, line_height * i));
			win->draw(text);

			text.move(sf::Vector2f(max_name_width + offset, 0));
			text.setString(current_dir[i].last_edit);
			win->draw(text);
		}

		//draw files
		text.setFillColor(file_color);
		icon_rect.setTexture(&file_image);
		for(; i < current_dir.size(); i++)
		{
			icon_rect.setPosition(sf::Vector2f(icon_offset * 0.5f, i * line_height + icon_offset * 0.5f));
			win->draw(icon_rect);

			text.setString(current_dir[i].name);
			text.setPosition(sf::Vector2f(line_height + icon_offset * 0.5f, line_height * i));
			win->draw(text);

			text.move(sf::Vector2f(max_name_width + offset, 0));
			text.setString(current_dir[i].last_edit);
			win->draw(text);

			text.move(sf::Vector2f(text.getLocalBounds().width + offset, 0));
			text.setString(current_dir[i].size);
			text.move(sf::Vector2f(max_size_width - text.getLocalBounds().width, 0));
			win->draw(text);
		}

		win->setView(temp_view);
	}

	void Explore::clearSelectedFile()
	{
		selected_file = L"";
	}

	////////////////////////////////////
	//            FUNCTIONS           //
	////////////////////////////////////

	void Explore::parentDirectory()
	{
		selection_index = 0;
		int i = current_path.rfind(L"/", current_path.size()-2);
		if(i == -1) return;
		current_path = current_path.substr(0, i+1);
		updateElements();
	}
	void Explore::goTo(std::wstring path)
	{
		if(std::filesystem::exists(current_path + path))
		{
			current_path += path + L"/";
			updateElements();
		}
	}
	void Explore::updateElements()
	{
		selection_index = 0;
		view.setCenter(sf::Vector2f(view.getSize().x * 0.5f, view.getSize().y * 0.5f));
		background.setPosition(sf::Vector2f(0.f, 0.f));
		current_dir = getElementsInPath(current_path);
		current_dir.insert(current_dir.begin(), Element("..", "", ""));
		updateMaxSize();
	}

	void Explore::reflash()
	{
		current_dir = getElementsInPath(current_path);
		current_dir.insert(current_dir.begin(), Element("..", "", ""));
		updateMaxSize();
		if(selection_index >= current_dir.size()) selection_index = current_dir.size()-1;
	}

	void Explore::resetView()
	{
		view.reset(sf::FloatRect(view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f,
			size.x, size.y));
		
		view.setViewport(sf::FloatRect(pos.x / win->getSize().x, pos.y / win->getSize().y,
			size.x / win->getSize().x, size.y / win->getSize().y));
		
		background.setSize(view.getSize());
		path_background_rect.setPosition(sf::Vector2f(pos.x, pos.y - line_height));
		path_background_rect.setSize(sf::Vector2f(size.x, line_height));

		checkView();
		updateMaxSize();
	}

	void Explore::checkView()
	{
		/*----General check on height----*/
		float temp;
		if(max_height < view.getSize().y)
		{
			view.setCenter({view.getCenter().x, view.getSize().y * 0.5f});
			goto checkViewX;
		}

		/*----Checking when too high----*/
		temp = view.getCenter().y - view.getSize().y * 0.5f;
		if(temp < 0.f) view.move({0.f, -temp});

		/*----Checking when too down----*/
		temp = view.getCenter().y + view.getSize().y * 0.5f - max_height;
		if(temp > 0.f) view.move({0.f, -temp});

		checkViewX:{}
		/*----General check on width----*/
		if(max_width < view.getSize().x)
		{
			view.setCenter({view.getSize().x * 0.5f, view.getCenter().y});
			goto checkViewEnd;
		}

		/*----Checking when too left----*/
		temp = view.getCenter().x - view.getSize().x * 0.5f;
		if(temp < 0.f) view.move({-temp, 0.f});

		/*----Checking when too right----*/
		temp = view.getCenter().x + view.getSize().x * 0.5f - max_width;
		if(temp > 0.f) view.move({-temp, 0.f});

		checkViewEnd:{}
		background.setPosition({view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f});
	}

	void Explore::updateMaxSize()
	{
		//height
		max_height = line_height * current_dir.size();
		//width
		max_name_width = 0.f;
		max_size_width = 0.f;
		for(Element& e : current_dir)
		{
			text.setString(e.name);
			if(text.getLocalBounds().width > max_name_width) max_name_width = text.getLocalBounds().width;
			
			text.setString(e.size);
			if(text.getLocalBounds().width > max_size_width) max_size_width = text.getLocalBounds().width;
		}
		if(max_name_width != 0.f)
			text.setString(current_dir[current_dir.size()-1].last_edit);

		max_width = max_name_width +
					max_size_width +
					text.getLocalBounds().width +
					offset * (max_size_width != 0.f ? 2 : 1) +
					icon_rect.getSize().x + icon_offset*2;
		//seleciton
		selection_rect.setSize(sf::Vector2f(max_width, line_height));
	}

	////////////////////////////////////
	//              SETS              //
	////////////////////////////////////

	void Explore::setWindow(sf::RenderWindow& window)
	{
		win = &window;
	}
	void Explore::setPos(sf::Vector2f new_pos)
	{
		pos = new_pos;
		pos.y += line_height;
		resetView();
	}
	void Explore::setSize(sf::Vector2f new_size)
	{
		size = new_size;
		size.y -= line_height;
		resetView();
	}
	void Explore::setBackgroundColor(sf::Color _color)
	{
		background.setFillColor(_color);
		path_background_rect.setFillColor(_color);
	}
	void Explore::setFont(sf::Font& font)
	{
		text.setFont(font);
	}
	void Explore::setTextSize(uint32_t value)
	{
		text.setCharacterSize(value);

		pos.y -= line_height;
		size.y += line_height;

		line_height = value + 10;

		pos.y += line_height;
		size.y -= line_height;

		icon_rect.setSize(sf::Vector2f(line_height - icon_offset, line_height - icon_offset));
		selection_rect.setSize(sf::Vector2f(selection_rect.getSize().x, line_height));
		resetView();
	}

	void Explore::setCurrentPath(std::wstring new_path)
	{
		if(new_path[new_path.size()-1] != L'/') new_path += L'/';
		current_path = new_path;
		updateElements();
	}
	void Explore::setFolderImage(std::string path)
	{
		folder_image.loadFromFile(path);
	}
	void Explore::setFileImage(std::string path)
	{
		file_image.loadFromFile(path);
	}

	void Explore::setSelectionColor(sf::Color _color)
	{
		selection_rect.setFillColor(_color);
	}

	////////////////////////////////////
	//              GETS              //
	////////////////////////////////////

	std::wstring Explore::getCurrentPath()
	{
		return current_path.substr(0, current_path.size()-1);
	}

	sf::Vector2f Explore::getPos()
	{
		return pos;
	}

	sf::Vector2f Explore::getSize()
	{
		return size;
	}

	std::wstring Explore::getSelectedFile()
	{
		return selected_file;
	}

	////////////////////////////////////
	//        Global Functions        //
	////////////////////////////////////

	std::vector<Element> getElementsInPath(std::wstring path)
	{
		std::vector<Element> res;
		std::wstring temp_str;
		std::string temp_time;

		size_t dir_counter = 0;
		for(auto const& dir : std::filesystem::directory_iterator{path})
		{
			temp_str = dir.path().wstring();
			temp_time = std::format("{}", std::filesystem::last_write_time(dir));

			try
			{
				if(std::filesystem::is_directory(std::filesystem::status(dir.path())))
				{
					res.insert(res.begin() + dir_counter++, Element(
						temp_str.substr(temp_str.find_last_of(L"\\/") + 1),
						temp_time.substr(0, temp_time.rfind('.')),
						""
					));
				}
				else
				{
					res.push_back(Element(
						temp_str.substr(temp_str.find_last_of(L"\\/") + 1),
						temp_time.substr(0, temp_time.rfind('.')),
						formatSize(toString(dir.file_size() >> 10))
					));
				}
			}
			catch(const std::filesystem::filesystem_error& ex)
			{
				//error
			}
		}
		return res;
	}

	std::vector<std::wstring> getElementsInPath(std::wstring& path, bool fod)
	{
		std::vector<std::wstring> res;
		std::wstring temp_str;
		for(auto const& dir : std::filesystem::directory_iterator{path})
			if(std::filesystem::is_directory(dir.path()) == fod)
			{
				temp_str = dir.path().wstring();
				res.push_back(temp_str.substr(temp_str.rfind(L"/") + 1));
			}
		return res;
	}

	std::vector<std::wstring> getFilesInPath(std::wstring path)
	{
		return getElementsInPath(path, false);
	}

	std::vector<std::wstring> getDirsInPath(std::wstring path)
	{
		return getElementsInPath(path, true);
	}
}