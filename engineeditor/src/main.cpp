#include "ec.h"

void create() {
	ec::logInfo("create");
}

void update(){
	
}

int main() {

	ec::Application app;
	app.create(create, update);

	return 0;

}