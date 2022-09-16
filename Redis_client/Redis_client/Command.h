#pragma once
#ifndef MINIREDIS_COMMAND_H
#define MINIREDIS_COMMAND_H
#include<string>
#include<iostream>
#include<vector>
using namespace std;

const string _all_order[5] = { "set","get","del","load","dump" };
const int _order_number = 5;
string delimiter2 = " ";

class Command {
public:
    string get_command();
    Command(string s) :_command(s) {};
    bool is_valid_command();
    void split_command();
    void print_error(int);
    vector<string> _arg;
private:

    string _command;

};

string Command::get_command() {
    return _command;
}


void Command::split_command() {

    if (_command == "") {
        return;
    }
    string strs = get_command() + delimiter2;

    size_t pos = strs.find(delimiter2);
    size_t size = strs.size();

    //nposÎª½áÊøÎ»ÖÃ
    while (pos != std::string::npos) {
        string x = strs.substr(0, pos);
        //cout << x << "  ";
        _arg.push_back(x);
        strs = strs.substr(pos + 1, size);
        pos = strs.find(delimiter2);
    }
    //cout << endl;
    return;
}

bool Command::is_valid_command() {
    split_command();
    if (_arg.size() == 0) {
        return false;
    }
    string _order = _arg[0];
    bool is_valid_order = false;
    for (int i = 0; i < _order_number; i++) {
        if (_order == _all_order[i]) {
            is_valid_order = true;
        }
    }
    if (!is_valid_order) {
        print_error(1);
        return false;
    }

    if (_order == _all_order[0] && _arg.size() != 3) {
        print_error(2);
        return false;
    }

    if (_order == _all_order[1] && _arg.size() != 2) {
        print_error(3);
        return false;
    }

    if (_order == _all_order[2] && _arg.size() != 2) {
        print_error(4);
        return false;
    }
    if (_order == _all_order[3] && _arg.size() != 1) {
        print_error(5);
        return false;
    }

    if (_order == _all_order[4] && _arg.size() != 1) {
        print_error(6);
        return false;
    }



    return true;
}

void Command::print_error(int error_number) {
    switch (error_number)
    {
    case 1/* constant-expression */:
        cout << "(error) ERR unknown command" << endl;
        break;

    case 2/* constant-expression */:
        cout << "ERR wrong number of arguments for 'set' command" << endl;
        break;

    case 3/* constant-expression */:
        cout << "(error) ERR wrong number of arguments for 'get' command" << endl;
        break;
    case 4/* constant-expression */:
        cout << "(error) ERR wrong number of arguments for 'del' command" << endl;
        break;

    case 5/* constant-expression */:
        cout << "(error) ERR wrong number of arguments for 'load' command" << endl;
        break;
    case 6/* constant-expression */:
        cout << "(error) ERR wrong number of arguments for 'dump' command" << endl;
        break;
    default:
        break;
    }
}

#endif