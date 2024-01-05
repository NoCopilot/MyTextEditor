#include "TextBox.hpp"

std::vector<sf::String> split(sf::String str, char del)
{
	std::vector<sf::String> res;
	sf::String s = "";
	for(std::size_t i = 0; i < str.getSize(); i++)
	{
		if((char)str[i] == del) {
			res.push_back(s);
			s = "";
			continue;
		}
		s += str[i];
	}
	res.push_back(s);
	return res;
}

namespace gui
{
	void TextBox::init(sf::RenderWindow& window)
	{
		win = &window;

		scrollbar_size = 10;

		scrollbar_x.setColor(sf::Color(200, 200, 200), sf::Color(100, 100, 100));
		scrollbar_y.setColor(sf::Color(200, 200, 200), sf::Color(100, 100, 100));

		resetView({0, 0}, {200 - scrollbar_size, 200 - scrollbar_size});

		background.setFillColor(sf::Color::White);
		focus = false;
		locked = false;
		editable = true;
		lost_focus = false;

		selection_begin = {0, 0};
		selection_end = {0, 0};
		selection_rect.setFillColor(sf::Color(51, 153, 255));
		selection_lines.clear();
		selecting = false;
		mouse_selecting = false;
		selection_switch = false;

		v_text.push_back("");
		multiple_lines = true;
		text.setFillColor(sf::Color::Black);
		text.setCharacterSize(20);
		line_height = 30; //(text ch size) + 10
		max_width = 0; //empty textbox

		tx = 0; ty = 0;
		max_width_line = 0;
		cursor.setFillColor(sf::Color::Black);
		cursor.setPosition({0, 0});
		cursor.setSize({2, line_height});
		last_input = 0;
		text_changed = false;

		intervals.clear();

		inputs.clear();
		input_index = 0;
	}

	//Class functions
	void TextBox::listen(sf::Event& e)
	{
		/////////////////////////////////////////////
		//Listen Scrollbars Event
		/////////////////////////////////////////////
		scrollbar_x.listen(e);
		if(multiple_lines) scrollbar_y.listen(e);
		if(scrollbar_x.isMoving() || scrollbar_y.isMoving()) background.setPosition({view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f});

		/////////////////////////////////////////////
		//Resize
		/////////////////////////////////////////////
		if(e.type == sf::Event::Resized)
		{
			resetView(pos, {size.x + scrollbar_size, size.y + scrollbar_size});
			return;
		}

		if(locked) return;

		/////////////////////////////////////////////
		//Button Pressed
		//-focus
		//-selection begin
		/////////////////////////////////////////////
		if(e.type == sf::Event::MouseButtonPressed)
		{
			//check if the click is in the textbox
			if(e.mouseButton.x < pos.x || e.mouseButton.x >(pos.x + size.x) ||
				e.mouseButton.y < pos.y || e.mouseButton.y >(pos.y + size.y))
			{
				if(focus) lost_focus = true;
				focus = false;
				return;
			}
			focus = true;

			cancelSelection();

			mouse_selecting = true;
			sf::Vector2<std::size_t> temp = getPosInText(getMousePosInView());
			tx = temp.x;
			ty = temp.y;

			updateSelection(temp);

			updateView();
			return;
		}
		if(!focus) return;
		/////////////////////////////////////////////
		//Mouse Released
		//-selection end
		/////////////////////////////////////////////
		if(mouse_selecting && e.type == sf::Event::MouseMoved)
		{
			sf::Vector2<std::size_t> temp = getPosInText(getMousePosInView());
			updateSelection(temp);
			tx = temp.x;
			ty = temp.y;
			updateView();
			return;
		}
		if(e.type == sf::Event::MouseButtonReleased)
		{
			mouse_selecting = false;
			return;
		}
		ctrl_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl);
		shift_pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
		/////////////////////////////////////////////
		//Mouse Wheel
		//-Zoom
		//-Scroll
		/////////////////////////////////////////////
		if(e.type == sf::Event::MouseWheelScrolled)
		{
			if(ctrl_pressed)
			{
				//zoom | need improvements
				float temp = (view.getCenter().y - view.getSize().y * 0.5f) / line_height;
				float temp1 = max_width;
				setTextSize(getTextSize() + e.mouseWheelScroll.delta);
				updateMaxWidth();
				temp1 = max_width - temp1;
				cursor.setPosition({getTextWidth(0, ty, tx), line_height * ty});
				//why 0.33 times the difference between the max width before and after the text size update? because it seems fine!
				view.setCenter({view.getCenter().x + temp1 * 0.33f, temp * line_height + view.getSize().y * 0.5f});
			}
			else if(view.getSize().y < (line_height * v_text.size()))
			{
				//scroll
				if(e.mouseWheelScroll.delta > 0.f) moveViewUp();
				else moveViewDown();
			}
			checkView();
			scrollbar_x.update(max_width);
			scrollbar_y.update(line_height * v_text.size());
			return;
		}

		if(!editable) return;

		/////////////////////////////////////////////
		//Key Pressed
		//-move in text
		//-move view
		//-selection
		//-canc key
		/////////////////////////////////////////////
		sf::Vector2<std::size_t> precedent_pos = {tx, ty};
		if(e.type == sf::Event::KeyPressed)
		{
			//check del -> check del select
			if(e.key.code == sf::Keyboard::Delete)
			{
				if(!deleteTextSelected())
				{
					if(tx == v_text[ty].getSize())
					{
						if(ty != (v_text.size() - 1))
						{
							v_text[ty] += v_text[ty + 1];
							v_text.erase(v_text.begin() + ty + 1);
						}
					}
					else
					{
						if(ctrl_pressed) v_text[ty].erase(tx, nextSymbol(1, tx, ty).x - tx);
						else v_text[ty].erase(tx);
					}
				}
				goto listenEnd;
			}
			//arrows
			switch((int)e.key.code)
			{
				case sf::Keyboard::Left:
					if(tx == 0)
					{
						if(ty != 0)
						{
							ty--;
							tx = v_text[ty].getSize();
						}
					}
					else
					{
						if(ctrl_pressed)
						{
							sf::Vector2<std::size_t> temp = nextSymbol(1, tx, ty);
							tx = temp.x;
							ty = temp.y;
						}
						else tx--;
					}
					goto arrowSel;
					break;
				case sf::Keyboard::Right:
					if(tx == v_text[ty].getSize())
					{
						if(ty != (v_text.size() - 1))
						{
							ty++;
							tx = 0;
						}
					}
					else
					{
						if(ctrl_pressed)
						{
							sf::Vector2<std::size_t> temp = nextSymbol(1, tx, ty);
							tx = temp.x;
							ty = temp.y;
						}
						else tx++;
					}
					goto arrowSel;
					break;
				case sf::Keyboard::Up:
					if(ctrl_pressed) moveViewUp();
					else if(ty != 0)
					{
						ty--;
						if(tx > v_text[ty].getSize()) tx = v_text[ty].getSize();
					}
					goto arrowSel;
					break;
				case sf::Keyboard::Down:
					if(ctrl_pressed) moveViewDown();
					else if(ty != (v_text.size() - 1))
					{
						ty++;
						if(tx > v_text[ty].getSize()) tx = v_text[ty].getSize();
					}
					goto arrowSel;
					break;
			}
			return;

		arrowSel:
			{
				if(shift_pressed)
				{
					if(!selecting) updateSelection(precedent_pos);
					updateSelection({tx, ty});
				}
				else cancelSelection();
			}
			goto listenEnd;
		}

		/////////////////////////////////////////////
		//Text Entered
		//-add text
		//-deal with special chars
		//-add keys to a better text manipulation
		/////////////////////////////////////////////
		if(e.type == sf::Event::TextEntered)
		{
			last_input = e.text.unicode;
			switch(e.text.unicode)
			{
				case COPY:
					sf::Clipboard::setString(getTextAsString(getTextSelected()));
					return;
					break;
				case CUT:
					sf::Clipboard::setString(getTextAsString(getTextSelected()));
					deleteTextSelected();
					break;
				case PASTE:
					deleteTextSelected();
					writeText(sf::Clipboard::getString());
					break;
				case ENTER:
					deleteTextSelected();
					if(!multiple_lines) break;
					v_text.insert(v_text.begin() + ty + 1, v_text[ty].substring(tx));
					v_text[ty] = v_text[ty].substring(0, tx);
					//set cursor pos
					ty++;
					tx = 0;
					break;
				case DELETE:
				case CDELETE:
					if(deleteTextSelected()) break;
					if(tx == 0)
					{
						if(ty == 0) break;
						tx = v_text[ty - 1].getSize();
						v_text[ty - 1] += v_text[ty];
						v_text.erase(v_text.begin() + ty);
						ty--;
					}
					else
					{
						if(ctrl_pressed)
						{
							sf::Vector2<std::size_t> temp = nextSymbol(0, tx, ty);
							v_text[ty].erase(temp.x, tx - temp.x);
							tx = temp.x;
							break;
						}
						v_text[ty].erase(--tx, 1);
					}
					break;
				default:
					if(e.text.unicode < 32 && e.text.unicode != TAB) return;
					deleteTextSelected();
					v_text[ty].insert(tx++, e.text.unicode);
			}
			goto listenEnd;
		}
		return;
	listenEnd:
		{
		}
		updateView();
	}

	void TextBox::draw()
	{
		sf::View temp_view = win->getView();
		win->setView(view);

		win->draw(background);

		for(std::size_t i = 0; i < selection_lines.size(); i++)
		{
			selection_rect.setSize({getTextWidth(selection_lines[i].x, selection_lines[i].y, selection_lines[i].z), line_height});
			selection_rect.setPosition({getTextWidth(0, selection_lines[i].y, selection_lines[i].x), line_height * selection_lines[i].y});
			win->draw(selection_rect);
		}

		if(multiple_lines) multipleLinesRender();
		else singleLineRender();

		win->draw(cursor);

		win->setView(temp_view);

		scrollbar_x.draw(win);
		if(multiple_lines) scrollbar_y.draw(win);
	}

	void TextBox::writeText(sf::String str)
	{
		writeText(str, tx, ty);
	}

	void TextBox::writeText(sf::String str, std::size_t px, std::size_t py)
	{
		if(py >= v_text.size()) return;
		if(px > v_text[py].getSize()) return;

		sf::String s = v_text[py].substring(px);
		std::vector<sf::String> t = split(str, '\n');
		if(!multiple_lines)
		{
			v_text[0] = v_text[0].substring(0, px) + t[0] + v_text[0].substring(px);
			return;
		}
		v_text[py] = v_text[py].substring(0, px) + t[0];
		for(std::size_t i = 1; i < t.size(); i++)
			v_text.insert(v_text.begin() + py + i, t[i]);
		v_text[py + t.size() - 1] += s;
	}

	void TextBox::deleteText(std::size_t px, std::size_t py, std::size_t n)
	{
		if(n == 0) return;

		if(py >= v_text.size()) return;
		if(px > v_text[py].getSize()) return;

		//first
		std::size_t tempS = v_text[py].getSize();
		v_text[py].erase(px, n);
		n -= tempS - px;

		//check if done
		if(n > 0) py++;
		else return;

		//continue erasing
		for(std::size_t i = 0; i < n; i++)
		{
			if(py >= v_text.size()) return;
			if(v_text[py].getSize() <= n)
			{
				n -= v_text[py].getSize();
				v_text.erase(v_text.begin() + py);
				continue;
			}
			break;
		}
		//end
		v_text[py - 1] += v_text[py].substring(n);
		v_text.erase(v_text.begin() + py);
	}

	void TextBox::clearText()
	{
		v_text.clear();
		v_text.push_back("");
		setCursorPos({0, 0});
	}

	void TextBox::resetIntervals()
	{
		intervals.clear();
	}

	void TextBox::addInterval(Interval iv)
	{
		for(std::size_t i = 0; i < intervals.size(); i++)
		{
			if(iv.y > intervals[i].y) continue;
			if(iv.y == intervals[i].y)
			{
				if(iv.x > intervals[i].x) continue;
				intervals.insert(intervals.begin() + i, iv);
				return;
			}
			intervals.insert(intervals.begin() + i, iv);
			return;
		}
		intervals.push_back(iv);
	}

	void TextBox::removeInterval(std::size_t pos)
	{
		intervals.erase(intervals.begin() + pos);
	}

	//draw single line
	void TextBox::singleLineRender()
	{

	}
	//draw text box
	void TextBox::multipleLinesRender()
	{
		std::size_t n = (view.getCenter().y - view.getSize().y / 2) / line_height;
		std::size_t i = n;
		n = (view.getCenter().y + view.getSize().y / 2) / line_height + 1;

		std::size_t z = 0;
		for(std::size_t j = 0; j < intervals.size(); j++)
		{
			if(intervals[j].y < i) z++;
			else break;
		}

		float temp;
		for(; i < n && i < v_text.size(); ++i)
		{
			temp = 0.f;
			for(std::size_t j = 0; j < v_text[i].getSize(); j++)
			{
				if(z < intervals.size() && intervals[z].y == i && j >= intervals[z].x && j <= (intervals[z].x + intervals[z].n))
				{
					text.setFillColor(intervals[z].color);
					if(j == (intervals[z].x + intervals[z].n - 1)) z++;
				}
				else text.setFillColor(text_color);

				text.setString(v_text[i][j]);
				text.setPosition({temp, line_height * i});
				win->draw(text);
				temp += text.getLocalBounds().width + text.getLocalBounds().left;
			}
		}
	}

	void TextBox::resetView(sf::Vector2f vp, sf::Vector2f vs)
	{
		pos = vp;
		size = {vs.x - scrollbar_size, vs.y - scrollbar_size};

		sf::Vector2f temp = {scrollbar_x.getScrollPos(), scrollbar_y.getScrollPos()};
		if(temp.x != temp.x) temp.x = 0;
		if(temp.y != temp.y) temp.y = 0;

		view.reset({temp.x * max_width, temp.y * line_height * v_text.size(), size.x, size.y});
		view.setViewport({pos.x / win->getSize().x, pos.y / win->getSize().y, size.x / win->getSize().x, size.y / win->getSize().y});

		checkView();

		background.setSize(size);
		background.setPosition({view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f});

		scrollbar_x.init(view, pos.x, pos.y + size.y, size.x, scrollbar_size);
		scrollbar_y.init(view, pos.x + size.x, pos.y, scrollbar_size, size.y);
		scrollbar_x.update(max_width);
		scrollbar_y.update(line_height * v_text.size());
	}

	void TextBox::checkView()
	{
		/*----General check on height----*/
		float temp;
		if((line_height * v_text.size()) < view.getSize().y)
		{
			view.setCenter({view.getCenter().x, view.getSize().y * 0.5f});
			goto checkViewX;
		}

		/*----Checking when too high----*/
		temp = view.getCenter().y - view.getSize().y * 0.5f;
		if(temp < 0.f) view.move({0.f, -temp});

		/*----Checking when too down----*/
		temp = view.getCenter().y + view.getSize().y * 0.5f - line_height * v_text.size();
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

	void TextBox::updateView()
	{
		updateMaxWidth();

		float temp = ty * line_height;
		//checking when cursor is up the view
		if(temp < (view.getCenter().y - view.getSize().y * 0.5f))
		{
			view.setCenter({view.getCenter().x, temp + view.getSize().y * 0.5f});
			background.setPosition({view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f});
		}
		//checking when cursor is down the view
		if(temp > (view.getCenter().y + view.getSize().y * 0.5f - line_height))
		{
			view.setCenter({view.getCenter().x, temp - view.getSize().y * 0.5f + line_height});
			background.setPosition({view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f});
		}
		//check whenever resize
		if((v_text.size() * line_height) > view.getSize().y && temp >= (v_text.size() * line_height - view.getSize().y) && temp < (v_text.size() * line_height))
		{
			temp = v_text.size() * line_height;
			if((v_text.size() * line_height) < (view.getCenter().y + view.getSize().y * 0.5f))
			{
				view.setCenter({view.getCenter().x, temp - view.getSize().y * 0.5f});
				background.setPosition({view.getCenter().x - view.getSize().x * 0.5f, view.getCenter().y - view.getSize().y * 0.5f});
			}
		}

		temp = getTextWidth(0, ty, tx);
		//checking when cursor is left the view

		//checking when cursor is right the view


		//update the cursor pos
		cursor.setPosition({temp, ty * line_height});
		//update the scrollbars
		scrollbar_x.update(max_width);
		scrollbar_y.update(line_height * v_text.size());
	}

	void TextBox::updateMaxWidth()
	{
		max_width = 0.f;
		float temp;
		for(std::size_t i = 0; i < v_text.size(); i++)
		{
			temp = getTextWidth(0, i, v_text[i].getSize());
			if(temp > max_width)
			{
				max_width = temp;
				max_width_line = i;
			}
		}
	}

	void TextBox::moveViewUp()
	{
		view.move({0, -line_height});
		background.move({0, -line_height});
		checkView();
	}

	void TextBox::moveViewDown()
	{
		view.move({0, line_height});
		background.move({0, line_height});
		checkView();
	}

	sf::String TextBox::getTextAsString(std::vector<sf::String> v)
	{
		sf::String str = "";

		for(std::size_t i = 0; i < v.size(); i++) str += v[i] + "\n";
		str.erase(str.getSize() - 1);

		return str;
	}

	sf::Vector2<std::size_t> TextBox::getPosInText(sf::Vector2i mouse)
	{
		//get y pos in v text
		mouse.y /= line_height;
		if((std::size_t)mouse.y > v_text.size()) mouse.y = v_text.size() - 1;
		//get x pos in v text
		if(getTextWidth(0, mouse.y, v_text[mouse.y].getSize()) <= mouse.x) return {v_text[mouse.y].getSize(), (std::size_t)mouse.y};
		else for(std::size_t i = 1; i <= v_text[mouse.y].getSize(); i++)
		{
			if(getTextWidth(0, mouse.y, i) <= mouse.x) continue;
			return {i - 1, (std::size_t)mouse.y};
		}
		return {0, 0};
	}

	std::vector<sf::String> TextBox::getTextSelected()
	{
		if(selection_lines.size() == 0) return {};

		std::vector<sf::String> v;
		for(std::size_t i = 0; i < selection_lines.size(); i++)
			v.push_back(v_text[selection_lines[i].y].substring(selection_lines[i].x, selection_lines[i].z));
		return v;
	}

	void TextBox::calculateSelection()
	{
		selection_lines.clear();
		if(selection_begin == selection_end) return;
		//single line
		if(selection_begin.y == selection_end.y)
		{
			selection_lines.push_back({selection_begin.x, selection_begin.y, selection_end.x - selection_begin.x});
			return;
		}
		//multilines
		selection_lines.push_back({selection_begin.x, selection_begin.y, v_text[selection_begin.y].getSize() - selection_begin.x});	//first line
		for(std::size_t i = selection_begin.y + 1; i < selection_end.y; i++)
			selection_lines.push_back({0, i, v_text[i].getSize()});	//in between
		selection_lines.push_back({0, selection_end.y, selection_end.x});	//last line
	}

	bool TextBox::deleteTextSelected()
	{
		if(selection_lines.size() == 0) return false;
		std::size_t i = 0;

		//single line
		if(selection_lines.size() == 1)
		{
			v_text[selection_lines[i].y].erase(selection_lines[i].x, selection_lines[i].z);
			goto delTSEnd;
		}
		//multilines
		v_text[selection_lines[i].y].erase(selection_lines[i].x, selection_lines[i].z);	//first line
		for(i = 1; i < (selection_lines.size() - 1); i++) v_text.erase(v_text.begin() + selection_lines[0].y + 1);	//in between
		v_text[selection_lines[0].y] += v_text[selection_lines[0].y + 1].substring(selection_lines[i].z);
		v_text.erase(v_text.begin() + selection_lines[0].y + 1);
	delTSEnd:
		{
		}

		tx = selection_begin.x;
		ty = selection_begin.y;

		cancelSelection();
		return true;
	}

	void TextBox::updateSelection(sf::Vector2<std::size_t> temp)
	{
		if(!selecting)
		{
			selecting = true;
			selection_begin = temp;
			selection_end = temp;
			return;
		}

		if(!selection_switch && temp == selection_begin)
		{
			selection_end = temp;
			calculateSelection();
			return;
		}

		if(temp.y < selection_begin.y || (temp.y == selection_begin.y && temp.x < selection_begin.x))
		{
			if(!selection_switch)
			{
				selection_end = selection_begin;
				selection_switch = true;
			}
			selection_begin = temp;
		}
		else if(temp.y > selection_begin.y || (temp.y == selection_begin.y && temp.x > selection_begin.x))
		{
			if(selection_switch)
			{
				if(temp.y < selection_end.y || (temp.y == selection_end.y && temp.x < selection_end.x))
				{
					selection_begin = temp;
					calculateSelection();
					return;
				}
				selection_switch = false;
				selection_begin = selection_end;
			}
			selection_end = temp;
		}
		calculateSelection();
	}

	void TextBox::cancelSelection()
	{
		selecting = false;
		selection_begin = {0, 0};
		selection_end = {0, 0};
		selection_lines.clear();
	}

	float TextBox::getTextWidth(std::size_t sx, std::size_t sy, std::size_t n)
	{
		if(sy >= v_text.size()) return 0.f;
		if(sx >= v_text[sy].getSize()) return 0.f;
		float total = 0.f;
		for(std::size_t i = sx; i < (sx + n); i++)
		{
			text.setString(v_text[sy][i]);
			total += text.getLocalBounds().width + text.getLocalBounds().left;
		}
		return total;
	}

	sf::Vector2i TextBox::getMousePosInView()
	{
		sf::Vector2i mouse_pos = sf::Mouse::getPosition(*win);
		sf::Vector2f view_origin(view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2);

		mouse_pos.x = (int)((mouse_pos.x - pos.x) * (view.getSize().x / size.x) + view_origin.x);
		mouse_pos.y = (int)((mouse_pos.y - pos.y) * (view.getSize().y / size.y) + view_origin.y);

		return mouse_pos;
	}

	sf::Vector2<std::size_t> TextBox::nextSymbol(bool direction, std::size_t px, std::size_t py)
	{
		//todo
		return {};
	}

	//////////////////////////////////////
	//sets
	//////////////////////////////////////
	void TextBox::setWindow(sf::RenderWindow& window)
	{
		win = &window;
		resetView(pos, size);
	}

	void TextBox::setSize(sf::Vector2f psize)
	{
		if(win == nullptr) return;
		resetView(pos, psize);
	}

	void TextBox::setPos(sf::Vector2f ppos)
	{
		if(win == nullptr) return;
		resetView(ppos, size);
	}

	void TextBox::setFocus(bool pfocus)
	{
		focus = pfocus;
	}

	void TextBox::setScrollbarSize(float psize)
	{
		scrollbar_size = psize;
	}

	void TextBox::setOutline(int n)
	{
		background.setOutlineThickness(n);
	}

	void TextBox::setOutlineColor(sf::Color color)
	{
		background.setOutlineColor(color);
	}

	void TextBox::setBackgroundColor(sf::Color color)
	{
		background.setFillColor(color);
	}

	void TextBox::setBackgroundImage(sf::String path)
	{
		if(!background_img_texture.loadFromFile(path)) return;
		background.setTexture(&background_img_texture);
	}

	void TextBox::setTextColor(sf::Color color)
	{
		text_color = color;
		text.setFillColor(color);
	}

	void TextBox::setFont(sf::Font& _font)
	{
		text.setFont(_font);
	}

	void TextBox::setTextSize(std::size_t psize)
	{
		if(psize < 10 || psize > 80) return;
		text.setCharacterSize(psize);
		line_height = psize + 10;
		cursor.setSize({cursor.getSize().x, line_height});
	}

	void TextBox::setMultiLines(bool pbool)
	{
		multiple_lines = pbool;
	}

	void TextBox::setEditable(bool pbool)
	{
		editable = pbool;
	}

	void TextBox::setCursorPos(sf::Vector2<std::size_t> ppos)
	{
		tx = ppos.x; ty = ppos.y;
		cursor.setPosition({getTextWidth(0, ty, tx), line_height * ty});
		//move the view
		updateView();
	}

	void TextBox::setCursorColor(sf::Color color)
	{
		cursor.setFillColor(color);
	}
	//////////////////////////////////////////////////////////////////////////////

	//gets
	sf::RenderWindow& TextBox::getWindow()
	{
		return *win;
	}

	sf::Vector2f TextBox::getSize()
	{
		return size;
	}

	sf::Vector2f TextBox::getPos()
	{
		return pos;
	}

	float TextBox::getScrollbarSize()
	{
		return scrollbar_size;
	}

	bool TextBox::isLocked()
	{
		return locked;
	}

	bool TextBox::isEditable()
	{
		return editable;
	}

	bool TextBox::hasFocus()
	{
		return focus;
	}

	sf::Color TextBox::getBackgroundColor()
	{
		return background.getFillColor();
	}


	sf::Color TextBox::getTextColor()
	{
		return text_color;
	}

	unsigned int TextBox::getTextSize()
	{
		return text.getCharacterSize();
	}

	std::vector<sf::String> TextBox::getTextAsVector()
	{
		return v_text;
	}

	sf::String TextBox::getTextAsString()
	{
		sf::String str = "";

		for(std::size_t i = 0; i < v_text.size(); i++) str += v_text[i] + "\n";
		str.erase(str.getSize() - 1);

		return str;
	}


	sf::Vector2<std::size_t> TextBox::getCursorPos()
	{
		return {tx, ty};
	}

	sf::Color TextBox::getCursorColor()
	{
		return cursor.getFillColor();
	}
	/////////////////////////////////////////////////////////////////////////
}