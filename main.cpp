#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "HUD.h"

#include "cstdlib"
#include "iostream"
#include <math.h>

#define solidBlockID 999

class LiquidSimulator : public olc::PixelGameEngine{
    private:
        //---User input section---
        float panelWidthPercent = 25;
        //------

        //---Need to be initialized in OnUserCreate()---
        olc::vi2d panelSize;
        olc::vi2d simulationSize;
        olc::vi2d matrixSize;
        //------

        std::vector<std::vector<float>> matrix;

        //---Graphic---
        std::unique_ptr<olc::Sprite> spriteSheet;
        olc::vi2d tileSize = {4, 4};
        //------

        //---Parameters---
        char maxWaterValue = 4;
        float compression = 0.1;

        //If divider would have value of 1, no states inbetween
        //of water flowing from cell to cell would be rendered
        float flowDivider = 1;

        //We have to stop dividing at some point
        float minFlow = 0.5;
        //------

        //---For updating on fixed intervals---
        double clock = 0;
        double updateInterval = 0;
        //------

        //Returns neighour of currentPosition, defined by versor
        //Returns -1 if out of range
        float getNeighbour(olc::vi2d currentPosition, olc::vi2d versor){
            int positionX = currentPosition.x + versor.x;
            int positionY = currentPosition.y + versor.y;

            if(positionX < matrixSize.x && positionY < matrixSize.y){
                return matrix[positionY][positionX];
            }
            else{
                return -1;
            }
        }

        //Returns amount of water that should flow from source to sink
        float waterFlowDown(float source, float sink){
            float sum = source + sink;

            //If all water from source will fit in the sink
            if(sum <= maxWaterValue){
                return source;
            }
            //If not all water from source will fit in the sink and source wouldn't be full
            //It means that bottom cell will become compressed but only proportionally to the amount of water above
            else if(sum < (2 * maxWaterValue + compression)){
                return (maxWaterValue * maxWaterValue + sum * compression) / (maxWaterValue + compression) - sink;
            }
            else{
                return ((sum + compression) / 2) - sink;
            }
        }

    public:
        bool OnUserCreate() override{
            //---Calculate sizes---
            panelSize = {int((float)ScreenWidth() * (panelWidthPercent / 100.f)), ScreenHeight()};
            simulationSize = olc::vi2d(ScreenWidth() - panelSize.x, ScreenHeight());
            matrixSize = simulationSize / tileSize;
            //------

            //Load sprite sheet
            spriteSheet = std::make_unique<olc::Sprite>("./Sprites/tiles.png");

            //---Initialization of cellular automaton matrix---
            for(int y = 0; y < matrixSize.y; y++){
                std::vector<float> row;

                for(int x = 0; x < matrixSize.x; x++){
                    //Filling everything with 0, except borders
                    row.push_back((y == 0 || x == 0 || y == matrixSize.y - 1 || x == matrixSize.x - 1) * solidBlockID);
                }

                matrix.push_back(row);
            }
            //------

            matrix[79][79] = solidBlockID;
            matrix[78][79] = solidBlockID;
            matrix[80][80] = solidBlockID;
            matrix[81][80] = solidBlockID;
            matrix[82][80] = solidBlockID;
            matrix[83][80] = solidBlockID;
            matrix[84][80] = solidBlockID;
            matrix[85][80] = solidBlockID;
            matrix[86][80] = solidBlockID;
            matrix[87][80] = solidBlockID;
            matrix[88][80] = solidBlockID;

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override{
            //---Spawn water cell on Enter---
            if(GetKey(olc::Key::ENTER).bHeld){
                matrix[30][100] = 4;
            }
            //------

            //---Executing simulation every update interval---
            clock += fElapsedTime;

            if(clock < updateInterval) return true;
            else clock = 0;
            //------

            Clear(olc::BLACK);

            //---Simulation step---
            for(int y = 0; y < matrixSize.y; y++){
                for(int x = 0; x < matrixSize.x; x++){
                    float &currentCell = matrix[y][x];

                    //Skipping block that are not water
                    if(currentCell == -1 || currentCell == solidBlockID) continue;

                    //---Values of current cell neighbours---
                    float bottomCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(0, 1));
                    float leftCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(-1, 0));
                    float rightCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(1, 0));
                    //------

                    //---Falling down---
                    if(currentCell > 0 && bottomCell != -1 && bottomCell != solidBlockID){
                        float waterToFlow = waterFlowDown(currentCell, bottomCell) / flowDivider;

                        //Instead of instant transfering water
                        //we do it partialy to create smooth transition
                        if(waterToFlow > minFlow) waterToFlow /= flowDivider;

                        matrix[y][x] -= waterToFlow;
                        matrix[y + 1][x] += waterToFlow;
                    }

                    //---Spilling to left---
                    if(currentCell > 0 && leftCell != -1 && leftCell != solidBlockID){
                        if(leftCell < currentCell){
                            float waterToFlow = (currentCell - leftCell) / 4.f;

                            //Instead of instant transfering water
                            //we do it partialy to create smooth transition
                            if(waterToFlow > minFlow) waterToFlow /= flowDivider;
                            
                            matrix[y][x] -= waterToFlow;
                            matrix[y][x - 1] += waterToFlow;
                        }
                    }
                    //------

                    //---Spilling to right---
                    if(currentCell > 0 && rightCell != -1 && rightCell != solidBlockID){
                        if(rightCell < currentCell){

                            float waterToFlow = (currentCell - rightCell) / 4.f;
                            if(waterToFlow > minFlow) waterToFlow /= flowDivider;
                            
                            matrix[y][x] -= waterToFlow;
                            matrix[y][x + 1] += waterToFlow;
                        }
                    }
                    //------
                }
            }
            //------

            //---Rendering matrix---
            for(int y = 0; y < matrixSize.y; y++){
                for(int x = 0; x < matrixSize.x; x++){
                    int value = round(matrix[y][x]);

                    switch(value){
                        case 0:
                            break;

                        case 1:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(0, 0) * tileSize, tileSize);

                            break;

                        case 2:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(1, 0) * tileSize, tileSize);

                            break;

                        case 3:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(2, 0) * tileSize, tileSize);

                            break;

                        default:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(3, 0) * tileSize, tileSize);

                            break;
                        
                        //Rework this to enum or something
                        case solidBlockID:
                            DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(4, 0) * tileSize, tileSize);

                            break;
                    }
                }
            }
            //------

            //---Draw panel---
            DrawString({simulationSize.x + 5, 5}, "Compression: " + std::to_string(compression));
            DrawString({simulationSize.x + 5, 20}, "Flow divider: " + std::to_string(flowDivider));
            //------
            return true;
        }

};

int main(){

    //---Creating window and starting simulation---
    LiquidSimulator LS;

    if(LS.Construct(640, 360, 2, 2)){
		LS.Start();
    }
    //------
}