#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "cstdlib"
#include "iostream"
#include <math.h>

#include <sstream>
#include <iomanip>

#include <fstream>
#include "nlohmann/json.hpp"

#include <algorithm>

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

        float interfaceFactor;
        //------
        
        struct cell{
            float value;
            bool isFalling;

            cell(float value, bool isFalling) : value(value), isFalling(isFalling){}
        };

        //Main matrix used for calculation
        std::vector<std::vector<cell>> matrix;

        //---Graphic---
        std::unique_ptr<olc::Decal> decalSheet;

        olc::vi2d tileSize = {4, 4};
        //------

        float minFlow = 0.5;
        char maxWaterValue = 4;
        
        //---Parameters---
        float compression = 0.4;
        float flowDivider = 1;
        //We have to stop dividing at some point
        float stepsPerFrame = 5;
        float brushSize = 2;

        //Float instead of bool so it can be compatible
        //with parametersToChange array
        //0 -> false
        //1 -> true
        float drawLines = 0;
        //------

        enum parametersTypes {par_float, par_int, par_bool};

        struct varParameter{
            float& value;
            const float defaultValue;
            parametersTypes type;
            std::string label;
            float step;
            float minValue;
            float maxValue;

            varParameter(float& value, parametersTypes type, std::string label, float step, float minValue = -INFINITY, float maxValue = INFINITY)
            : value(value), type(type), label(label), step(step), minValue(minValue), maxValue(maxValue), defaultValue(value){
            }

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

            void resetValue(){
                value = defaultValue;
            }
        };

        //---Panel variables---
        varParameter parametersToChange[5] = {
            varParameter(compression, par_float, "Compression: ", 0.001),
            varParameter(flowDivider, par_float, "Flow divider: ", 0.001, 1),
            varParameter(stepsPerFrame, par_int, "Steps per frame: ", 1, 1),
            varParameter(brushSize, par_int, "Brush size: ", 1),
            varParameter(drawLines, par_bool, "Draw lines: ", 1, 0, 1)
        };

        char parametersAmount = 5;
        char graphicParameters = 2;
        char activeOption = 0;
        //------

        //matrixSizes have to be initialized before calling this method
        void initializeMatrix(){
            matrix.clear();

            for(int y = 0; y < matrixSize.y; y++){
                std::vector<cell> row;

                for(int x = 0; x < matrixSize.x; x++){
                    //Filling everything with 0
                    row.push_back(cell(0, false));
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

        //Transform given parameter number value to string to be rendered
        std::string formatNumber(varParameter parameter){
            std::stringstream stream;

            if(parameter.type == par_float){
                stream << std::fixed << std::setprecision(4) << parameter.value;
            }
            else if(parameter.type == par_int){
                stream << (int)parameter.value;
            }
            else if(parameter.type == par_bool){
                std::string label[2] = {"Off", "On"};

                stream << label[(int)parameter.value];
            }

            return stream.str();
        }

        //Struct that calculates and stores fixed position of interface text (like headers and labels)
        //based on given margin from the left side of window,
        //header margin from top side of window,
        //label margin - margin between labels
        struct interfacePositions{
            olc::vf2d firstHeader;
            olc::vf2d secondHeader;
            std::vector<olc::vf2d> labels;

            interfacePositions(){}

            interfacePositions(float leftMargin, float headerTopMargin, float labelMargin, int firstSectionlabelsAmount, int secondSectionlabelsAmount){
                firstHeader = {leftMargin, headerTopMargin};

                for(int i = 0; i < firstSectionlabelsAmount; i++){
                    float positionY = headerTopMargin * 2 + i * labelMargin;
                    labels.push_back({leftMargin, positionY});
                }

                float firstSectionHeight = headerTopMargin * 2 + firstSectionlabelsAmount * labelMargin - labelMargin;

                secondHeader = {leftMargin, firstSectionHeight + headerTopMargin};

                for(int i = 0; i < secondSectionlabelsAmount; i++){
                    float positionY = headerTopMargin * 2 + i * labelMargin + firstSectionHeight;
                    labels.push_back({leftMargin, positionY});
                }
            }
        };

        interfacePositions panelPositions;

        void drawMatrixLine(olc::vi2d startPosition, olc::vi2d endPosition){
            int dx =  abs(endPosition.x - startPosition.x);
            int sx = startPosition.x < endPosition.x ? 1 : -1;

            int dy = -abs(endPosition.y - startPosition.y);
            int sy = startPosition.y < endPosition.y ? 1 : -1;

            int err = dx + dy;
            int e2;
            

            while(1){
                //Very unefficient way of drawing line with specific thickness
                //But it's easy and it works
                int left = startPosition.x - (brushSize / 2);
                int up = startPosition.y - (brushSize / 2);

                for(int i = left; i <= left + brushSize; i++){
                    for(int j = up; j <= up + brushSize; j++){
                        if(getNeighbour({i, j}, {0, 0}) != -1){
                            matrix[j][i].value = solidBlockID;
                        }
                    }
                }
                //------

                if(startPosition.x == endPosition.x && startPosition.y == endPosition.y) break;

                e2 = 2 * err;

                if(e2 >= dy){
                    err += dy;
                    startPosition.x += sx;
                }

                if(e2 <= dx){
                    err += dx;
                    startPosition.y += sy;
                }
            }
        }

        void drawPanel(){
            //---Simulation parameters---
            DrawStringDecal(panelPositions.firstHeader, "--Simulation parameters--", olc::WHITE, {interfaceFactor, interfaceFactor});

            for(int i = 0; i < parametersAmount - graphicParameters; i++){                
                std::string label = parametersToChange[i].label;

                DrawStringDecal(panelPositions.labels[i], label + formatNumber(parametersToChange[i]), panelColors[activeOption == i], {interfaceFactor, interfaceFactor});
            }
            //------

            DrawStringDecal(panelPositions.secondHeader, "--Graphic parameters--", olc::WHITE, {interfaceFactor, interfaceFactor});
            
            //---Graphic parameters---
            for(int i = parametersAmount - graphicParameters; i < parametersAmount; i++){
                float number = parametersToChange[i].value;
                char precision = 4;

                if(parametersToChange[i].type == par_int){
                    number = (int)number;
                    precision = 0;
                }
                
                std::string label = parametersToChange[i].label;

                DrawStringDecal(panelPositions.labels[i], label + formatNumber(parametersToChange[i]), panelColors[activeOption == i], {interfaceFactor, interfaceFactor});
            }
            //------
        }

        olc::vi2d firstPosition = {-1, -1};
        void handleUserInput(){
            //---Reset matrix on R press---
            if(GetKey(olc::Key::R).bPressed){
                initializeMatrix();
            }
            //------

            //---Reset parameters on P press---
            if(GetKey(olc::Key::P).bPressed){
                for(int i = 0; i < parametersAmount; i++){
                    parametersToChange[i].resetValue();
                }
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
            if(drawLines){
                if(GetMouse(0).bPressed){
                    if(firstPosition == olc::vi2d(-1, -1)){
                        olc::vi2d position = {GetMouseX(), GetMouseY()};

                        if(position.x <= simulationSize.x && position.y <= simulationSize.y){
                            firstPosition = position / tileSize;
                        }
                    }
                    else{
                        olc::vi2d position = {GetMouseX(), GetMouseY()};

                        if(position.x <= simulationSize.x && position.y <= simulationSize.y){
                            position /= tileSize;

                            drawMatrixLine(firstPosition, position);

                            firstPosition = {-1, -1};
                        }  
                    }
                }
            }
            else{
                if(GetMouse(0).bHeld){
                    olc::vi2d position = {GetMouseX(), GetMouseY()};
                    if(position.x <= simulationSize.x && position.y <= simulationSize.y){
                        position /= tileSize;

                        int left = position.x - (brushSize / 2);
                        int up = position.y - (brushSize / 2);

                        for(int i = left; i <= left + brushSize; i++){
                            for(int j = up; j <= up + brushSize; j++){
                                if(getNeighbour({i, j}, {0, 0}) != -1){
                                    matrix[j][i].value = solidBlockID;
                                }
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

                                if(matrix[j][i].value != solidBlockID){
                                    matrix[j][i].value += maxWaterValue;
                                }
                                else{
                                    matrix[j][i].value = maxWaterValue;
                                }
                            }
                        }
                    }
                }
            }
        
            //Remove tile
            if(GetMouse(2).bHeld){
                olc::vi2d position = {GetMouseX(), GetMouseY()};
                if(position.x <= simulationSize.x && position.y <= simulationSize.y){
                    position /= tileSize;

                    int left = position.x - (brushSize / 2);
                    int up = position.y - (brushSize / 2);

                    for(int i = left; i <= left + brushSize; i++){
                        for(int j = up; j <= up + brushSize; j++){
                            if(getNeighbour({i, j}, {0, 0}) != -1){
                                matrix[j][i].value = 0;
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

            interfaceFactor = ScreenHeight() / 360;

            panelPositions = interfacePositions(
                    (5 + simulationSize.x),
                    25 * interfaceFactor,
                    15 * interfaceFactor,
                    parametersAmount - graphicParameters,
                    graphicParameters
                );
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
    //---Reading user setting from file---
    std::ifstream jsonFile("config.json");
    nlohmann::json configJson;

    jsonFile >> configJson;
    //------

    //---Creating window and starting simulation---
    LiquidSimulator LS;

    if(LS.Construct(
            configJson["width"],
            configJson["height"],
            configJson["scale"],
            configJson["scale"],
            configJson["fullscreen"],
            configJson["vsync"],
            configJson["cohesion"]
        )
    ){
		LS.Start();
    }
    //------
}