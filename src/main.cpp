// Written by Thurston Yates for team 877J
// 2021-2022

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// forkliftMotor1       motor         3               
// Controller1          controller                    
// liftMotor            motor         4               
// intakeMotor          motor         5               
// LimitSwitchA         limit         A               
// forkliftMotor2       motor         11              
// Drivetrain           drivetrain    1, 6, 2, 7, 20  
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"
#include "ui.h"
#include <functional>
#include <sstream>
#include <math.h>
#include <map>

using namespace vex;

// ================= Globals ================= //

enum FieldOrigin { Left, Right };

competition Competition;
int selectedAutonRoutine;
bool reversed = false;
bool turbo = false;
int origin;

// =========== Autonomous Routines =========== //

// Type definition for routine function
typedef std::function<void()> routineFn;

// Map of routines
std::map<int, routineFn> routines;

// ============== Control Loops ============== //

void liftControlLoop() {
  bool liftActive = false;
  bool intakeActive = false;
  bool brakeLift = true;

  while(Competition.isDriverControl()) {
    bool const R2Pressing = Controller1.ButtonR2.pressing();
    bool const R1Pressing = Controller1.ButtonR1.pressing();
    bool const UpPressing = Controller1.ButtonUp.pressing();
    bool const DownPressing = Controller1.ButtonDown.pressing();

    // Check if button B is pressing when bucket is closed and open it
    if (UpPressing && !DownPressing) {
      intakeActive = true;
      intakeMotor.spin(forward);
    }

    // Check if button B is pressing when bucket is open and close it
    if (DownPressing && !UpPressing) {
      intakeActive = true;
      intakeMotor.spin(reverse);
    }

    // Turn off intake if no input
    if (!UpPressing && !DownPressing && intakeActive) {
      intakeActive = false;
      intakeMotor.stop();
    } 

    // Check if running without user input and stop
    if (!R2Pressing && !R1Pressing && liftActive) {
      liftMotor.stop(brake);
      liftActive = false;
      brakeLift = true;
    } 

    if (brakeLift) {
      liftMotor.stop(brake);
    }

    // Listen for reverse input
    if (R2Pressing && !R1Pressing) {
      liftMotor.spin(reverse, 100, pct);
      liftActive = true;
      brakeLift = false;
    }

    // Listen for foward input
    if (R1Pressing && !R2Pressing) {
      liftMotor.spin(forward, 100, pct);
      liftActive = true;
      brakeLift = false;
    }
  }
}

void forkliftControlLoop() {
  bool forkliftActive = false;

  while(Competition.isDriverControl()) {
    bool const L2Pressing = Controller1.ButtonL2.pressing();
    bool const L1Pressing = Controller1.ButtonL1.pressing();

    // Check if running without user input and stop
    if ((!L2Pressing && !L1Pressing) && forkliftActive) {
      forkliftActive = false;
      forkliftMotor1.stop(coast);
      forkliftMotor2.stop(coast);
    }

    // Listen for reverse input
    if (L2Pressing && !L1Pressing && !LimitSwitchA.pressing()) {
      forkliftMotor1.spin(reverse, 100, pct);
      forkliftMotor2.spin(reverse, 100, pct);
      forkliftActive = true;
    }

    // Listen for foward input
    if (L1Pressing && !L2Pressing) {
      forkliftMotor1.spin(forward, 100, pct);
      forkliftMotor2.spin(forward, 100, pct);
      forkliftActive = true;
    }
  }
}

void driveControlLoop() {
  bool driving = false;
  bool turning = false;

  while(Competition.isDriverControl()) {
    int const YPos = (reversed) ? -(Controller1.Axis3.position()) : Controller1.Axis3.position();
    int const XPos = (reversed) ? -(Controller1.Axis1.position()) : Controller1.Axis1.position(); 

    // Foward-backward movement
    // Check if control input is greater than 5 for deadzones
    if (YPos > 5 || YPos < -5) {
      Drivetrain.setDriveVelocity(abs((turbo) ? YPos : YPos / 2), pct);
      if (YPos < 0) Drivetrain.drive(reverse);
      if (YPos > 0) Drivetrain.drive(forward);
      driving = true;
    } else if (driving) {
      Drivetrain.setDriveVelocity(0, pct);
      driving = false;
    }

    // left-right movement
    // Check if control input is greater than 5 for deadzones
    if (XPos > 5 || XPos < -5) {
      Drivetrain.setTurnVelocity(XPos, pct);
      Drivetrain.turn(right);
      turning = true;
    } else if (turning) {
      Drivetrain.setTurnVelocity(0, pct);
      turning = false;
    }

    vex::wait(20, msec);
  }
}

void buttonListener() {
  bool debounceY = false;
  bool debounceX = false;
  while(Competition.isDriverControl()) {
    bool const buttonYPressing = Controller1.ButtonY.pressing();
    bool const buttonXPressing = Controller1.ButtonX.pressing();

    // Listen for button Y to be pressed
    if (buttonYPressing && !debounceY) {
      debounceY = true;
      reversed = !reversed;
    } else if (!buttonYPressing && debounceY) debounceY = false;

    // Listen for button X to be pressed
    if (buttonXPressing && !debounceX) {
      debounceX = true;
      turbo = !turbo;
    } else if (!buttonXPressing && debounceX) debounceX = false;
  }
}

// ==================== UI ==================== //

void driveUI() {
  while(true) {
    // Game mode
    bool isAuton = Competition.isAutonomous();
    bool isDrive = Competition.isDriverControl();
    Brain.Screen.clearLine(1);
    
    if (isAuton) {
      std::ostringstream routineStr;
      routineStr << "(ROUTINE " << selectedAutonRoutine << ")";
      Brain.Screen.setCursor(1, 1);
      Brain.Screen.setFillColor(orange);
      Brain.Screen.print(("AUTON MODE" + routineStr.str()).c_str());
      Brain.Screen.setFillColor(transparent);
    }

    if (isDrive) {
      Brain.Screen.setCursor(1, 1);
      Brain.Screen.setFillColor(blue);
      Brain.Screen.print("DRIVE MODE");
      Brain.Screen.setFillColor(transparent);
    }

    // Turbo status
    Brain.Screen.clearLine(2);
    Brain.Screen.setCursor(2, 1);

    (turbo)
    ? Brain.Screen.print("Turbo: On")
    : Brain.Screen.print("Turbo: Off");

    // Reverse status
    Brain.Screen.clearLine(3);
    Brain.Screen.setCursor(3, 1);

    if (reversed) {
      Brain.Screen.setFillColor(green);
      Brain.Screen.print("Reverse: On");
    } else {
      Brain.Screen.setFillColor(red);
      Brain.Screen.print("Reverse: Off");
    }

    Brain.Screen.setFillColor(transparent);

    // Heading
    double const compassHeading = Drivetrain.heading(deg);
    std::ostringstream headingStr;
    headingStr << "Heading: " << round(compassHeading);

    Brain.Screen.clearLine(4);
    Brain.Screen.setCursor(4, 1);
    Brain.Screen.print(headingStr.str().c_str());

    wait(1, sec);
  }
}

void selectionUI() {
  Brain.Screen.print("Select field position");

  ui::Button leftButton = ui::Button(0, 240, 240, 100, 0, 1);
  leftButton.setColor(color(0, 0, 255));
  leftButton.setText("Left");
  leftButton.render();

  ui::Button rightButton = ui::Button(480, 240, 240, 100, 1, 1);
  rightButton.setColor(color(255, 0, 0));
  rightButton.setText("Right");
  rightButton.render();

  bool selected = false;

  while(!selected) {
    if (leftButton.pressing()) {
      origin = FieldOrigin::Left;
      selected = true;
    }

    if (rightButton.pressing()) {
      origin = FieldOrigin::Right;
      selected = true;
    }
  }

  // Clear screen
  Brain.Screen.clearScreen();
  delete &leftButton;
  delete &rightButton;

  // Draw next step
  ui::Textlabel("Select routine", 240, 0, 0.5, 0);

}

// ========= Main Competition Methods ========= //

void pre_auton(void) {
  vexcodeInit();
  selectionUI();
  driveUI();
}

void autonomous(void) {
  // Register autonomous routines
  routines.emplace(0, [&] () -> void {
    liftMotor.spinFor(360, deg, true);
    Drivetrain.driveFor(10, distanceUnits::cm, true);
    intakeMotor.spin(forward);
  });

  routines.emplace(1, [&] () -> void {
    forkliftMotor1.spinFor(forward, 360, deg, false);
    forkliftMotor2.spinFor(forward, 360, deg, false);
    Drivetrain.driveFor(75, distanceUnits::cm, true);
    Drivetrain.driveFor(-75, distanceUnits::cm, true);
    forkliftMotor1.spinFor(reverse, 360, deg, false);
    forkliftMotor2.spinFor(reverse, 360, deg, false);
  });

  // Run selected routine
  routines.find(selectedAutonRoutine)->second();
}

void usercontrol(void) {
  thread driveLoop = thread(driveControlLoop);
  thread forkLoop = thread(forkliftControlLoop);
  thread btnListener = thread(buttonListener);
  thread liftLoop = thread(liftControlLoop);

  btnListener.join();
  forkLoop.join();
  liftLoop.join();
  driveLoop.join();
}

// ================ Entrypoint ================ //

int main() {
  // Set up callbacks for autonomous and driver control periods.
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  // Run the pre-autonomous function.
  pre_auton();

  // Prevent main from exiting with an infinite loop.
  while (true) {
    vex::wait(100, msec);
  }
}
