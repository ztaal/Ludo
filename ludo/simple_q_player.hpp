#pragma once
#include <QObject>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <sstream>
#include <fstream>
#include "positions_and_dice.h"

class simple_q_player : public QObject {
    Q_OBJECT
private:
    std::vector<int> pos_start_of_turn;
    std::vector<int> pos_end_of_turn;
    int dice_roll;

    int iterations;
    // bool training = true;
    // double EXPLORE_RATE = 0.9;
    bool training = false;
    double EXPLORE_RATE = 0;
    double EXPLORE_RATE_DECAY;
    // double DISCOUNT_FACTOR = 0.95;
    double DISCOUNT_FACTOR = 0.4;
    double LEARNING_RATE = 0.7;
    int make_decision();

    std::vector<int> input_to_state();
    std::vector<int> get_actions();
    int select_action(std::vector<std::vector<double>> q_table,
        std::vector<int> states, std::vector<int> possible_actions);

    void get_reward(std::vector<std::vector<double>> &q_table,
         int action, int state, int decision);

    std::vector<std::vector<double>> load_qtable(std::string filename);
    void save_qtable(std::vector<std::vector<double>> &q_table, std::string filename);
public:
    simple_q_player(int);
signals:
    void select_piece(int);
    void turn_complete(bool);
public slots:
    void start_turn(positions_and_dice relative);
    void post_game_analysis(std::vector<int> relative_pos);
};

// Actions
// No action 0
// Move out of home 1
// Move 2
// Kill 3
// Suicide 4
// Form Blockade 5
// Protect token 6
// Move into goal 7
