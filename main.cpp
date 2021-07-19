class Robot{
public:
	Robot(){}
	void Rotate(float degrees)
	{
		//rotates the robot at degrees degrees
	}

	void Move/*Forward*/(float distance)
	{
		//linear motion at distance meters
	}

	/*float MoveBackward(float distance){}*/

	void Algorithm(float destinationX, float destinationY)
	{
		//identifies intermediate points
		//and passes between them
		//using the methods of the class
	}
private:
	float x;
	float y;
	float azimuth;
	enum State
	{
		WAITING,
		FINDING_PUCK,
		MOVING_TO_PUCK,
		MOVING_WITH_PUCK
	};
	State state;
};

class Operator{
public:
	bool On()
	{
		//turns on the robot
	}

	bool Off()
	{
		//turns off the robot
	}
	void RecieveState()
	{
		//recieve the state of the robot
	}
};
