#ifndef SCROLLBAR_HPP
#define SCROLLBAR_HPP

#include "SFML/Graphics.hpp"

class Scrollbar
{
	public:
		void init(sf::View& v, float x, float y, float w, float h)
		{
			view = &v;
			
			base.setSize({w, h});
			movable.setSize({w, h});
			base.setPosition({x, y});
			movable.setPosition({x, y});
			
			if(w < h) horizontal = false;
			else horizontal = true;
		}
		void update(float f)
		{
			if(view == nullptr) return;
			if(horizontal)
			{
				if(f > view->getSize().x) max = f;
				else max = view->getSize().x;
				movable.setSize({base.getSize().x * (view->getSize().x / max), base.getSize().y});
				movable.setPosition({(view->getCenter().x - view->getSize().x*0.5f) / view->getSize().x * movable.getSize().x + base.getPosition().x, base.getPosition().y});
			}
			else
			{
				if(f > view->getSize().y) max = f;
				else max = view->getSize().y;
				movable.setSize({base.getSize().x, base.getSize().y * (view->getSize().y / max)});
				movable.setPosition({base.getPosition().x, (view->getCenter().y - view->getSize().y*0.5f) / view->getSize().y * movable.getSize().y + base.getPosition().y});
			}
			checkPos();
		}
		float getScrollPos()
		{
			if(horizontal) return (movable.getPosition().x - base.getPosition().x) / base.getSize().x; 
			return (movable.getPosition().y - base.getPosition().y) / base.getSize().y;
		}
		void setScrollPos(float x)
		{
			if(x < 0.f || x > 1.f) return;
			float temp = max * x;
			if(horizontal)
			{
				movable.setPosition({temp * base.getSize().x, movable.getPosition().y});
				checkPos();
				return;
			}
			movable.setPosition({movable.getPosition().x, temp * base.getSize().y});
			checkPos();
		}
		void draw(sf::RenderWindow* win)
		{
			win->draw(base);
			win->draw(movable);
		}
		void listen(sf::Event& e)
		{
			if(view == nullptr) return;
			if(e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
			{
				mouse = {e.mouseButton.x, e.mouseButton.y};
				mousePressing = false;
				if(mouse.x >= movable.getPosition().x && mouse.x <= (movable.getPosition().x + movable.getSize().x) &&
				   mouse.y >= movable.getPosition().y && mouse.y <= (movable.getPosition().y + movable.getSize().y))
				{
					mousePressing = true;
				}
			}
			if(mousePressing && e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
			{
				mousePressing = false;
			}
			if(mousePressing && e.type == sf::Event::MouseMoved)
			{
				if(horizontal)
				{
					movable.move({(float)e.mouseMove.x - mouse.x, 0});
					mouse.x = e.mouseMove.x;
				}
				else
				{
					movable.move({0, (float)e.mouseMove.y - mouse.y});
					mouse.y = e.mouseMove.y;
				}
				checkPos();
				if(horizontal) view->setCenter({getViewDiff() * view->getSize().x + view->getSize().x*0.5f, view->getCenter().y});
				else view->setCenter({view->getCenter().x, getViewDiff() * view->getSize().y + view->getSize().y*0.5f});
			}
		}
		bool isMoving()
		{
			return mousePressing;
		}
		float getDiff()
		{
			if(horizontal) return movable.getPosition().x - base.getPosition().x;
			return movable.getPosition().y - base.getPosition().y;
		}
		float getViewDiff()
		{
			if(horizontal) return (movable.getPosition().x - base.getPosition().x) / movable.getSize().x;
			return (movable.getPosition().y - base.getPosition().y) / movable.getSize().y;
		}
		sf::Vector2f getSize()
		{
			return base.getSize();
		}
		sf::Vector2f getMovableSize()
		{
			return movable.getSize();
		}
		void setSize(sf::Vector2f newsize)
		{
			base.setSize(newsize);
			update(max);
		}
		void setPos(sf::Vector2f newpos)
		{
			float diff = getDiff();
			base.setPosition(newpos);
			movable.setPosition(newpos);
			if(horizontal) movable.move({diff, 0});
			else movable.move({0, diff});
		}
		void setColor(sf::Color b, sf::Color m)
		{
			base.setFillColor(b);
			movable.setFillColor(m);
		}
	private:
		sf::View* view = nullptr;
		sf::RectangleShape base, movable;
		float max = 0;
		bool horizontal = false, mousePressing = false;
		sf::Vector2i mouse = { 0, 0 };
		
		void checkPos()
		{
			if(horizontal)
			{
				if(base.getPosition().x > movable.getPosition().x)
					movable.setPosition(base.getPosition());
				if((movable.getPosition().x + movable.getSize().x) > (base.getPosition().x + base.getSize().x))
					movable.setPosition({base.getPosition().x + base.getSize().x - movable.getSize().x, movable.getPosition().y});
				
				return;
			}
			if(base.getPosition().y > movable.getPosition().y)
				movable.setPosition(base.getPosition());
			if((movable.getPosition().y + movable.getSize().y) > (base.getPosition().y + base.getSize().y))
				movable.setPosition({movable.getPosition().x, base.getPosition().y + base.getSize().y - movable.getSize().y});
		}
		
		void checkSize()
		{
			if(horizontal)
			{
				if(base.getSize().x < movable.getSize().x)
					movable.setSize(base.getSize());
				return;
			}
			if(base.getSize().y < movable.getSize().y)
					movable.setSize(base.getSize());
		}
};
#endif
