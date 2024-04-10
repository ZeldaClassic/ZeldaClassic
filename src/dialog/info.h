#pragma once

#include "base/qrs.h"
#include "base/headers.h"
#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/grid.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <initializer_list>
#include <string>
#include <set>
#include <string_view>
#include "dialog/externs.h"

class InfoDialog: public GUI::Dialog<InfoDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL, TOGGLE_QR, BTN };

	InfoDialog(string const& title, string const& text,          string subtext = "");
	InfoDialog(string const& title, vector<string> const& lines, string subtext = "");
	
	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
	
	void setSubtext(string subtext) {d_subtext = subtext;}
protected:
	std::shared_ptr<GUI::Window> window;
	string d_title, d_text;
	string d_subtext;
	
	
	std::set<int> qrs; //related qrs
	std::set<int> ruleTemplates; //related rule templates
	
	byte local_qrs[QR_SZ];
	byte* dest_qrs;
	byte* old_dest_qrs;
	bool on_templates[sz_ruletemplate] = {false};
	
	void postinit();
	
	std::shared_ptr<GUI::Grid> build_text();
};
