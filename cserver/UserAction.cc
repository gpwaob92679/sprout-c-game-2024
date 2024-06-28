#include "UserAction.h"

#include <algorithm>
#include <climits>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include "GameController.h"

using namespace wfrest;
using namespace ctl;
using namespace std;

GameController& controller = GameController::getInstance("default");

// -- DON'T MODIFY ANY CODE ABOVE THIS LINE -- //
// -- YOU ARE ALLOW TO ADD HEADER FILES UNDER THIS LINE -- //

// A template map to store the position of each counter
// Question: How do you extend this map to store more than one position for a
// counter? Question2: cutting / normal counter have more than one position, how
// will you handle it?
const map<Counter, vector<pair<int, int>>> counter_positions = {
    {BreadCounter, {{0, 0}, {0, 1}}},
    {CabbageCounter, {{8, 2}, {8, 3}, {8, 4}}},
    {CheeseBlockCounter, {{0, 8}, {0, 9}, {0, 10}}},
    {TomatoCounter, {{8, 5}, {8, 6}, {8, 7}}},
    {RawPattyCounter, {{0, 20}}},
    {StoveCounter, {{0, 17}, {0, 18}, {0, 19}}},
    {PlatesCounter, {{2, 20}, {3, 20}, {4, 20}}},
    {TrashCounter, {{5, 20}, {6, 20}, {7, 20}}},
    {DeliveryCounter, {{0, 20}, {1, 20}}},
    {CuttingCounter, {{0, 14}, {0, 15}, {0, 16}, {8, 14}, {8, 15}, {8, 16}}},
    //{NormalCounter, {{8, 20}}},
};

enum class FacingRequirement {
  kNone = 0,
  kVertical,
  kHorizontal,
};

const map<Counter, FacingRequirement> counter_facing_requirement = {
    {BreadCounter, FacingRequirement::kVertical},
    {CabbageCounter, FacingRequirement::kVertical},
    {CheeseBlockCounter, FacingRequirement::kVertical},
    {TomatoCounter, FacingRequirement::kVertical},
    {RawPattyCounter, FacingRequirement::kVertical},
    {StoveCounter, FacingRequirement::kVertical},
    {PlatesCounter, FacingRequirement::kHorizontal},
    {TrashCounter, FacingRequirement::kHorizontal},
    {DeliveryCounter, FacingRequirement::kHorizontal},
    {CuttingCounter, FacingRequirement::kVertical},
    //{NormalCounter, },
};

enum class Operation {
  kMoveUp = 0,
  kMoveDown,
  kMoveLeft,
  kMoveRight,
  kInteract,
  kInteractSpecial,
};
queue<Operation> operations;

void MakeSalad();
void MakeBurger();
void MakeCheeseBurger();
void MakeMegaBurger();
void AddOperation(Operation op);
void Serve();
pair<int, int> MoveToCounter(Counter counter);
void InteractPoint(const pair<int, int> to, FacingRequirement facing);

pair<int, int> player_position = {4, 10};
Operation last_move_operation = Operation::kMoveUp;
int operation_count = 0;
constexpr pair<int, int> kReservedPlatePosition = {8, 13};
bool has_reserved_plate = true;

int Distance(const pair<int, int> a, const pair<int, int> b) {
  return abs(a.first - b.first) + abs(a.second - b.second);
}

pair<int, int> NearestCounterPosition(Counter counter) {
  pair<int, int> answer;
  int shortest_distance = INT_MAX;
  for (const pair<int, int>& counter_position : counter_positions.at(counter)) {
    int distance = Distance(player_position, counter_position);
    if (distance < shortest_distance) {
      answer = counter_position;
      shortest_distance = distance;
    }
  }
  return answer;
}

// Init the game (DO NOT MODIFY THIS FUNCTION)
void UserAction::InitGame() {
  Initialize();
  // Set the response to "ok" when finished initialization
  controller.SetResponse("ok");
}

// Just a cute Initializing function
void UserAction::Initialize() {
  // TODO: You can do some initialization in this function.
  // Feel free to modify this function.
  // DefaultInitialize() will make you a MEGABurger!
  cout << "Initializing the game..." << endl;
}

void SendOperationFromQueue() {
  if (operations.empty()) {
    controller.Interact();
    return;
  }
  switch (operations.front()) {
    case Operation::kMoveUp:
      controller.MoveUp();
      break;
    case Operation::kMoveDown:
      controller.MoveDown();
      break;
    case Operation::kMoveLeft:
      controller.MoveLeft();
      break;
    case Operation::kMoveRight:
      controller.MoveRight();
      break;
    case Operation::kInteract:
      controller.Interact();
      break;
    case Operation::kInteractSpecial:
      controller.InteractSpecial();
      break;
  }
  operations.pop();
}

// Main Function of you game logic
void UserAction::SendOperation() {
  // TODO: Implement your gaming logic here
  // DefaultSendOperation() will make you a MEGABurger!
  // DefaultSendOperation();

  if (controller.GetRound() == 1) {
    MoveToCounter(Counter::PlatesCounter);
    // Wait for the first plate.
    for (int i = 0; i < 10; ++i) {
      AddOperation(Operation::kMoveRight);
    }
    AddOperation(Operation::kInteract);
    InteractPoint(kReservedPlatePosition, FacingRequirement::kVertical);
    MoveToCounter(Counter::DeliveryCounter);
    return;
  }

  Order new_order = controller.GetNewOrder();
  // Avoid new order at round 1.
  if (controller.GetRound() > 1 && new_order.orderID != -1) {
    operation_count = 0;
    switch (new_order.recipe) {
      case Recipe::Salad:
        MakeSalad();
        break;
      case Recipe::Burger:
        MakeBurger();
        break;
      case Recipe::CheeseBurger:
        MakeCheeseBurger();
        break;
      case Recipe::MegaBurger:
        MakeMegaBurger();
        break;
    }

    cerr << "operation_count = " << operation_count << endl;
    operation_count = 0;
  }
  SendOperationFromQueue();
}

void AddOperation(Operation op) {
  cerr << "AddOperation(" << static_cast<int>(op) << ')' << endl;
  ++operation_count;
  operations.push(op);
  switch (op) {
    case Operation::kMoveUp:
      --player_position.first;
      last_move_operation = op;
      break;
    case Operation::kMoveDown:
      ++player_position.first;
      last_move_operation = op;
      break;
    case Operation::kMoveLeft:
      --player_position.second;
      last_move_operation = op;
      break;
    case Operation::kMoveRight:
      ++player_position.second;
      last_move_operation = op;
      break;
  }
  if (player_position.first < 0) {
    player_position.first = 0;
  } else if (player_position.first > 8) {
    player_position.first = 8;
  }

  if (player_position.second < 0) {
    player_position.second = 0;
  } else if (player_position.second > 20) {
    player_position.second = 20;
  }
  cerr << "player_position = " << player_position.first << ','
       << player_position.second << endl;
}

void CutIngredient(Items ingredient) {
  int cut_count = 0;
  switch (ingredient) {
    case Items::CheeseBlock:
    case Items::Tomato:
      cut_count = 3;
      break;
    case Items::Cabbage:
      cut_count = 5;
      break;
  }
  for (int i = 0; i < cut_count; ++i) {
    AddOperation(Operation::kInteractSpecial);
  }
}

void AddHorizontalMoveOperations(int delta) {
  Operation op = Operation::kMoveRight;
  if (delta < 0) {
    op = Operation::kMoveLeft;
  }
  for (int i = 0; i < abs(delta); ++i) {
    AddOperation(op);
  }
}

void AddVerticalMoveOperations(int delta) {
  Operation op = Operation::kMoveDown;
  if (delta < 0) {
    op = Operation::kMoveUp;
  }
  for (int i = 0; i < abs(delta); ++i) {
    AddOperation(op);
  }
}

void MoveToPoint(const pair<int, int> to, FacingRequirement facing) {
  cerr << "MoveToPoint((" << to.first << ',' << to.second << "))" << endl;
  if (facing == FacingRequirement::kHorizontal) {
    AddVerticalMoveOperations(to.first - player_position.first);
    AddHorizontalMoveOperations(to.second - player_position.second);
    if (last_move_operation == Operation::kMoveUp ||
        last_move_operation == Operation::kMoveDown) {
      if (to.second == 0) {
        AddOperation(Operation::kMoveLeft);
      } else {
        AddOperation(Operation::kMoveRight);
      }
    }
  } else {
    AddHorizontalMoveOperations(to.second - player_position.second);
    AddVerticalMoveOperations(to.first - player_position.first);
    if (last_move_operation == Operation::kMoveLeft ||
        last_move_operation == Operation::kMoveRight) {
      if (to.first == 0) {
        AddOperation(Operation::kMoveUp);
      } else {
        AddOperation(Operation::kMoveDown);
      }
    }
  }
}

pair<int, int> MoveToCounter(Counter counter) {
  pair<int, int> to = NearestCounterPosition(counter);
  cerr << "MoveToCounter(" << counter << ", (" << to.first << ',' << to.second
       << "))" << endl;
  MoveToPoint(to, counter_facing_requirement.at(counter));
  return to;
}

void MoveCounterToCounter(Counter from, Counter to) {}

inline void Serve() {
  // Serve order.
  MoveToCounter(Counter::DeliveryCounter);
  AddOperation(Operation::kInteract);
}

inline void InteractCounter(Counter counter) {
  MoveToCounter(counter);
  AddOperation(Operation::kInteract);
}

inline void InteractPoint(const pair<int, int> to, FacingRequirement facing) {
  MoveToPoint(to, facing);
  AddOperation(Operation::kInteract);
}

void ReservePlate() {
  InteractCounter(Counter::PlatesCounter);
  InteractPoint(kReservedPlatePosition, FacingRequirement::kVertical);
  // Avoid repeatedly picking up and putting down the plate.
  AddOperation(Operation::kMoveRight);
  has_reserved_plate = true;
}

void MakeSalad() {
  if (!has_reserved_plate) {
    InteractCounter(Counter::PlatesCounter);
    InteractPoint(kReservedPlatePosition, FacingRequirement::kVertical);
    has_reserved_plate = true;
  }

  // Grab cabbage.
  InteractCounter(Counter::CabbageCounter);
  assert(last_move_operation == Operation::kMoveUp);
  // Cut cabbage.
  InteractCounter(Counter::CuttingCounter);
  CutIngredient(Items::Cabbage);
  // Move cabbage to next counter.
  AddOperation(Operation::kInteract);
  InteractPoint(kReservedPlatePosition, FacingRequirement::kVertical);
  assert(last_move_operation == Operation::kMoveDown);

  // Grab tomato.
  InteractCounter(Counter::TomatoCounter);
  // Cut tomato.
  InteractCounter(Counter::CuttingCounter);
  CutIngredient(Items::Tomato);

  has_reserved_plate = false;
  InteractPoint(kReservedPlatePosition, FacingRequirement::kVertical);
  InteractCounter(Counter::CuttingCounter);

  MoveToCounter(Counter::DeliveryCounter);
  // if (controller.GetRecipeMode() == "Salad") {
  //   int order_arrival_time = (controller.GetRound() / 100 + 1) * 100;
  //   for (int i = controller.GetRound() + operation_count;
  //        i < order_arrival_time; ++i) {
  //     AddOperation(Operation::kMoveRight);
  //   }
  //   AddOperation(Operation::kInteract);
  // } else {
  //   Serve();
  // }
  Serve();
}

void MakeBurger() {
  InteractCounter(Counter::RawPattyCounter);
  InteractCounter(Counter::StoveCounter);

  InteractCounter(Counter::BreadCounter);
  pair<int, int> bread = {0, 13};
  InteractPoint(bread, FacingRequirement::kVertical);

  InteractCounter(Counter::PlatesCounter);
  InteractPoint(bread, FacingRequirement::kVertical);
  InteractCounter(Counter::StoveCounter);
  Serve();
}

void MakeCheeseBurger() {
  InteractCounter(Counter::RawPattyCounter);
  InteractCounter(Counter::StoveCounter);

  InteractCounter(Counter::BreadCounter);
  pair<int, int> bread = {0, 13};
  InteractPoint(bread, FacingRequirement::kVertical);

  InteractCounter(Counter::CheeseBlockCounter);
  InteractCounter(Counter::CuttingCounter);
  CutIngredient(Items::CheeseBlock);

  InteractCounter(Counter::PlatesCounter);
  InteractPoint(bread, FacingRequirement::kVertical);
  InteractCounter(Counter::CuttingCounter);
  InteractCounter(Counter::StoveCounter);
  Serve();
}

void MakeMegaBurger() {
  InteractCounter(Counter::RawPattyCounter);
  InteractCounter(Counter::StoveCounter);

  InteractCounter(Counter::BreadCounter);
  pair<int, int> bread = {0, 13};
  InteractPoint(bread, FacingRequirement::kVertical);

  InteractCounter(Counter::CheeseBlockCounter);
  InteractCounter(Counter::CuttingCounter);
  CutIngredient(Items::CheeseBlock);

  if (!has_reserved_plate) {
    InteractCounter(Counter::PlatesCounter);
    InteractPoint(kReservedPlatePosition, FacingRequirement::kVertical);
  }
  has_reserved_plate = false;

  InteractCounter(Counter::CabbageCounter);
  InteractCounter(Counter::CuttingCounter);
  CutIngredient(Items::Cabbage);
  AddOperation(Operation::kInteract);
  InteractPoint(kReservedPlatePosition, FacingRequirement::kVertical);

  InteractCounter(Counter::TomatoCounter);
  InteractCounter(Counter::CuttingCounter);
  CutIngredient(Items::Tomato);

  InteractPoint(kReservedPlatePosition, FacingRequirement::kVertical);
  InteractCounter(Counter::CuttingCounter);

  InteractPoint(bread, FacingRequirement::kVertical);
  InteractCounter(Counter::CuttingCounter);
  InteractCounter(Counter::StoveCounter);
  Serve();
}
