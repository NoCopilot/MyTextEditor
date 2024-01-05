#include "Explore.hpp"

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
		background.setFillColor(sf::Color(30, 30, 30));
		resetView();

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
			case sf::Event::MouseWheelScrolled:
				//e.mouseWheelScroll.delta  down -> -1 | top   -> 1
				//e.mouseWheel.delta        left -> -1 | right -> 1
				break;
			case sf::Event::KeyPressed:
				if(e.key.code == sf::Keyboard::Up)
				{
					if(selection_index <= 0) selection_index = current_dir.size() - 1;
					else selection_index--;
				}
				if(e.key.code == sf::Keyboard::Down)
				{
					if(selection_index >= (current_dir.size() - 1)) selection_index = 0;
					else selection_index++;
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
					selected_file = current_path + L"/" + current_dir[selection_index].name;
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
	}
	void Explore::draw()
	{
		if(win == nullptr) return;

		sf::View temp_view = win->getView();
		win->setView(view);

		//draw background
		win->draw(background);

		selection_rect.setPosition(sf::Vector2f(0.f, selection_index * line_height + line_height));
		win->draw(selection_rect);

		//draw path
		text.setFillColor(path_color);
		text.setString(current_path);
		text.setPosition(sf::Vector2f(0.f, 0));
		win->draw(text);

		size_t i;
		//draw directories
		text.setFillColor(dir_color);
		icon_rect.setTexture(&folder_image);
		for(i = 0; i < current_dir.size() && current_dir[i].size == ""; i++)
		{
			icon_rect.setPosition(sf::Vector2f(icon_offset * 0.5f, (i + 1) * line_height + icon_offset * 0.5f));
			win->draw(icon_rect);

			text.setString(current_dir[i].name);
			text.setPosition(sf::Vector2f(line_height + icon_offset * 0.5f, line_height * (i + 1)));
			win->draw(text);
		}

		//draw files
		text.setFillColor(file_color);
		icon_rect.setTexture(&file_image);
		for(; i < current_dir.size(); i++)
		{
			icon_rect.setPosition(sf::Vector2f(icon_offset * 0.5f, (i + 1) * line_height + icon_offset * 0.5f));
			win->draw(icon_rect);

			text.setString(current_dir[i].name);
			text.setPosition(sf::Vector2f(line_height + icon_offset * 0.5f, line_height * (i + 1)));
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
		int i = current_path.find_last_of(L"\\/");
		if(i == -1) return;
		current_path = current_path.substr(0, current_path.find_last_of(L"\\/"));
		updateElements();
	}
	void Explore::goTo(std::wstring path)
	{
		if(std::filesystem::exists(current_path + L"/" + path))
		{
			current_path += L"/" + path;
			updateElements();
		}
	}
	void Explore::updateElements()
	{
		selection_index = 0;
		view.setCenter(sf::Vector2f(view.getSize().x * 0.5f, view.getSize().y * 0.5f));
		current_dir = getElementsInPath(current_path);
		current_dir.insert(current_dir.begin(), Element("..", "", ""));
		updateMaxSize();
	}

	void Explore::resetView()
	{
		view.reset(sf::FloatRect(view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f,
			size.x - (scrollbar_x_visible ? scrollbar_size : 0), size.y - (scrollbar_x_visible ? scrollbar_size : 0)));
		view.setViewport(sf::FloatRect(pos.x / win->getSize().x, pos.y / win->getSize().y,
			size.x / win->getSize().x, size.y / win->getSize().y));
		background.setSize(view.getSize());
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

	checkViewX:
		{
		}
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

	checkViewEnd:
		{
		}
		background.setPosition({view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f});
	}

	void Explore::updateMaxSize()
	{
		//height
		max_height = line_height * current_dir.size();
		//width
		max_width = 0.f;
		for(Element& e : current_dir)
		{
			text.setString(e.name);
			if(text.getLocalBounds().width > max_width) max_width = text.getLocalBounds().width;
		}
		if(max_width < view.getSize().x) max_width = view.getSize().x;
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
		resetView();
	}
	void Explore::setSize(sf::Vector2f new_size)
	{
		size = new_size;
		resetView();
	}

	void Explore::setFont(sf::Font& font)
	{
		text.setFont(font);
	}
	void Explore::setTextSize(uint32_t value)
	{
		text.setCharacterSize(value);
		line_height = value + 10;
		icon_rect.setSize(sf::Vector2f(line_height - icon_offset, line_height - icon_offset));
	}

	void Explore::setCurrentPath(std::wstring new_path)
	{
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
		return current_path;
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

		for(auto const& dir : std::filesystem::directory_iterator{path})
		{
			temp_str = dir.path().wstring();
			temp_time = std::format("{}", std::filesystem::last_write_time(dir));

			if(std::filesystem::exists(dir.path())) {
				try
				{
					std::filesystem::file_status file_status = std::filesystem::status(dir.path());
					if(std::filesystem::is_directory(file_status))
					{
						res.insert(res.begin(), Element(
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
							toString(dir.file_size())
						));
					}
				}
				catch(const std::filesystem::filesystem_error& ex)
				{
					//error
				}
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
				res.push_back(temp_str.substr(temp_str.find_last_of(L"\\/") + 1));
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

	std::string toString(int n)
	{
		int i = 0;
		std::string res("");

		while(n > 0)
		{
			res = res + (char)(n % 10);
			n /= 10;
		}
		if(res == "") return "0";
		return res;
	}
}