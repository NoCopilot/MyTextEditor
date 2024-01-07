#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP

#include "Scrollbar.hpp"
#include <vector>
#include <thread>

#define _COPY 3
#define _CUT 24
#define _PASTE 22
#define _ENTER 13
#define _DELETE 8
#define _CDELETE 127
#define _TAB 9

std::vector<sf::String> split(sf::String, sf::String);

namespace gui
{
	struct Interval
	{
		std::size_t x, y, n;
		sf::Color color;
		Interval(std::size_t X, std::size_t Y, std::size_t N, sf::Color c)
		{
			x = X; y = Y; n = N;
			color = c;
		}
	};

	class TextBox
	{
	public:
		/*--------------------------------------Class Functions---------------------------------------*/
		void init(sf::RenderWindow&);
		void listen(sf::Event&);
		void draw();
		void writeText(sf::String);
		void writeText(sf::String, std::size_t, std::size_t);
		void deleteText(std::size_t, std::size_t, std::size_t);
		void clearText();

		void addInterval(Interval);
		void removeInterval(std::size_t);
		void resetIntervals();

		void lock(bool);
		bool hasLostFocus();
		bool hasTextChanged();
		/*--------------------------------------------------------------------------------------------*/

		/*--------------------------------------------Sets--------------------------------------------*/
		void setWindow(sf::RenderWindow&);
		void setSize(sf::Vector2f);
		void setPos(sf::Vector2f);
		void setFocus(bool);
		void setScrollbarSize(float);
		void setScrollbarXVisible(bool);
		void setScrollbarYVisible(bool);

		void setOutline(int);
		void setOutlineColor(sf::Color);
		void setBackgroundColor(sf::Color);
		void setBackgroundImage(sf::String);

		void setTextColor(sf::Color);
		void setFont(sf::Font&);
		void setTextSize(std::size_t);
		void setMultiLines(bool);
		void setEditable(bool);

		void setCursorPos(sf::Vector2<std::size_t>);
		void setCursorColor(sf::Color);
		/*--------------------------------------------------------------------------------------------*/

		/*--------------------------------------------Gets--------------------------------------------*/
		sf::RenderWindow& getWindow();
		sf::Vector2f getSize();
		sf::Vector2f getPos();
		float getScrollbarSize();
		bool isLocked();
		bool isEditable();
		bool hasFocus();

		sf::Color getBackgroundColor();

		sf::Color getTextColor();
		unsigned int getTextSize();
		std::vector<sf::String> getTextAsVector();
		sf::String getTextAsString();

		sf::Vector2<std::size_t> getCursorPos();
		sf::Color getCursorColor();
		/*--------------------------------------------------------------------------------------------*/
	private:
		/*-----------------------------------------Variables------------------------------------------*/
		sf::RenderWindow* win;
		sf::View view;
		sf::Vector2f size, pos;
		sf::RectangleShape background;
		sf::Texture background_img_texture;

		std::size_t tx, ty;
		sf::Text text;
		sf::Font font;
		std::vector<sf::String> v_text;
		sf::Uint32 last_input;
		sf::RectangleShape cursor;
		sf::Color text_color;
		bool locked, editable, focus, lost_focus, multiple_lines, text_changed, ctrl_pressed, shift_pressed;
		float max_width, line_height;
		std::size_t max_width_line;
		size_t first_line_modified, last_line_modified;

		Scrollbar scrollbar_x, scrollbar_y;
		float scrollbar_size;
		bool scrollbar_x_visible, scrollbar_y_visible;

		sf::Vector2<std::size_t> selection_begin, selection_end;
		sf::RectangleShape selection_rect;
		std::vector<sf::Vector3<std::size_t>> selection_lines;
		bool selecting, mouse_selecting, selection_switch;

		std::vector<Interval> intervals;

		sf::Clock timer;

		struct Input
		{
			sf::String text;
			std::size_t x, y;
			bool insert;
		};
		std::vector<Input> inputs;
		std::size_t input_index;
		/*--------------------------------------------------------------------------------------------*/

		/*------------------------------------------Functions-----------------------------------------*/
		void singleLineRender();
		void multipleLinesRender();

		std::vector<sf::String> getTextSelected();
		bool deleteTextSelected();
		void cancelSelection();
		void calculateSelection();
		void updateSelection(sf::Vector2<std::size_t>);

		void updateView();
		void moveViewUp();
		void moveViewDown();
		void checkView();
		void resetView();

		sf::String getTextAsString(std::vector<sf::String>);
		float getTextWidth(std::size_t, std::size_t, std::size_t);
		sf::Vector2<std::size_t> getPosInText(sf::Vector2i);
		void updateMaxWidth();
		void checkIntervalWidth(size_t, size_t, size_t&);
		sf::Vector2<std::size_t> nextSymbol(bool, std::size_t, std::size_t);
		sf::Vector2i getMousePosInView();
		/*--------------------------------------------------------------------------------------------*/
	};
}
#endif