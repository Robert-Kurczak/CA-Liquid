#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "random"
#include "iostream"

class Cell{
    public:
        float x, y, velocity;
        int lifeTime, value;

        Cell(float x, float y, int value = 1, int lifeTime = -1, float velocity = 1.f)
        : x(x), y(y), value(value), lifeTime(lifeTime), velocity(velocity){}
};

class LiquidSimulator : public olc::PixelGameEngine{
    private:
        std::unique_ptr<olc::Sprite> spriteSheet;
        olc::vi2d tileSize = {4, 4};

        std::vector<std::vector<char>> matrix;
        olc::vi2d matrixSize;
        std::vector<Cell> waterCells;

        float waterAcceleration = 5.f;

        char getNeighbour(Cell targetCell, olc::vi2d versor){
            int positionX = targetCell.x + versor.x;
            int positionY = targetCell.y + versor.y;

            if(positionX <= matrixSize.x - 1 && positionY <= matrixSize.y - 1){
                return matrix[positionY][positionX];
            }
            else{
                return -1;
            }
        }

        void updateCellPosition(Cell &targetCell, olc::vf2d position){
            if(position.x > matrixSize.x - 1 || position.y > matrixSize.y - 1) return;
            
            matrix[targetCell.y][targetCell.x] = 0;

            targetCell.x = position.x;
            targetCell.y = position.y;

            matrix[targetCell.y][targetCell.x] = targetCell.value;
        }
    
    public:
        bool OnUserCreate() override{
            matrixSize = {ScreenWidth() / tileSize.x, ScreenHeight() / tileSize.y};

            //Load sprite sheet
            spriteSheet = std::make_unique<olc::Sprite>("./Sprites/tiles.png");

            //---Initialization of cellular automaton matrix---
            for(int y = 0; y < matrixSize.y; y++){
                std::vector<char> row;

                for(int x = 0; x < matrixSize.x; x++){
                    //Filling everything with 0, except borders, which are 5
                    row.push_back((y == 0 || x == 0 || y == matrixSize.y - 1 || x == matrixSize.x - 1) * 5);
                }

                matrix.push_back(row);
            }
            //------

            //---Creating test objects---
            waterCells.push_back(Cell(5, 5, 4));
            //------

            //---Adding waterCells to matrix---
            for(int i = 0; i < waterCells.size(); i++){
                Cell currentCell = waterCells[i];

                matrix[currentCell.y][currentCell.x] = currentCell.value;
            }
            //------

            matrix[8][4] = 5;

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override{
            if(GetKey(olc::Key::ENTER).bPressed){
                waterCells.push_back(Cell(1, 5, 4));
                matrix[5][1] = 4;
            }

            Clear(olc::BLACK);

            //---Rendering matrix---
            for(int y = 0; y < matrixSize.y; y++){
                for(int x = 0; x < matrixSize.x; x++){
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

            //Bufor for storing new water cells, created in simulation step
            std::vector<Cell> newWaterCells;

            //---Simulation step---
            for(int i = 0; i < waterCells.size(); i++){
                Cell &currentCell = waterCells[i];
                char bottomNeighbour = getNeighbour(currentCell, olc::vi2d(0, 1));

                //Falling down case
                if(bottomNeighbour != -1 && bottomNeighbour < currentCell.value){
                    char waterToFlow = currentCell.value - bottomNeighbour;
                    
                    updateCellPosition(currentCell, olc::vd2d(currentCell.x, currentCell.y + currentCell.velocity * fElapsedTime));
                    currentCell.velocity += waterAcceleration * fElapsedTime;
                    currentCell.value += bottomNeighbour;
                }
                else if(bottomNeighbour == 5 && GetKey(olc::Key::SPACE).bPressed){
                    //Spilling on left site and then right site
                    for(char j = -1; j <= 1; j+=2){
                        //Making sure cell object have same value as in matrix
                        currentCell.value = matrix[currentCell.y][currentCell.x];

                        char neighbour = getNeighbour(currentCell, olc::vi2d(j, 0));

                        //If neighbour isn't wall or isn't outside of screen
                        if(neighbour != 5 && neighbour != -1){
                            
                            if(neighbour < currentCell.value && currentCell.value > 1){
                                char waterToSpill = currentCell.value / 2;
                                //If can spill in other direction
                                if(getNeighbour(currentCell, olc::vi2d(j * -1, 0)) < currentCell.value){
                                    waterToSpill = currentCell.value / 3;
                                }

                                //That means that waterCell objest doesn't exist yet
                                if(neighbour == 0){
                                    newWaterCells.push_back(Cell(currentCell.x + j, currentCell.y, 1));
                                }

                                matrix[currentCell.y][currentCell.x + j] += waterToSpill;
                                matrix[currentCell.y][currentCell.x] -= waterToSpill;

                                //I directly modified matrix without looking for proper waterCell object (for performance),
                                //so now, values in matrix don't match with values stored in waterCell objects.
                                //That's why at beggining of this for loop I synchornise those.
                            }
                        }
                    }
                }
            }

            //Adding newly created water cells to main vector
            for(int i = 0; i < newWaterCells.size(); i++){
                waterCells.push_back(newWaterCells[i]);
            }

            newWaterCells = {};

            //------

            return true;
        }

};

int main(){

    //---Creating window and start simulation---
    LiquidSimulator LS;

    if(LS.Construct(40, 40, 20, 20)){
		LS.Start();
    }
    //------
}