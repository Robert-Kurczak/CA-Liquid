#include <vector>
#include <cstdint>
// #include <string>
// #include <fstream>
// #include <ctime>
// #include <random>

class Matrix{
    private:

        // //Date stamp used for images and videos unique names
        // char dateStamp[80];

        // void updateDateStamp(){
        //     time_t t = time(0);
        //     struct tm *now = localtime(&t);

        //     strftime(dateStamp, 80, "%d.%m.%y_%H-%M-%S", now);
        // }

    public:
        const uint32_t width, height;
        std::vector<std::vector<int>> valueMatrix;

        Matrix(uint32_t width, uint32_t height): width(width), height(height){
            //---Matrix initialization with zeros---
            for(size_t y = 0; y < height; y++){
                std::vector<int> row;

                for(size_t x = 0; x < width; x++){
                    row.push_back(0);
                }

                valueMatrix.push_back(row);
            }
            //------
        }
        
        // void savePBM(std::string location = ""){
        //     updateDateStamp();

        //     if(location == ""){
        //         location = std::string("Images/") + dateStamp;
        //     }

        //     std::ofstream imageFile(location + ".pbm");
        //     imageFile << "P1 " << width << " " << height << std::endl;

        //     for(size_t y = 0; y < height; y++){
        //         for(size_t x = 0; x < width; x++){
        //             imageFile << !valueMatrix[y][x] << " ";
        //         }

        //         imageFile << std::endl;
        //     }

        //     imageFile.close();

        //     //string imageCommand = ".\\ffmpeg -i " + location + ".ppm " + location + ".jpg";
        //     //system(imageCommand.c_str());
        // }
};