#pragma once

#include "ofMain.h"

class Player {
  public:
    Player(const int size);

    void play_melody();
    void play_movie_sequence();
    void lose();
    void win();

    std::vector<std::pair<int,std::string>> m_sequence{};
    std::vector<std::pair<int,std::string>>::iterator m_seq_it{};
    std::vector<int> m_answer{};
    std::vector<int>::iterator m_answer_it{};

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

    std::pair<int,std::string> random_choice();

    ofVideoPlayer m_player;
};

class ofApp : public ofBaseApp
{
  public:
    void setup  ();
    void update ();
    void draw   ();

    void keyPressed      (ofKeyEventArgs&);

    void wait_input(); // wait for user input
    void wait_player(); // wait for choosing a player number
    void record_key(int key);
    void draw_text(const std::string& text, ofPoint center=ofPoint(ofGetWidth()/2,5));

    void credits();
    void timeout();
    void start_splash();

    std::vector<ofFile> m_wait;
    std::vector<ofFile> m_credits;
    std::vector<ofFile> m_splash;


    ofVideoPlayer m_player;

    ofTrueTypeFont m_font;
};
