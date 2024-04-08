#pragma once

class ZModule
{
	public:
		bool init(bool d); //bool default
		bool load(bool zquest);
		void debug(); //Print struct data to allegro.log.
		
	private:
		char n;
};
