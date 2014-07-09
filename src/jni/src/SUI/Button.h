#ifndef BUTTON_H
#define BUTTON_H

#include "SUI.h"
#include "../Deleters.h"
#include "../CommonFunctions.h"
#ifndef HAVE_PRECOMPILED_HEADERS
#include <SDL.h>
#endif

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
	GuiSignal signal;
	bool global_button;
public:
	IntegerSignalButton(SUI *sui, GUIElement *parent): Button(sui, parent), global_button(0){
		this->signal.type = SignalType::BUTTON_SIGNAL;
	}
	virtual ~IntegerSignalButton(){}
	void set_signal_value(unsigned signal_value){
		this->signal.data.button_signal = signal_value;
	}
	void set_global_button(bool global_button){
		this->global_button = global_button;
	}
	void on_click(){
		(this->global_button ? this->sui : this->parent)->gui_signal(this->signal);
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
	TextButton(SUI *sui, GUIElement *parent, unsigned signal_value): IntegerSignalButton(sui, parent), scale(1), inner_position(0){
		this->bounding_box.w = max_possible_value(this->bounding_box.w);
		this->bounding_box.h = max_possible_value(this->bounding_box.h);
		this->set_signal_value(signal_value);
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
