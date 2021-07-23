#include <SFML/Graphics.hpp>
#include "Msqtt.h"
#include <opencv2/core.hpp>
#include "recognition.h"
#include "interface.h"

int main()
{
	Operator RobotControl("Operator", "127.0.0.1", 1883, false, 2);

	MainWindow window;

	window.MainCycle(RobotControl);

	return 0;
}