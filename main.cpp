#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "cstdlib"
#include "iostream"
#include <math.h>

#include <sstream>
#include <iomanip>

#define solidBlockID 999

class LiquidSimulator : public olc::PixelGameEngine{
    private:
        //---User input section---
        float panelWidthPercent = 35;
        const olc::Pixel panelColors[2] = {olc::WHITE, olc::BLUE};
        //------

        //---Need to be initialized in OnUserCreate()---
        olc::vi2d panelSize;
        olc::vi2d simulationSize;
        olc::vi2d matrixSize;
        //------
        struct cell{
            float value;
            bool isFalling;

            cell(float value, bool isFalling) : value(value), isFalling(isFalling){}
        };

        //Main matrix used for calculation
        std::vector<std::vector<cell>> matrix;

        //Vector for storing prefabricated maps
        std::vector<std::vector<std::vector<cell>>> preMadeMatrix;

        //---Graphic---
        std::unique_ptr<olc::Decal> decalSheet;

        olc::vi2d tileSize = {4, 4};
        //------

        //---Parameters---
        float compression = 0.1;

        //If divider would have value of 1, no states inbetween
        //water flowing from cell to cell would be rendered
        float flowDivider = 1;

        //We have to stop dividing at some point
        float minFlow = 0.5;

        float stepsPerFrame = 1;

        char maxWaterValue = 4;

        float brushSize = 20;
        //------

        //---For updating on fixed intervals---
        float clock = 0;
        float updateInterval = 0;
        //------

        enum parametersTypes {par_float, par_int};

        struct varParameter{
            float& value;
            parametersTypes type;
            std::string label;
            float step;
            float minValue;
            float maxValue;

            varParameter(float& value, parametersTypes type, std::string label, float step, float minValue = -INFINITY, float maxValue = INFINITY)
            : value(value), type(type), label(label), step(step), minValue(minValue), maxValue(maxValue){}

            void increase(){
                value += step;

                if(value > maxValue){
                    value = maxValue;
                }
            }

            void decrease(){
                value -= step;

                if(value < minValue){
                    value = minValue;
                }
            }
        };

        //---Panel variables---
        varParameter parametersToChange[5] = {
            varParameter(compression, par_float, "Compression: ", 0.001),
            varParameter(flowDivider, par_float, "Flow divider: ", 0.001, 1),
            varParameter(updateInterval, par_float, "Update interval: ", 0.0001, 0),
            varParameter(stepsPerFrame, par_int, "Steps per frame: ", 1, 1),
            varParameter(brushSize, par_int, "Brush size: ", 1)
        };

        char parametersAmount = 5;
        char activeOption = 0;
        //------

        //matrixSizes have to be initialized before calling this method
        void initializeMatrix(){
            matrix.clear();

            for(int y = 0; y < matrixSize.y; y++){
                std::vector<cell> row;

                for(int x = 0; x < matrixSize.x; x++){
                    //Filling everything with 0, except borders
                    float value = (y == 0 || x == 0 || y == matrixSize.y - 1 || x == matrixSize.x - 1) * solidBlockID;
                    row.push_back(cell(value, false));
                }

                matrix.push_back(row);
            }
        }

        //Returns neighour of currentPosition, defined by versor
        //Returns -1 if out of range
        float getNeighbour(olc::vi2d currentPosition, olc::vi2d versor){
            int positionX = currentPosition.x + versor.x;
            int positionY = currentPosition.y + versor.y;

            if(positionX < matrixSize.x && positionY < matrixSize.y && positionX >= 0 && positionY >= 0){
                return matrix[positionY][positionX].value;
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

        float waterFlowUp(float source, float sink){
            return source - (waterFlowDown(source, sink) + sink);
        }

        std::string formatNumber(float value, char precision){
            std::stringstream stream;

            stream << std::fixed << std::setprecision(precision) << value;

            return stream.str();
        }

        void drawPanel(){
            for(int i = 0; i < parametersAmount; i++){
                //---Simulation parameters---
                float number = parametersToChange[i].value;
                char precision = 4;

                if(parametersToChange[i].type == par_int){
                    number = (int)number;
                    precision = 0;
                }
                
                std::string label = parametersToChange[i].label;

                DrawString({simulationSize.x + 5, 15 * i + 5}, label + formatNumber(number, precision), panelColors[activeOption == i]);
                //------
            }
        }

        void handleUserInput(){
            //---Reset matrix on R press---
            if(GetKey(olc::Key::R).bPressed){
                initializeMatrix();
            }
            //------

            //---Input panel options---
            //Switch parameter -> Down
            if(GetKey(olc::Key::DOWN).bPressed){
                activeOption++;

                if(activeOption == parametersAmount) activeOption = 0;
            }

            //Switch parameter -> Up
            if(GetKey(olc::Key::UP).bPressed){
                activeOption--;
                
                if(activeOption == -1) activeOption = parametersAmount - 1;
            }

            //Changing parameter value
            if(parametersToChange[activeOption].type == par_float){
                if(GetKey(olc::Key::RIGHT).bHeld) parametersToChange[activeOption].increase();

                if(GetKey(olc::Key::LEFT).bHeld) parametersToChange[activeOption].decrease();
            }
            else{
                if(GetKey(olc::Key::RIGHT).bPressed) parametersToChange[activeOption].increase();

                if(GetKey(olc::Key::LEFT).bPressed) parametersToChange[activeOption].decrease();
            }
            //------

            //---Drawing tiles---
            //Draw solid tile
            if(GetMouse(0).bHeld){
                olc::vi2d position = {GetMouseX(), GetMouseY()};
                if(position.x <= simulationSize.x && position.y <= simulationSize.y){

                    position /= tileSize;

                    int left = position.x - (brushSize / 2);
                    int up = position.y - (brushSize / 2);

                    for(int i = left; i <= left + brushSize; i++){
                        for(int j = up; j <= up + brushSize; j++){
                            if(getNeighbour({i, j}, {0, 0}) != -1){
                                matrix[j][i] = cell(solidBlockID, false);
                            }
                        }
                    }
                }
            }
            //Draw water tile
            if(GetMouse(1).bHeld){
                olc::vi2d position = {GetMouseX(), GetMouseY()};
                if(position.x <= simulationSize.x && position.y <= simulationSize.y){

                    position /= tileSize;

                    int left = position.x - (brushSize / 2);
                    int up = position.y - (brushSize / 2);

                    for(int i = left; i <= left + brushSize; i++){
                        for(int j = up; j <= up + brushSize; j++){
                            if(getNeighbour({i, j}, {0, 0}) != -1){
                                matrix[j][i] = cell(maxWaterValue, false);
                            }
                        }
                    }
                }
            }
            //------
        }

    public:
        bool OnUserCreate() override{
            //---Calculate sizes---
            panelSize = {int((float)ScreenWidth() * (panelWidthPercent / 100.f)), ScreenHeight()};
            simulationSize = olc::vi2d(ScreenWidth() - panelSize.x, ScreenHeight());
            matrixSize = simulationSize / tileSize;
            //------

            //Load sprite sheet
            std::unique_ptr<olc::Sprite> spriteSheet = std::make_unique<olc::Sprite>("./Sprites/tiles.png");
            decalSheet = std::make_unique<olc::Decal>(spriteSheet.get());

            //---Initialization of cellular automaton matrix---
            initializeMatrix();
            //------

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override{
            handleUserInput();

            //---Executing simulation every update interval---
            clock += fElapsedTime;

            if(clock < updateInterval) return true;
            else clock = 0;
            //------

            Clear(olc::BLACK);

            for(int i = 0; i < (int)stepsPerFrame; i++){
                //---Simulation step---
                for(int y = matrixSize.y - 1; y >= 0; y--){
                    for(int x = matrixSize.x - 1; x >= 0; x--){
                        float& currentCell = matrix[y][x].value;
                        //matrix[y][x].isFalling = false;

                        //Skipping blocks that are not water
                        if(currentCell == -1 || currentCell == solidBlockID) continue;

                        //---Values of current cell neighbours---
                        float upperCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(0, -1));
                        float bottomCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(0, 1));
                        float leftCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(-1, 0));
                        float rightCell = getNeighbour(olc::vi2d(x, y), olc::vi2d(1, 0));
                        //------

                        //---Falling down---
                        if(currentCell > 0 && bottomCell != -1 && bottomCell != solidBlockID){
                            float waterToFlow = waterFlowDown(currentCell, bottomCell);

                            //Instead of instant transfering water
                            //we do it partialy to create smooth transition
                            if(waterToFlow > minFlow) waterToFlow /= flowDivider;

                            matrix[y][x].value -= waterToFlow;
                            matrix[y + 1][x].value += waterToFlow;


                            if(waterToFlow > 0.1) matrix[y + 1][x].isFalling = true;
                        }

                        //---Spilling to left---
                        if(currentCell > 0 && leftCell != -1 && leftCell != solidBlockID){
                            if(leftCell < currentCell){

                                float waterToFlow = (currentCell - leftCell) / 4.f;

                                //Instead of instant transfering water
                                //we do it partialy to create smooth transition
                                if(waterToFlow > minFlow) waterToFlow /= flowDivider;

                                matrix[y][x].value -= waterToFlow;
                                matrix[y][x - 1].value += waterToFlow;
                            }     
                        }
                        //------

                        //---Spilling to right---
                        if(currentCell > 0 && rightCell != -1 && rightCell != solidBlockID){
                            if(rightCell < currentCell){

                                float waterToFlow = (currentCell - rightCell) / 4.f;
                                if(waterToFlow > minFlow) waterToFlow /= flowDivider;
                                
                                matrix[y][x].value -= waterToFlow;
                                matrix[y][x + 1].value += waterToFlow;
                            }
                        }
                        //------

                        //---Going up---
                        if(currentCell > 0 && upperCell != -1 && upperCell != solidBlockID){

                            float waterToFlow = waterFlowUp(currentCell, upperCell);

                            //Instead of instant transfering water
                            //we do it partialy to create smooth transition
                            if(waterToFlow > minFlow) waterToFlow /= flowDivider;

                            matrix[y][x].value -= waterToFlow;
                            matrix[y - 1][x].value += waterToFlow;
                        }
                        //------
                    }
                }
                //------
            }

            //---Rendering matrix---
            for(int y = 0; y < matrixSize.y; y++){
                for(int x = 0; x < matrixSize.x; x++){
                    int value = round(matrix[y][x].value);

                    if(value == solidBlockID){
                        DrawPartialDecal(olc::vi2d(x, y) * tileSize, decalSheet.get(), olc::vi2d(4, 0) * tileSize, tileSize);

                        continue;
                    }

                    float compression = matrix[y][x].value - (float)maxWaterValue;
                    int tint = 255;

                    //Interpolate compression to value between 255 (no change) and 64 (dark)
                    if(value != solidBlockID && compression > 0){
                        tint = -191.f/(float)(2 * maxWaterValue) * compression + 255.f;

                        if(tint < 64) tint = 64;
                    }

                    //---Rendering falling liquid as full tile---
                    if(matrix[y][x].isFalling){
                        DrawPartialDecal(olc::vi2d(x, y) * tileSize, decalSheet.get(), olc::vi2d(3, 0) * tileSize, tileSize, {(1.f), (1.f)}, olc::Pixel(tint, tint, tint, 200));

                        matrix[y][x].isFalling = false;
                        continue;
                    }
                    //------

                    switch(value){
                        case 0:
                            break;

                        case 1:
                            // DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(0, 0) * tileSize, tileSize);

                            DrawPartialDecal(olc::vi2d(x, y) * tileSize, decalSheet.get(), olc::vi2d(0, 0) * tileSize, tileSize, {(1.f), (1.f)}, olc::Pixel(tint, tint, tint));
                            break;

                        case 2:
                            // DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(1, 0) * tileSize, tileSize);

                            DrawPartialDecal(olc::vi2d(x, y) * tileSize, decalSheet.get(), olc::vi2d(1, 0) * tileSize, tileSize, {(1.f), (1.f)}, olc::Pixel(tint, tint, tint));

                            break;

                        case 3:
                            // DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(2, 0) * tileSize, tileSize);
                            
                            DrawPartialDecal(olc::vi2d(x, y) * tileSize, decalSheet.get(), olc::vi2d(2, 0) * tileSize, tileSize, {(1.f), (1.f)}, olc::Pixel(tint, tint, tint));

                            break;

                        default:
                            // DrawPartialSprite(olc::vi2d(x, y) * tileSize, spriteSheet.get(), olc::vi2d(3, 0) * tileSize, tileSize);
                            
                            DrawPartialDecal(olc::vi2d(x, y) * tileSize, decalSheet.get(), olc::vi2d(3, 0) * tileSize, tileSize, {(1.f), (1.f)}, olc::Pixel(tint, tint, tint));

                            break;
                    }
                }
            }
            //------

            //---Draw panel---
            drawPanel();
            //------
            return true;
        }

};

//Use dedicated GPU
extern "C"{
  __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

int main(){  
    //---Creating window and starting simulation---
    LiquidSimulator LS;

    if(LS.Construct(640, 360, 2, 2)){
		LS.Start();
    }
    //------
}