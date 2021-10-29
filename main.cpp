#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "random"
#include "iostream"

class Cell{
    public:
        float x, y;
        int lifeTime, value;

        Cell(float x, float y, int lifeTime = -1, int value = 1){
            this->x = x;
            this->y = y;
            this->lifeTime = lifeTime;
            this->value = value;
        }
};

class LiquidSimulator : public olc::PixelGameEngine{
    private:
        std::vector<std::vector<char>> matrix;
        std::vector<Cell> waterCells;

        double simulationTime = 0;
        std::unique_ptr<olc::Sprite> spriteSheet;
        olc::vi2d tileSize = {4, 4};

    public:
        bool OnUserCreate() override{
            //Load sprite sheet
            spriteSheet = std::make_unique<olc::Sprite>("./Sprites/tiles.png");

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
            // matrix[0][0] = 1;
            // matrix[0][1] = 2;
            // matrix[0][2] = 3;
            // matrix[0][3] = 4;

            waterCells.push_back(Cell(100, 20));
            matrix[20][100] = 2;

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override{
            simulationTime += fElapsedTime;

            Clear(olc::BLACK);

            //---Rendering matrix---
            for(int y = 0; y < ScreenHeight(); y++){
                for(int x = 0; x < ScreenWidth(); x++){
                    switch(matrix[y][x]){
                        case 1:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(0, 0) * tileSize, tileSize);

                            break;

                        case 2:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(1, 0) * tileSize, tileSize);

                            break;

                        case 3:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(2, 0) * tileSize, tileSize);

                            break;

                        case 4:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(3, 0) * tileSize, tileSize);

                            break;
                        
                        case 5:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(4, 0) * tileSize, tileSize);

                            break;
                    }
                }
            }
            //------

            for(int i = 0; i < waterCells.size(); i++){
                float newPositionY = (9.81 * simulationTime * simulationTime) / 2.f;

                if(newPositionY < 90){
                    matrix[waterCells[i].y][waterCells[i].x] = 0;
                    waterCells[i].y = newPositionY;
                    matrix[newPositionY][waterCells[i].x] = 2;

                    std::cout << newPositionY << std::endl;
                }
            }

            return true;
        }

};
std::vector<Cell> activeCells;

int main(){

    //---Creating window and start simulation---
    LiquidSimulator LS;

    if(LS.Construct(640, 360, 2, 2)){
		LS.Start();
    }
    //------
}