#pragma once
#include <string>

class Component {
private:
	std::string _name;              //ten tap tin/thu muc
	unsigned int _status;           //tinh trang(thu muc = 1, tap tin = 2)
	unsigned int _firstCluster;     //cluster bat dau
	unsigned int _lastCluster;      //cluster ket thuc
	unsigned int _level;            //tap tin/thu muc level 2 nam trong thu muc level 1
	//tap tin/thu muc level 3 nam trong thu muc level 2
	unsigned int _size;             //kich thuoc tap tin/thu muc
public:
	Component();
	Component(std::string name, unsigned int status, unsigned int firstCluster, unsigned int lastCluster, unsigned int level, unsigned int size);
public:
	std::string name();
	unsigned int status();
	unsigned int firstCluster();
	unsigned int lastCluster();
	unsigned int level();
	unsigned int size();
};