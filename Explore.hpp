#ifndef EXPLORE_HPP
#define EXPLORE_HPP

#include "Scrollbar.hpp"
#include <filesystem>
#include <string>
#include <format>
#include <vector>

namespace gui
{
	struct Element
	{
		sf::String name;
		std::string last_edit;
		std::string size;
	};

	std::vector<Element> getElementsInPath(std::wstring);
	std::vector<std::wstring> getElementsInPath(std::wstring&, bool);
	std::vector<std::wstring> getFilesInPath(std::wstring);
	std::vector<std::wstring> getDirsInPath(std::wstring);
	std::string toString(int);

	class Explore
	{
	public:
		////////////////////////////////////
		//         CLASS FUNCTIONS        //
		////////////////////////////////////

		void init(sf::RenderWindow&);
		void listen(sf::Event&);
		void draw();

		void clearSelectedFile();

		////////////////////////////////////
		//              SETS              //
		////////////////////////////////////

		void setWindow(sf::RenderWindow&);
		void setPos(sf::Vector2f);
		void setSize(sf::Vector2f);

		void setFont(sf::Font&);
		void setTextSize(uint32_t);

		void setCurrentPath(std::wstring);
		void setFolderImage(std::string);
		void setFileImage(std::string);

		void setSelectionColor(sf::Color);

		////////////////////////////////////
		//              GETS              //
		////////////////////////////////////

		std::wstring getCurrentPath();
		sf::Vector2f getPos();
		sf::Vector2f getSize();

		std::wstring getSelectedFile();

	private:
		////////////////////////////////////
		//           VARIABLES            //
		////////////////////////////////////

		sf::RenderWindow* win;
		sf::View view;
		sf::Vector2f pos, size;
		sf::RectangleShape background;

		float max_width, max_height;
		Scrollbar scrollbar_x, scrollbar_y;
		float scrollbar_size;
		bool scrollbar_x_visible, scrollbar_y_visible;

		std::wstring current_path;
		std::vector<Element> current_dir;

		sf::Texture folder_image, file_image;
		sf::RectangleShape icon_rect;
		float icon_offset;

		sf::RectangleShape selection_rect;
		size_t selection_index;
		std::wstring selected_file;

		sf::Text text;
		sf::Color dir_color, file_color, path_color;
		float line_height;

		////////////////////////////////////
		//            FUNCTIONS           //
		////////////////////////////////////

		void goTo(std::wstring);
		void parentDirectory();
		void updateElements();

		void resetView();
		void checkView();
		void updateMaxSize();
	};
}
#endif
