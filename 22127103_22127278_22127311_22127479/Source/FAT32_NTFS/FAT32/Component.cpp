#include "Component.h"
#include <iostream>

Component::Component() {

}

Component::Component(std::string name, unsigned int status, unsigned int firstCluster, unsigned int lastCluster, unsigned int level, unsigned int size) {
	_name = name;
	_status = status;
	_firstCluster = firstCluster;
	_lastCluster = lastCluster;
	_level = level;
	_size = size;
}

std::string Component::name() {
	return _name;
}

unsigned int Component::status() {
	return _status;
}
unsigned int Component::firstCluster() {
	return _firstCluster;
}
unsigned int Component::lastCluster() {
	return _lastCluster;
}
unsigned int Component::level() {
	return _level;
}
unsigned int Component::size() {
	return _size;
}

