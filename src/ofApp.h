#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp
{
  public:
    void setup  ();
    void update ();
    void draw   ();

    void keyPressed      (ofKeyEventArgs&);

    std::pair<int,std::string> random_choice();
    void play_sequence();
    void wait();
    void replay_sequence();
    void record_key(int key);
    void tone(int c);
    void light(int c);
    void credits();
    void timeout();
    void reset();
    void lose();
    void win();

    std::vector<ofFile> shuffle_file_list(std::string path);

    ofVideoPlayer m_player;
    std::vector<ofSoundPlayer> m_samplers;

    std::vector<ofFile> m_vert;
    std::vector<ofFile> m_jaune;
    std::vector<ofFile> m_rouge;
    std::vector<ofFile> m_bleu;

    std::vector<ofFile> m_perdu_vert;
    std::vector<ofFile> m_perdu_jaune;
    std::vector<ofFile> m_perdu_rouge;
    std::vector<ofFile> m_perdu_bleu;

    std::vector<ofFile> m_gagne_vert;
    std::vector<ofFile> m_gagne_jaune;
    std::vector<ofFile> m_gagne_rouge;
    std::vector<ofFile> m_gagne_bleu;

    std::vector<std::pair<int,std::string>> m_sequence;
    std::vector<std::pair<int,std::string>>::iterator m_seq_it;
    std::vector<int> m_answer;
    std::vector<int>::iterator m_answer_it;

    enum GameStatus { WAIT, PLAY, REPLAY, LOSE, WIN, GAME_OVER, TIME_OUT };
    GameStatus m_status;

    ofFbo m_fbo;

    bool m_new_tone{true};
    float m_delay{1.}; // time in second between each note
};
