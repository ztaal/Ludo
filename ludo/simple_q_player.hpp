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
    double DISCOUNT_FACTOR = 0.95;
    double LEARNING_RATE = 0.5;
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

// States
// 000 Home
// 001 Other
// 010 Star
// 011 Globe
// 100 Enemy globe
// 101 Blockade
// 110 Goal stretch
// 111 Goal

// Actions
// 000 No action
// 001 Move out of home
// 010 Move
// 011 Kill
// 100 Suicide
// 101 Form Blockade
// 110 Protect token
// 111 Move into goal
