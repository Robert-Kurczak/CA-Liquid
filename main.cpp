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
        std::vector<std::vector<char>> matrix;

    public:
        bool OnUserCreate() override{
            //---Initialization of cellular automaton matrix---
            for(int y = 0; y < ScreenHeight(); y++){
                std::vector<char> row;

                for(int x = 0; x < ScreenWidth(); x++){
                    row.push_back(0);
                }

                matrix.push_back(row);
            }
            //------

            //---For tests---
            for(int y = 0; y < ScreenHeight() / 2; y++){
                matrix[y][y % 2] = 1;
            }

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override{
            //Clear(olc::BLACK);

            for(int y = 0; y < ScreenHeight(); y++){
                for(int x = 0; x < ScreenWidth(); x++){
                    if(matrix[y][x]) Draw(x, y, olc::WHITE);
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