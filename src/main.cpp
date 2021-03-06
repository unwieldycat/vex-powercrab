  // Written by Thurston Yates for team 877J
// 2021-2022

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// Controller1          controller                    
// LimitSwitchA         limit         A               
// Drivetrain           drivetrain    3, 2, 6, 1, 21  
// forkliftMotor1       motor         4               
// forkliftMotor2       motor         7               
// liftMotor            motor         11              
// liftGrab             motor         5               
// ---- END VEXCODE CONFIGURED DEVICES ----

// ================================ Imports ================================ //

#include "autonutils.h"
#include "vex.h"
#include "ui.h"
#include <functional>
#include <iostream>
#include <sstream>
#include <math.h>
#include <map>

using namespace vex;

// ================================ Globals ================================ //

autonutils::RoutineManager routineManager;
competition Competition;
int selectedAutonRoutine = 0;
bool doSkills = false;
bool reversed = false;
int origin;

// ============================= Control Loops ============================= //

void grabControlLoop() {
	bool grabActive = false;
	while (Competition.isDriverControl()) {
		bool const R1Pressing = Controller1.ButtonR1.pressing();
		bool const UpPressing = Controller1.ButtonX.pressing();
		bool const DownPressing = Controller1.ButtonB.pressing();

		// Check if running without user input and stop
		if (!R1Pressing && !R2Pressing && grabActive)
		{
			liftMotor.stop(hold);
			grabActive = false;
		}

		// Listen for reverse input
		if (R2Pressing && !R1Pressing)
		{
			liftMotor.spin(reverse, 100, pct);
			grabActive = true;
		}

		// Listen for foward input
		if (R1Pressing && !R2Pressing)
		{
			liftMotor.spin(forward, 100, pct);
			grabActive = true;
		}
	}
}

void liftControlLoop()
{
	bool liftActive = false;
	bool brakeLift = true;

	while (Competition.isDriverControl())
	{
		bool const UpPressing = Controller1.ButtonUp.pressing();
		bool const DownPressing = Controller1.ButtonDown.pressing();

		// Check if running without user input and stop
		if (!UpPressing && !DownPressing && liftActive)
		{
			liftMotor.stop(brake);
			liftActive = false;
			brakeLift = true;
		}

		// Listen for reverse input
		if (DownPressing && !UpPressing)
		{
			liftMotor.spin(reverse, 100, pct);
			liftActive = true;
			brakeLift = false;
		}

		// Listen for foward input
		if (UpPressing && !DownPressing)
		{
			liftMotor.spin(forward, 100, pct);
			liftActive = true;
			brakeLift = false;
		}
	}
}

void forkliftControlLoop()
{
	bool forkliftActive = false;

	while (Competition.isDriverControl())
	{
		bool const L2Pressing = Controller1.ButtonL2.pressing();
		bool const L1Pressing = Controller1.ButtonL1.pressing();

		// Check if running without user input and stop
		if ((!L2Pressing && !L1Pressing) && forkliftActive)
		{
			forkliftActive = false;
			forkliftMotor1.stop(coast);
			forkliftMotor2.stop(coast);
		}

		// Listen for reverse input
		if (L2Pressing && !L1Pressing && !LimitSwitchA.pressing())
		{
			forkliftMotor1.spin(reverse, 100, pct);
			forkliftMotor2.spin(reverse, 100, pct);
			forkliftActive = true;
		}

		// Listen for foward input
		if (L1Pressing && !L2Pressing)
		{
			forkliftMotor1.spin(forward, 100, pct);
			forkliftMotor2.spin(forward, 100, pct);
			forkliftActive = true;
		}
	}
}

void driveControlLoop()
{
  bool driving = false;

  while (Competition.isDriverControl())
  {
    int const YPos = (reversed) ? -(Controller1.Axis3.position()) : Controller1.Axis3.position();
    int const XPos = (reversed) ? -(Controller1.Axis1.position()) : Controller1.Axis1.position();

	if ((YPos > 5 || YPos < -5) || (XPos > 5 || XPos < -5)) {
      LeftDriveSmart.setVelocity(YPos + XPos, pct);
      RightDriveSmart.setVelocity(YPos - XPos, pct);
      LeftDriveSmart.spin(forward);
      RightDriveSmart.spin(forward);
      driving = true;
    } else if (driving) {
      LeftDriveSmart.setVelocity(0, pct);
      RightDriveSmart.setVelocity(0, pct);
	  LeftDriveSmart.stop(hold);
	  RightDriveSmart.stop(hold);
      driving = false;
    }

    vex::wait(5, msec);
  }
}

void buttonListener()
{
  bool debounceY = false;
  while (Competition.isDriverControl())
  {
    bool const buttonYPressing = Controller1.ButtonY.pressing();
    bool const buttonXPressing = Controller1.ButtonX.pressing();

    // Listen for button Y to be pressed
    if (buttonYPressing && !debounceY)
    {
      debounceY = true;
      reversed = !reversed;
    }
    else if (!buttonYPressing && debounceY) debounceY = false;
  }
}

// =================================== UI =================================== //

void driveUI()
{
	while (true)
	{
    // Default competition mode label
    ui::Textlabel modeLabel("Disabled", 240, 120, 0.5, 0.5);
    
    // Set background and status text depending on game mode
    if (Competition.isEnabled()) {
      if (Competition.isAutonomous()) {
        Brain.Screen.clearScreen(vex::color(255, 128, 0));
        modeLabel.setText("Autonomous");
      } 

      if (Competition.isDriverControl()) {
        Brain.Screen.clearScreen(vex::color(0, 128, 255));
        modeLabel.setText("Driver");

      }
    } else Brain.Screen.clearScreen();

    // Render mode label
    modeLabel.render();

    // Wait before next cycle to reduce cpu load
		wait(1, sec);
	}
}

void selectionUI()
{
	// Initalize variables
	bool selected;

	// Draw first step
	ui::Textlabel stepLabel = ui::Textlabel("Select field origin position", 240, 0, 0.5, 0);
  	stepLabel.render();

	ui::Button leftButton = ui::Button(ui::Shape::Rect, 0, 240, 240, 100, 0, 1);
	leftButton.setColor(color(0, 0, 255));
	leftButton.setText("Left");
	leftButton.render();

	ui::Button rightButton = ui::Button(ui::Shape::Rect, 480, 240, 240, 100, 1, 1);
	rightButton.setColor(color(255, 0, 0));
	rightButton.setText("Right");
	rightButton.render();

	ui::Button skillsButton = ui::Button(ui::Shape::Rect, 480, 0, 100, 50, 1, 0);
	skillsButton.setColor(color(255, 128, 0));
	skillsButton.setText("Skills");
	skillsButton.render();

	// Await user selection
	selected = false;

	while (!selected)
	{
		if (skillsButton.pressing()) {
			selectedAutonRoutine = -1;
			return;
		}

		if (leftButton.pressing())
		{
			origin = autonutils::FieldOrigin::Left;
			selected = true;
		}

		if (rightButton.pressing())
		{
			origin = autonutils::FieldOrigin::Right;
			selected = true;
		}
	}

	// Clear screen
	Brain.Screen.clearScreen();

	// Draw next step
	stepLabel.setText("Select routine");
  	stepLabel.render();

	// Draw routine label
	ui::Textlabel routineLabel = ui::Textlabel("X", 240, 120, 0.5, 0.5);
	routineLabel.render();

	// Get routines from routineManager
	std::vector<int> routines = routineManager.find(origin);
	// Inline function to update routine label
	auto const updateRoutineLabel = [&](int id) -> void {
		// Convert routine id at index to string
		std::ostringstream stringified;
		stringified << id;
		
		// Re-render
		routineLabel.setText(stringified.str());
		routineLabel.render();
	};

	if (routines.empty()) {
		selectedAutonRoutine = -1;
		return;
	}
	
	// Render buttons
	ui::Button down = ui::Button(ui::Shape::Rect, 0, 240, 100, 100, 0, 1);
	down.setColor(vex::color(255, 0, 0));
	down.setText("Down");
	down.render();

	ui::Button up = ui::Button(ui::Shape::Rect, 480, 240, 100, 100, 1, 1);
	up.setColor(vex::color(0, 255, 0));
	up.setText("Up");
	up.render();

	ui::Button done = ui::Button(ui::Shape::Rect, 240, 240, 100, 100, 0.5, 1);
	done.setColor(vex::color(0, 0, 255));
	done.setText("Done");
	done.render();

	// Await user selection
	int routineIndex = 0;
	selected = false;
	
	while (!selected) {
		if (down.pressing() && routineIndex > 0) {
			routineIndex--;
			selectedAutonRoutine = routines[routineIndex];
			updateRoutineLabel(selectedAutonRoutine);
		}

		if (up.pressing() && routineIndex < routines.size() - 1) {
			routineIndex++;
			selectedAutonRoutine = routines[routineIndex];
			updateRoutineLabel(selectedAutonRoutine);
		}

		if (done.pressing()) selected = true;
	}

	// Clear screen and reset pen
	Brain.Screen.clearScreen();
	Brain.Screen.setFillColor(transparent);
}

// ============================== Main Methods ============================== //

void pre_auton(void)
{
	vexcodeInit();

	routineManager.add(0, autonutils::FieldOrigin::Both, [&]() -> void {
		liftMotor.spinFor(-(360), deg, true);
		Drivetrain.driveFor(40, distanceUnits::cm, true);
		//intakeMotor.spin(forward); 
	});

	routineManager.add(1, autonutils::FieldOrigin::Both, [&]() -> void {
		forkliftMotor1.spin(reverse, 100, pct);
		forkliftMotor2.spin(reverse, 100, pct);
		Drivetrain.drive(forward, 100, velocityUnits::pct);
    vex::wait(2.5, sec);

    Drivetrain.stop();
		forkliftMotor1.stop(coast);
		forkliftMotor2.stop(coast);
		forkliftMotor1.spin(forward, 100, pct);
		forkliftMotor2.spin(forward, 100, pct);
		wait(2, sec);
		forkliftMotor1.stop(coast);
		forkliftMotor2.stop(coast);

		Drivetrain.drive(reverse, 100, velocityUnits::pct);
		forkliftMotor1.spin(forward, 100, pct);
		forkliftMotor2.spin(forward, 100, pct); 
    vex::wait(2.5, sec);
    Drivetrain.stop();

    forkliftMotor1.stop();
    forkliftMotor2.stop();
	});

	selectionUI();
	driveUI(); 
}

void autonomous(void)
{
	// Run autonomous routine if specified
	if (selectedAutonRoutine > -1) {
		routineManager.exec(selectedAutonRoutine);
		return;
	}

	/******************/
	/* SKILLS ROUTINE */
	/******************/

	// Step 1: Push mobile goals to origin side
	
	// Set start heading to north
	Drivetrain.setHeading(0, deg);

	// Drive to first mobile goal
	Drivetrain.drive(forward, 100, velocityUnits::pct);
	wait(2, sec);
	Drivetrain.turnToHeading(90, deg, true);

	// Pick up goal
	Drivetrain.drive(forward, 50, velocityUnits::pct);
	wait(1.75, sec);

	// Loop this block of code 3 times
	for(int i = 0; i < 3; i++) {
		// Turn to origin side
		Drivetrain.turnToHeading(0, deg, true);

		// Put goal on alliance side
		Drivetrain.drive(forward, 100, velocityUnits::pct);
		wait(2, sec);

		// Go back to center
		Drivetrain.drive(reverse, 100, velocityUnits::pct);
		wait(2, sec);
		Drivetrain.turnToHeading(90, deg, true);

		if (i == 2) break;

		// Pick up next goal if not last goal
		Drivetrain.drive(forward, 50, velocityUnits::pct);
		wait(2.5, sec);
	}

	// Step 2: Retrieve alliance goal and navigate to balance

	// Line up on axis of alliance goal and turn to game north
	Drivetrain.drive(forward, 50, velocityUnits::pct);
	wait(2.5, sec);
	Drivetrain.turnToHeading(0, rotationUnits::deg, true);

	// Reverse into mobile goal
	Drivetrain.drive(reverse, 100, velocityUnits::pct);
	wait(1.5, sec);
	// TODO: Implement grabber motor
	
	// Drive to axis of balance
	Drivetrain.driveFor(60 * 3, distanceUnits::cm, true);
	Drivetrain.turnToHeading(292, rotationUnits::deg, true);
	Drivetrain.driveFor(67, distanceUnits::cm, true);
	Drivetrain.turnToHeading(270, rotationUnits::deg, true);

	// Step 3: Get onto balance

	// Put forklift down
	forkliftMotor1.spin(reverse, 100, pct);
	forkliftMotor2.spin(reverse, 100, pct);
	wait(1.5, sec);
	forkliftMotor1.stop();
	forkliftMotor2.stop();
	
	// Drive onto balance
	Drivetrain.setDriveVelocity(100, percentUnits::pct);
	Drivetrain.driveFor(90, distanceUnits::cm, true);

}

void usercontrol(void)
{
	thread driveLoop = thread(driveControlLoop);
	thread forkLoop = thread(forkliftControlLoop);
	thread btnListener = thread(buttonListener);
	thread liftLoop = thread(liftControlLoop);

	btnListener.join();
	forkLoop.join();
	liftLoop.join();
	driveLoop.join();
}

// =============================== Entrypoint =============================== //

int main()
{
	// Set up callbacks for autonomous and driver control periods.
	Competition.autonomous(autonomous);
	Competition.drivercontrol(usercontrol);

	// Run the pre-autonomous function.
	pre_auton();

	// Prevent main from exiting with an infinite loop.
	while (true)
	{
		vex::wait(100, msec);
	}
}
