#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "random"

class Cell{
    public:
        int x, y, lifeTime, value;

        Cell(int x, int y, int lifeTime = -1, int value = 1){
            this->x = x;
            this->y = y;
            this->lifeTime = lifeTime;
            this->value = value;
        }
};

class LiquidSimulator : public olc::PixelGameEngine{
    private:
        double simulationTime = 0;

        std::vector<std::vector<char>> valueMatrix;

    public:
        bool OnUserCreate() override{
            //---Initialization of cellular automaton matrix---
            for(int y = 0; y < ScreenHeight(); y++){
                std::vector<char> row;

                for(int x = 0; x < ScreenWidth(); x++){
                    row.push_back(0);
                }

                valueMatrix.push_back(row);
            }

            for(int x = 0; x < ScreenWidth(); x++){
                valueMatrix[x % 2][x] = 1;
            }
            //------

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override{
            //Updating simulation time
            simulationTime += fElapsedTime;

            //Clearing previous screen
            FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);

            for(int y = 0; y < ScreenHeight(); y++){
                for(int x = 0; x < ScreenWidth(); x++){
                    Draw(x, y, valueMatrix[y][x] == 1 ? olc::BLUE : olc::BLACK);
                }
            }

            // waterCell.y = startPositionY + (9.81 * simulationTime * simulationTime) / 2.f;

            return true;
	}

};
std::vector<Cell> activeCells;

int main(){

    //---Creating window and start simulation---
    LiquidSimulator LS;

    if(LS.Construct(256, 256, 1, 1)){
		LS.Start();
    }
    //------
}