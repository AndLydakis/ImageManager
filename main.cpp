//
// Created by lydakis on 3/31/18.
//
#include <cstdlib>
#include <unistd.h>
#include "ImageManager.h"

using namespace std;

string available_commands = "Avaible Commands\nDISPLAY\nDISPLAY <image>\nDISPLAY PIXELS\nFIND REGION\nFIND PERIMETER\nSAVE PIXELS\nQUIT\n";

void command_loop(ImageManager &IM) {
    string command;
    cout << available_commands;
    while (true) {
        getline(cin, command);
        if (command == "DISPLAY") {
            IM.DISPLAY_IMAGE();
        } else if (command.find(" ") != std::string::npos) {
            if (command.substr(0, command.find(" ")) == "DISPLAY") {
                if (command.substr(command.find(" ") + 1) != "PIXELS") {
                    IM.DISPLAY_IMAGE(command.substr(command.find(" ") + 1));
                } else if (command == "DISPLAY PIXELS") {
                    IM.DISPLAY_PIXELS();
                }

            } else if (command.substr(0, command.find(' ')) == "FIND") {
                vector<string> tokens;
                size_t pos = 0;
                std::string token;
                while ((pos = command.find(' ')) != std::string::npos) {
                    token = command.substr(0, pos);
                    std::cout << token << std::endl;
                    tokens.emplace_back(token);
                    command.erase(0, pos + 1);
                }
                tokens.emplace_back(command);
                if (tokens.size() < 2) {
                    std::cout << "Invalid command\n";
                    continue;
                }
                if ((tokens.size() == 4) && (tokens[1] == "REGION")) {
                    IM.FIND_REGION(stoi(tokens[2]), stoi(tokens[3]));
                } else if ((tokens.size() == 2) && (tokens[1] == "REGION")) {
                    IM.FIND_REGION();
                } else if ((tokens.size() == 2) && (tokens[1] == "PERIMETER")) {
                    IM.FIND_PERIMETER();
                }
            } else if (command == "SAVE PIXELS") {
                if (command.substr(command.find(" ") + 1) != "") {
                    IM.SAVE_PIXELS(command.substr(command.find(" ") + 1));
                } else {
                    IM.SAVE_PIXELS();
                }
            }
        } else if (command == "QUIT") {
            return;
        } else {
            cout << available_commands;
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
        if (function == "DISPLAY") {
            ImageManager IM(name);
            command_loop(IM);
        } else if (function == "TEST") {
            ImageManager IM;
            IM.DISPLAY_IMAGE("test1.png");
            IM.FIND_REGION(123, 123);
            IM.DISPLAY_PIXELS();
            usleep(3000);
            IM.DISPLAY_IMAGE("test2.png");
            IM.FIND_REGION(123, 43);
            IM.DISPLAY_PIXELS();
            usleep(3000);
            IM.DISPLAY_IMAGE("test3.png");
            IM.FIND_REGION(43, 123);
            IM.DISPLAY_PIXELS();
            usleep(3000);
            IM.DISPLAY_IMAGE("test4.jpg");
            IM.FIND_REGION(55, 55);
            IM.DISPLAY_PIXELS();
            IM.DISPLAY_IMAGE("yesthisfileiscorrupt.png");
            IM.FIND_REGION(123, 123);
            IM.DISPLAY_PIXELS();
            return 0;
        } else {
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
