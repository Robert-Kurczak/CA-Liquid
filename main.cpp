#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "cstdlib"
#include "iostream"
#include <math.h>

#define solidBlockID 999

// class Cell{

//     public:
//         float waterValue, velocity;
//         Cell(float waterValue, float velocity) : waterValue(waterValue), velocity(velocity){}
// };

class LiquidSimulator : public olc::PixelGameEngine{
    private:
        std::unique_ptr<olc::Sprite> spriteSheet;
        olc::vi2d tileSize = {4, 4};

        char maxWaterValue = 4;
        float compression = 0.1;

        //If divider would have value of 1, no states inbetween
        //of water flowing from cell to cell would be rendered
        float flowDivider = 2;

        //We have to stop dividing at some point
        float minFlow = 0.5;

        std::vector<std::vector<float>> matrix;

        //Need to be initialized in OnUserCreate()
        olc::vi2d matrixSize;

        double clock = 0;
        double updateInterval = 0.02;

        int getNeighbour(olc::vi2d currentPosition, olc::vi2d versor){
            int positionX = currentPosition.x + versor.x;
            int positionY = currentPosition.y + versor.y;

            if(positionX < matrixSize.x && positionY < matrixSize.y){
                return matrix[positionY][positionX];
            }
            else{
                return -1;
            }
        }

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
            matrixSize = {ScreenWidth() / tileSize.x, ScreenHeight() / tileSize.y};

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

            //matrix[8][4] = 5;
            matrix[18][15] = 4;

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override{
            //---Spawn water cell on Enter---
            if(GetKey(olc::Key::ENTER).bPressed){
                matrix[15][15] = 4;
            }//------

            //---Executing simulation every update interval---
            clock += fElapsedTime;

            if(clock < updateInterval){
                return true;
            }
            else{
                clock -= updateInterval;
            }
            //------

            Clear(olc::BLACK);

            //---Simulation step---
            std::vector<std::vector<float>> newMatrix = matrix;
            for(int y = 0; y < matrixSize.y; y++){
                for(int x = 0; x < matrixSize.x; x++){
                    float &currentCell = newMatrix[y][x];

                    //---Debug---//
                    //float xRow[25] = 
                    //------

                    if(currentCell == -1 || currentCell == solidBlockID) continue;

                    float bottomCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(0, 1));
                    float leftCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(-1, 0));
                    float rightCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(1, 0));

                    //---Falling down---
                    if(currentCell > 0 && bottomCell != -1 && bottomCell != solidBlockID){
                        float waterToFlow = waterFlowDown(currentCell, bottomCell) / flowDivider;

                        //Instead of instant transfering water
                        //we do it partialy to create smooth transition
                        if(waterToFlow > minFlow) waterToFlow /= flowDivider;

                        newMatrix[y][x] -= waterToFlow;
                        newMatrix[y + 1][x] += waterToFlow;

                        continue;
                    }

                    //---Spilling to left---
                    if(currentCell > 0 && leftCell != -1 && leftCell != solidBlockID){
                        if(leftCell < currentCell){
                            // float waterToFlow = ((leftCell + currentCell) / 2.f) - leftCell;

                            // //Instead of instant transfering water
                            // //we do it partialy to create smooth transition
                            // if(waterToFlow > minFlow) waterToFlow /= flowDivider;

                            // newMatrix[y][x] -= waterToFlow;
                            // newMatrix[y][x - 1] += waterToFlow;

                            float waterToFlow = (matrix[y][x] - matrix[y][x-1]) / 4.f;
                            if(waterToFlow > minFlow) waterToFlow /= flowDivider;
                            //Flow = constrain(Flow, 0, remaining_mass);
                            
                            newMatrix[y][x] -= waterToFlow;
                            newMatrix[y][x - 1] += waterToFlow;
                            //remaining_mass -= Flow;
                        }
                    }
                    //------

                    //---Spilling to right---
                    if(currentCell > 0 && rightCell != -1 && rightCell != solidBlockID){
                        if(rightCell < currentCell){
                            // float waterToFlow = ((rightCell + currentCell) / 2.f) - rightCell;

                            // //Instead of instant transfering water
                            // //we do it partialy to create smooth transition
                            // if(waterToFlow > minFlow) waterToFlow /= flowDivider;

                            // newMatrix[y][x] -= waterToFlow;
                            // newMatrix[y][x + 1] += waterToFlow;

                            float waterToFlow = (matrix[y][x] - matrix[y][x + 1]) / 4.f;
                            if(waterToFlow > minFlow) waterToFlow /= flowDivider;
                            //Flow = constrain(Flow, 0, remaining_mass);
                            
                            newMatrix[y][x] -= waterToFlow;
                            newMatrix[y][x + 1] += waterToFlow;
                        }
                    }
                    //------
                }
            }

            matrix = newMatrix;
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

            return true;
        }

};

int main(){

    //---Creating window and start simulation---
    LiquidSimulator LS;

    if(LS.Construct(100, 100, 5, 5)){
		LS.Start();
    }
    //------
}