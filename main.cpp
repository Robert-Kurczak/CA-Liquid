#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class CA : public olc::PixelGameEngine{
    public:
        bool OnUserCreate() override{
            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override{
            for(int x = 0; x < ScreenWidth(); x++)
                for(int y = 0; y < ScreenHeight(); y++)
                    Draw(x, y, olc::Pixel(rand() % 256, rand() % 256, rand()% 256));	
            return true;
	}

};

int main(){
    CA cellularAutomaton;

    if(cellularAutomaton.Construct(256, 240, 4, 4))
		cellularAutomaton.Start();

}