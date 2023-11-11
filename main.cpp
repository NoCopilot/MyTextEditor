#include "TextBox.hpp"
#include "FileExplorer.hpp"

void addTextBox(std::vector<TextBox*>& v, sf::RenderWindow& win);

int main(int argc, char* argv[])
{
	sf::RenderWindow win(sf::VideoMode(1000, 800), "MyTextEditor");
	sf::View view = win.getView();
	sf::Event e;

	std::vector<TextBox*> textboxes;
	addTextBox(textboxes, win);
	int i = 0;
	//textboxes[i]->loadSpecialKeys({"int", "float", "double", "char", "bool", "short", "long", "unsigned"}, sf::Color::Red);

	win.setFramerateLimit(50);
	win.setView(view);
	while (win.isOpen())
	{
		while (win.pollEvent(e))
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && e.type == sf::Event::KeyPressed)
			{
				switch (e.key.code)
				{
				case sf::Keyboard::N:
					addTextBox(textboxes, win);
					i = textboxes.size() - 1;
					break;
				case sf::Keyboard::Tab:
					i++;
					if (i >= textboxes.size()) i = 0;
					break;
				case sf::Keyboard::O:
					addTextBox(textboxes, win);
					i = textboxes.size() - 1;
					break;
				case sf::Keyboard::S:

					break;
				case sf::Keyboard::W:
					textboxes.erase(textboxes.begin() + i);
					if(i > 0) i--;
					if (textboxes.size() == 0) addTextBox(textboxes, win);
					break;
				}
			}
			if (e.type == sf::Event::Resized)
			{
				view.reset({0, 0, (float)win.getSize().x, (float)win.getSize().y});
				win.setView(view);

				textboxes[i]->setSize(sf::Vector2f(win.getSize()));
			}
			
			textboxes[i]->listen(e);
			
			if (e.type == sf::Event::Closed) win.close();
		}
		win.clear();

		textboxes[i]->draw();

		win.display();
	}
	return 0;
}

void addTextBox(std::vector<TextBox*>& v, sf::RenderWindow& win)
{
	v.push_back(new TextBox);
	v[v.size()-1]->setUp(win, 0, 0, win.getSize().x, win.getSize().y);
	v[v.size()-1]->setFont("font.ttf");
	v[v.size()-1]->setChSize(18);
	v[v.size()-1]->setMultiLines(true);
	v[v.size()-1]->setBarColor(sf::Color::Black);
	v[v.size()-1]->setBgColor(sf::Color::White);
	v[v.size()-1]->setTextColor(sf::Color::Black);
	v[v.size()-1]->setFocus();
}