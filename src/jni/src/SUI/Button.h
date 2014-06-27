#ifndef BUTTON_H
#define BUTTON_H

#include <SDL.h>
#include "SUI.h"
#include "../Deleters.h"
#include "../CommonFunctions.h"

class Button : public GUIElement{
protected:
	SDL_Rect bounding_box;
	int offset_x,
		offset_y;
public:
	Button(SUI *sui, GUIElement *parent): GUIElement(sui, parent), offset_x(0), offset_y(0){}
	virtual ~Button(){}
	unsigned handle_event(const SDL_Event &);
	void set_bounding_box(const SDL_Rect &bb){
		this->bounding_box = bb;
	}
	const SDL_Rect &get_bounding_box() const{
		return this->bounding_box;
	}
	void set_offset(int offset_x, int offset_y){
		this->offset_x = offset_x;
		this->offset_y = offset_y;
	}
	virtual void on_click() = 0;
};

class IntegerSignalButton : public Button{
protected:
	unsigned signal_value;
	bool global_button;
public:
	IntegerSignalButton(SUI *sui, GUIElement *parent): Button(sui, parent), signal_value(0), global_button(0){}
	virtual ~IntegerSignalButton(){}
	void set_signal_value(unsigned signal_value){
		this->signal_value = signal_value;
	}
	void set_global_button(bool global_button){
		this->global_button = global_button;
	}
	void on_click(){
		(this->global_button ? this->sui : this->parent)->gui_signal(this->signal_value);
	}
};

class GraphicButton : public IntegerSignalButton{
protected:
	Subtexture graphic;
public:
	GraphicButton(SUI *sui, GUIElement *parent): IntegerSignalButton(sui, parent), graphic(){}
	void set_position(int x, int y){
		this->bounding_box.x = x;
		this->bounding_box.y = y;
	}
	void set_graphic(Subtexture graphic){
		this->graphic = graphic;
		auto rect = graphic.get_rect();
		this->bounding_box.x = 0;
		this->bounding_box.y = 0;
		this->bounding_box.w = rect.w;
		this->bounding_box.h = rect.h;
	}
	void update();
};

class TextButton : public IntegerSignalButton{
protected:
	std::wstring text;
	int wrapping_limit;
	double scale;
	int inner_position;

	void calculate_bounding_box();
public:
	TextButton(SUI *sui, GUIElement *parent): IntegerSignalButton(sui, parent), scale(1), inner_position(0){
		this->bounding_box.w = max_possible_value(this->bounding_box.w);
		this->bounding_box.h = max_possible_value(this->bounding_box.h);
	}
	void set_text(const std::wstring &text, int max_width = INT_MAX, double scale = 1.0){
		this->text = text;
		this->wrapping_limit = max_width;
		this->scale = scale;
		this->calculate_bounding_box();
	}
	void set_text_size_mm(double millimeters = 1.0);
	void update();
	const std::wstring &get_text() const{
		return this->text;
	}
	void set_minimum_height(double millimeters);
};

#endif