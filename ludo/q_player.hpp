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

class q_player : public QObject {
    Q_OBJECT
private:
    std::vector<int> pos_start_of_turn;
    std::vector<int> pos_end_of_turn;
    int dice_roll;

    int iterations;
    // double EXPLORE_RATE = 0.9;
    double EXPLORE_RATE = 0;
    double EXPLORE_RATE_DECAY;
    double DISCOUNT_FACTOR = 0.95;
    double LEARNING_RATE = 0.5;
    int make_decision();
    void get_reward(std::vector<std::vector<double>> &q_table,
        std::vector<int> pos_previous_turn, int action, int state);
    std::vector<int> input_to_state();
    int get_action(std::vector<std::vector<double>> q_table,
        std::vector<int> states);
    std::vector<std::vector<double>> load_qtable(std::string filename);
    void save_qtable(std::vector<std::vector<double>> &q_table, std::string filename);
public:
    q_player(int);
signals:
    void select_piece(int);
    void turn_complete(bool);
public slots:
    void start_turn(positions_and_dice relative);
    void post_game_analysis(std::vector<int> relative_pos);
};
