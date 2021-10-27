#include <vector>
#include <random>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;

int randomInt(int max){
    return rand() % (max + 1);
}

int randomInt(int min, int max){
    return rand() % (max - min + 1) + min;
}

class cell{
    public:
        int x, y, lifeTime;

        cell(int x, int y, int lifeTime = -1){
            this->x = x;
            this->y = y;
            this->lifeTime = lifeTime;
        }
};

class Map{
    private:
        vector<vector<int>> valueMatrix;
        vector<cell> livingCells;
        char dateStamp[80];
        int lastFrame = 1;

        void saveFrame(){
            string name = to_string(lastFrame);
            while(name.length() < 5){
                name = "0" + name;
            }

            savePBM("Videos/temp/"+ name);
            lastFrame++;
        }

        //Fill matrix with zeros (water)
        void initializeMap(){
            for(int y = 0; y < height; y++){
                vector<int> row;

                for(int x = 0; x < width; x++){
                    row.push_back(0);
                }

                valueMatrix.push_back(row);
            }
        }
        //

        //Moore's neighbourhood of given cell
        vector<cell> getNeighbors(cell center, int searchedValue, int distance = 1){
            vector<cell> neighbors;
            
            for(int y = max(0, center.y - distance); y <= min(height - 1, center.y + distance); y++){
                for(int x = max(0, center.x - distance); x <= min(width - 1, center.x + distance); x++){
                    if(x != center.x || y != center.y){
                        if(valueMatrix[y][x] == searchedValue){
                            neighbors.push_back(cell(x, y));
                        }
                    }
                }
            }

            return neighbors;
        }
        //

        //---Land expand step---
        void landIteration(float multiplyChance, int deathTreshold){
            if(makeVideo){
                saveFrame();
            }

            for(int i = 0; i < livingCells.size(); i++){
                //Searching for avaliable cells to expand
                vector<cell> neighbors = getNeighbors(livingCells[i], 0);

                int neighborsAmount = neighbors.size();

                //Death from lack of space case
                if(neighborsAmount < deathTreshold){
                    livingCells.erase(livingCells.begin() + i);
                    continue;
                }
                //

                //Multiply case
                if(randomInt(100) < multiplyChance){
                    livingCells.push_back(livingCells[i]);
                }
                //

                //Expand
                int randomIndex = randomInt(neighborsAmount - 1);
                cell nextCell = neighbors[randomIndex];
                nextCell.lifeTime = livingCells[i].lifeTime - 1;
                livingCells[i] = nextCell;

                valueMatrix[nextCell.y][nextCell.x] = 1;
                //

                //Death from senility case
                if(livingCells[i].lifeTime == 0){
                    livingCells.erase(livingCells.begin() + i);
                }
                //
            }
        }
        //------

        //Smooth generated map
        void smoothTerrain(int smoothLevel){
            for(int i = 0; i < smoothLevel; i++){
                for(int y = 0; y < height; y++){
                    for(int x = 0; x < width; x++){
                        vector<cell> neighbors = getNeighbors(cell(x, y), 1);
                        int neighborsAmount = neighbors.size();

                        if(neighborsAmount > 4){
                            valueMatrix[y][x] = 1;
                        }
                        else if(neighborsAmount < 4){
                            valueMatrix[y][x] = 0;
                        }
                    }

                    if(makeVideo){
                        saveFrame();
                    }
                }
            }
        }
        //

    public:
        int width, height;
        bool makeVideo = false;

        //---Constructor---
        Map(int width, int height){
            this->width = width;
            this->height = height;
        }
        //------

        //Save current valueMatrix state to graphic file
        void savePBM(string location = ""){
            if(location == ""){
                location = string("Images/") + dateStamp;
            }

            ofstream imageFile(location + ".pbm");
            imageFile << "P1 " << width << " " << height << endl;

            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    imageFile << !valueMatrix[y][x] << " ";
                }

                imageFile << endl;
            }

            imageFile.close();

            //string imageCommand = ".\\ffmpeg -i " + location + ".ppm " + location + ".jpg";
            //system(imageCommand.c_str());
        }
        //

        void generateTerrain(
                int initLand,
                int cellLife,
                float multiplyChance,
                int deathTreshold,
                int smoothLevel,
                int seed = time(NULL)
            ){
                
            srand(seed);

            initializeMap();

            //Spawn living cells at random position
            for(int i = 0; i < initLand; i++){
                int randomY = randomInt(height - 1);
                int randomX = randomInt(width - 1);
                
                valueMatrix[randomY][randomX] = 1;

                cell livingCell(randomX, randomY, cellLife);
                livingCells.push_back(livingCell);
            }
            //

            while(livingCells.size() > 0){
                landIteration(multiplyChance, deathTreshold);
            }
            
            smoothTerrain(smoothLevel);
            
            //Generating date stamp
            time_t t = time(0);
            struct tm *now = localtime(&t);

            strftime(dateStamp, 80, "%d.%m.%y_%H-%M-%S", now);
            //

            if(makeVideo){
                lastFrame = 1;

                //Generating video using ffmpeg
                string videoCommand = string(".\\ffmpeg -framerate 350 -i \"Videos\\temp\\%5d.pbm\" \"Videos\\") + dateStamp + string(".avi\"");
                
                system(videoCommand.c_str());

                //Cleaning temp directory
                system("rmdir /s /q .\\Videos\\temp");
                system("mkdir .\\Videos\\temp");
                //
            }
        }
};