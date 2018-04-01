//
// Created by lydakis on 3/31/18.
//
#include <cstdlib>
#include "ImageManager.h"

using namespace std;

void command_loop(ImageManager& IM){
	string command;
		cout<<"Avaible Commands\nDISPLAY IMAGE\nDISPLAY PIXELS\nFIND REGION\nFIND PERIMETER\nSAVE PIXELS\nQUIT\n";
		while(true){
			getline(cin, command);
			if(command=="DISPLAY IMAGE"){
				IM.DISPLAY_IMAGE();
			}else if(command == "DISPLAY PIXELS"){
				IM.DISPLAY_PIXELS()
			}else if(command == "FIND REGION"){
				IM.FIND_REGION();
			}else if(command == "FIND PERIMETER"){
				IM.FIND_PERIMETER();
			}else if(command == "SAVE PIXELS"){
				IM.SAVE_PIXELS();
			}else if(command == "QUIT"){
				return;
			}else{
				cout<<"Avaible Commands\nDISPLAY IMAGE\nDISPLAY PIXELS\nFIND REGION\nFIND PERIMETER\nSAVE PIXELS\nQUIT\n";
			}
		}
}
int main(int argc, char **argv) {
    cxxopts::Options options(argv[0], " - example command line options");
    options.add_options()
            ("i,image", "Target Image", cxxopts::value<string>()->default_value("test1.png"), "")
            ("b,blue_threshold", "Threshold for blue color", cxxopts::value<string>()->default_value("2"), "")
            ("g,green_threshold", "Threshold for green color", cxxopts::value<string>()->default_value("2"), "")
            ("r,red_threshold", "Threshold for red color", cxxopts::value<string>()->default_value("2"), "")
            ("f,function", "Desired Functionality", cxxopts::value<string>()->default_value("DISPLAY"), "");
    try {
        auto result = options.parse(argc, argv);
        string name = result["image"].as<string>();
        string function = result["function"].as<string>();
        if(function=="DISPLAY"){
			ImageManager IM(name, 1);
			command_loop(IM);
		}else{
			ImageManager IM(name);
			command_loop(IM);
		}
		
    } catch (const cxxopts::OptionException &oe) {
        cout << oe.what() << endl << endl << " *** usage *** " << endl;
        cout << options.help() << endl;
        return -1;
    }
    return EXIT_SUCCESS;
}
